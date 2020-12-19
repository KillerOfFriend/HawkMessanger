#include "jsondatastorage.h"

#include <string>
#include <fstream>
#include <algorithm>

#include <HawkLog.h>
#include <systemerrorex.h>

#include "jsondatastorageconst.h"
#include "datastorage/datastorageerrorcategory.h"

#include <QCryptographicHash>

using namespace hmservcommon::datastorage;

//-----------------------------------------------------------------------------
HMJsonDataStorage::HMJsonDataStorage(const std::filesystem::path &inJsonPath) :
    HMAbstractHardDataStorage(), // Инициализируем предка
    m_jsonPath(inJsonPath)
{

}
//-----------------------------------------------------------------------------
HMJsonDataStorage::~HMJsonDataStorage()
{
    close();
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::open()
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    close();

    if (std::filesystem::is_directory(m_jsonPath, Error))
    {
        if (!Error) // Не зарегестрирована внутренняя ошибка filesystem
            Error = make_error_code(hmcommon::eSystemErrorEx::seObjectNotFile); // Задаём свою (Путь указывает не на является файлом)
    }
    else // Объект файл
    {
        if (!std::filesystem::exists(m_jsonPath, Error)) // Проверяем что файл вообще существует
        {
            if (!Error) // Не зарегестрирована внутренняя ошибка filesystem
            {
                Error = makeDefault(); // Создадим пустой файл
                if (!Error) // Создание структуры прошло без ошибок
                {
                    Error = checkCorrectStruct(); // Проверяем корректность созданной структуры
                    if (Error) // Если структура не прошла проверку
                        m_json.clear(); // Сносим структуру
                }
            }
        }
        else // Файл существует
        {
            std::ifstream inFile(m_jsonPath, std::ios_base::in);

            if (!inFile.is_open())
                Error = make_error_code(hmcommon::eSystemErrorEx::seOpenFileFail);
            else // Если файл успешно открылся
            {
                m_json = nlohmann::json::parse(inFile, nullptr, false);

                if (m_json.is_discarded()) // Если при парсинге произошла ошибка
                {
                    Error = make_error_code(hmcommon::eSystemErrorEx::seReadFileFail);
                    m_json.clear();
                }
                else
                {
                    Error = checkCorrectStruct(); // Проверяем корректность считанной структуры

                    if (Error) // Если структура повреждена
                        m_json.clear(); // Очищаем считанные данные
                }

                inFile.close();
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
bool HMJsonDataStorage::is_open() const
{
    return !m_json.is_null();
}
//-----------------------------------------------------------------------------
void HMJsonDataStorage::close()
{
    if (is_open()) // Только при "открытом файле"
    {
        std::error_code Error = write(); // Вызываем запись

        if (Error)
            LOG_ERROR_EX(QString::fromStdString(Error.message()), this);

        m_json = nlohmann::json(); // Очищаем хранилище
    }
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::addUser(const std::shared_ptr<hmcommon::HMUser> inUser)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        if (!inUser) // Работаем только с валидным указателем
            Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
        else
        {
            Error = checkNewUserUnique(inUser); // Проверяем пользователья на уникальность

            if (!Error) // Если проверка на уникальность прошла успешно о том
            {   // Будем добавлять
                nlohmann::json NewUser = userToJson(inUser, Error); // Формируем объект пользователя

                if (!Error) // Если объект сформирован корректно
                {
                    Error = setUserContacts(inUser->m_uuid, std::make_shared<hmcommon::HMContactList>()); // Пытаемся сформировать пустой список контактов пользователя

                    if (!Error) // Если пустой список контактов добавлен корректно
                        m_json[J_USERS].push_back(NewUser); // Добавляем пользователя
                }
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::updateUser(const std::shared_ptr<hmcommon::HMUser> inUser)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        if (!inUser) // Работаем только с валидным указателем
            Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
        else
        {
            const std::string UUID = inUser->m_uuid.toString().toStdString(); // Единоразово запоминаем UUID
            auto UserIt = std::find_if(m_json[J_USERS].begin(), m_json[J_USERS].end(), [&](const nlohmann::json& UserObject)
            {
                std::error_code ValidError = m_validator.checkUser(UserObject); // Проверяем валидность объекта пользователя
                if (ValidError)
                {
                    LOG_WARNING_EX(QString::fromStdString(ValidError.message()), this);
                    return false; // Повреждённый пользователь игнорируется
                }
                else
                    return UserObject[J_USER_UUID].get<std::string>() == UUID; // Сравниваем UUID пользователя с заданым
            });

            if (UserIt == m_json[J_USERS].end()) // Если пользователь не найден
                Error = make_error_code(eDataStorageError::dsUserNotExists);
            else // Пользователь найден
            {
                nlohmann::json UpdateUser = userToJson(inUser, Error); // Формируем объект пользователя

                if (!Error) // Если объект сформирован корректно
                    *UserIt = UpdateUser; // Обновляем данные пользователя
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMUser> HMJsonDataStorage::findUserByUUID(const QUuid &inUserUUID, std::error_code &outErrorCode) const
{
    std::shared_ptr<hmcommon::HMUser> Result = nullptr;
    outErrorCode = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open())
        outErrorCode = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        const std::string UUID = inUserUUID.toString().toStdString(); // Единоразово запоминаем UUID
        // Ищим пользователя
        auto UserIt = std::find_if(m_json[J_USERS].cbegin(), m_json[J_USERS].cend(), [&](const nlohmann::json& UserObject)
        {
            std::error_code ValidError = m_validator.checkUser(UserObject); // Проверяем валидность объекта пользователя
            if (ValidError)
            {
                LOG_WARNING_EX(QString::fromStdString(ValidError.message()), this);
                return false; // Повреждённый пользователь игнорируется
            }
            else
                return UserObject[J_USER_UUID].get<std::string>() == UUID; // Сравниваем UUID пользователя с заданым
        });

        if (UserIt == m_json[J_USERS].cend()) // Если пользователь не найден
            outErrorCode = make_error_code(eDataStorageError::dsUserNotExists);
        else // Пользователь успешно найден
            Result = jsonToUser(*UserIt, outErrorCode); // Преобразуем JSON объект в пользователя
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMUser> HMJsonDataStorage::findUserByAuthentication(const QString &inLogin, const QByteArray &inPasswordHash, std::error_code &outErrorCode) const
{
    std::shared_ptr<hmcommon::HMUser> Result = nullptr;
    outErrorCode = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open())
        outErrorCode = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        const std::string Login = inLogin.toStdString(); // Единоразово запоминаем Login
        // Ищим пользователя
        auto UserIt = std::find_if(m_json[J_USERS].cbegin(), m_json[J_USERS].cend(), [&](const nlohmann::json& UserObject)
        {
            std::error_code ValidError = m_validator.checkUser(UserObject); // Проверяем валидность объекта пользователя
            if (ValidError)
            {
                LOG_WARNING_EX(QString::fromStdString(ValidError.message()), this);
                return false; // Повреждённый пользователь игнорируется
            }
            else
                return UserObject[J_USER_LOGIN].get<std::string>() == Login; // Сравниваем Login пользователя с заданым
        });

        if (UserIt == m_json[J_USERS].cend()) // Если пользователь не найден
            outErrorCode = make_error_code(eDataStorageError::dsUserNotExists);
        else // Пользователь найден
        {
            if (m_validator.jsonToByteArr((*UserIt)[J_USER_PASS]) != inPasswordHash)   // Срваниваем PasswordHash с заданным
                outErrorCode = make_error_code(eDataStorageError::dsUserPasswordIncorrect); // Хеш пароля не совпал
            else // Хеш пароля совпал
            {
                Result = jsonToUser(*UserIt, outErrorCode); // Преобразуем JSON объект в пользователя

                if (outErrorCode)
                    Result = nullptr;
            }
        }
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::removeUser(const QUuid& inUserUUID)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        const std::string UUID = inUserUUID.toString().toStdString(); // Единоразово запоминаем UUID
        // Ищим пользователя
        auto UserIt = std::find_if(m_json[J_USERS].cbegin(), m_json[J_USERS].cend(), [&](const nlohmann::json& UserObject)
        {
            std::error_code ValidError = m_validator.checkUser(UserObject); // Проверяем валидность объекта пользователя
            if (ValidError)
            {
                LOG_WARNING_EX(QString::fromStdString(ValidError.message()), this);
                return false; // Повреждённый пользователь игнорируется
            }
            else
                return UserObject[J_USER_UUID].get<std::string>() == UUID; // Сравниваем UUID пользователя с заданым
        });

        if (UserIt != m_json[J_USERS].cend()) // Если пользователь существует
        {
            Error = removeUserContacts(inUserUUID); // Пытаемся удалить списокконтактов пользователя

            if (!Error) // Если список контактов пользователей корректо удалён
                m_json[J_USERS].erase(UserIt); // Удаляем пользователя
        }

        // Если не найден пользователь на удаление то это не ошибка
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::addGroup(const std::shared_ptr<hmcommon::HMGroup> inGroup)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        if (!inGroup) // Работаем только с валидным указателем
            Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
        else
        {
            Error = checkNewGroupUnique(inGroup); // Проверяем группу на уникальность

            if (!Error) // Если проверка на уникальность прошла успешно
            {   // Будем добавлять
                nlohmann::json NewGroup = groupToJson(inGroup, Error); // Формируем объект группы

                if (!Error) // Если объект сформирован корректно
                    m_json[J_GROUPS].push_back(NewGroup); // Добавляем группу
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::updateGroup(const std::shared_ptr<hmcommon::HMGroup> inGroup)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        if (!inGroup) // Работаем только с валидным указателем
            Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
        else
        {
            const std::string UUID = inGroup->m_uuid.toString().toStdString(); // Единоразово запоминаем UUID
            auto UserIt = std::find_if(m_json[J_GROUPS].begin(), m_json[J_GROUPS].end(), [&](const nlohmann::json& GroupObject)
            {
                std::error_code ValidError = m_validator.checkGroup(GroupObject); // Проверяем валидность объекта группы
                if (ValidError)
                {
                    LOG_WARNING_EX(QString::fromStdString(ValidError.message()), this);
                    return false; // Повреждённая группа игнорируется
                }
                else
                    return GroupObject[J_GROUP_UUID].get<std::string>() == UUID; // Сравниваем UUID группы с заданым
            });

            if (UserIt == m_json[J_GROUPS].end()) // Если группа не найдена
                Error = make_error_code(eDataStorageError::dsGroupNotExists);
            else // Пользователь найден
            {
                nlohmann::json UpdateGroup = groupToJson(inGroup, Error); // Формируем объект группы

                if (!Error) // Если объект сформирован корректно
                    *UserIt = UpdateGroup; // Обновляем данные группы
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMGroup> HMJsonDataStorage::findGroupByUUID(const QUuid &inGroupUUID, std::error_code &outErrorCode) const
{
    std::shared_ptr<hmcommon::HMGroup> Result = nullptr;
    outErrorCode = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open())
        outErrorCode = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        const std::string UUID = inGroupUUID.toString().toStdString(); // Единоразово запоминаем UUID
        // Ищим пользователя
        auto GroupIt = std::find_if(m_json[J_GROUPS].cbegin(), m_json[J_GROUPS].cend(), [&](const nlohmann::json& GroupObject)
        {
            std::error_code ValidError = m_validator.checkGroup(GroupObject); // Проверяем валидность объекта группы
            if (ValidError)
            {
                LOG_WARNING_EX(QString::fromStdString(ValidError.message()), this);
                return false; // Повреждённая группа игнорируется
            }
            else
                return GroupObject[J_GROUP_UUID].get<std::string>() == UUID; // Сравниваем UUID группы с заданым
        });

        if (GroupIt == m_json[J_GROUPS].cend()) // Если группа не найдена
            outErrorCode = make_error_code(eDataStorageError::dsGroupNotExists);
        else // Группа найдена
        {
            Result = jsonToGroup(*GroupIt, outErrorCode); // Преобразуем JSON объект в группы

            if (outErrorCode) // Если ошибка построения группы
                Result = nullptr;
        }
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::removeGroup(const QUuid& inGroupUUID)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        const std::string UUID = inGroupUUID.toString().toStdString(); // Единоразово запоминаем UUID
        // Ищим пользователя
        auto UserIt = std::find_if(m_json[J_GROUPS].cbegin(), m_json[J_GROUPS].cend(), [&](const nlohmann::json& GroupObject)
        {
            std::error_code ValidError = m_validator.checkGroup(GroupObject); // Проверяем валидность объекта группы
            if (ValidError)
            {
                LOG_WARNING_EX(QString::fromStdString(ValidError.message()), this);
                return false; // Повреждённая группа игнорируется
            }
            else
                return GroupObject[J_GROUP_UUID].get<std::string>() == UUID; // Сравниваем UUID группы с заданым
        });

        if (UserIt != m_json[J_GROUPS].cend()) // Если группа существует
            m_json[J_GROUPS].erase(UserIt); // Удаляем группу
        // Если не найдена группа на удаление то это не ошибка
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::addMessage(const std::shared_ptr<hmcommon::HMGroupMessage> inMessage)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        if (!inMessage) // Работаем только с валидным указателем
            Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
        else
        {
            if (findMessage(inMessage->m_uuid, Error)) // Если сообщение с таким UUID уже существует
                Error = make_error_code(eDataStorageError::dsMessageAlreadyExists);
            else // Нет такого сообщения
            {   // Будем добавлять
                if (!findGroupByUUID(inMessage->m_group, Error)) // Добавляем только для существующей группы
                    Error = make_error_code(eDataStorageError::dsGroupNotExists);
                else
                {
                    nlohmann::json NewMessage = messageToJson(inMessage, Error); // Формируем объект сообщения

                    if (!Error) // Если объект сформирован корректно
                        m_json[J_MESSAGES].push_back(NewMessage); // Добавляем сообщение
                }
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::updateMessage(const std::shared_ptr<hmcommon::HMGroupMessage> inMessage)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        if (!inMessage) // Работаем только с валидным указателем
            Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
        else
        {
            const std::string UUID = inMessage->m_uuid.toString().toStdString(); // Единоразово запоминаем UUID
            auto MessageIt = std::find_if(m_json[J_MESSAGES].begin(), m_json[J_MESSAGES].end(), [&](const nlohmann::json& MessageObject)
            {
                std::error_code ValidError = m_validator.checkMessage(MessageObject); // Проверяем валидность объекта сообщения
                if (ValidError)
                {
                    LOG_WARNING_EX(QString::fromStdString(ValidError.message()), this);
                    return false; // Повреждённное сообщение игнорируется
                }
                else
                    return MessageObject[J_MESSAGE_UUID].get<std::string>() == UUID; // Сравниваем UUID сообщения с заданым
            });

            if (MessageIt == m_json[J_MESSAGES].end()) // Если сообщение не найдено
                Error = make_error_code(eDataStorageError::dsMessageNotExists);
            else // Сообщение найдено
            {
                nlohmann::json UpdateMessage = messageToJson(inMessage, Error); // Формируем объект сообщения

                if (!Error) // Если объект сформирован корректно
                    *MessageIt = UpdateMessage; // Обновляем данные сообщения
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMGroupMessage> HMJsonDataStorage::findMessage(const QUuid& inMessageUUID, std::error_code& outErrorCode) const
{
    std::shared_ptr<hmcommon::HMGroupMessage> Result = nullptr;
    outErrorCode = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open())
        outErrorCode = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        const std::string UUID = inMessageUUID.toString().toStdString(); // Единоразово запоминаем UUID
        // Ищим сообщение
        auto MessageIt = std::find_if(m_json[J_MESSAGES].cbegin(), m_json[J_MESSAGES].cend(), [&](const nlohmann::json& MessageObject)
        {
            std::error_code ValidError = m_validator.checkMessage(MessageObject); // Проверяем валидность объекта сообщения
            if (ValidError)
            {
                LOG_WARNING_EX(QString::fromStdString(ValidError.message()), this);
                return false; // Повреждённное сообщение игнорируется
            }
            else
                return MessageObject[J_MESSAGE_UUID].get<std::string>() == UUID; // Сравниваем UUID сообщения с заданым
        });

        if (MessageIt == m_json[J_MESSAGES].cend()) // Если сообщение не найдено
            outErrorCode = make_error_code(eDataStorageError::dsMessageNotExists);
        else // Сообщение найдено
        {
            Result = jsonToMessage(*MessageIt, outErrorCode); // Преобразуем JSON объект в сообщение

            if (outErrorCode)
                Result = nullptr;
        }
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::vector<std::shared_ptr<hmcommon::HMGroupMessage>> HMJsonDataStorage::findMessages(const QUuid& inGroupUUID, const hmcommon::MsgRange& inRange,  std::error_code& outErrorCode) const
{
    std::vector<std::shared_ptr<hmcommon::HMGroupMessage>> Result;
    outErrorCode = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open())
        outErrorCode = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        nlohmann::json FindedMessages = nlohmann::json::array();
        std::back_insert_iterator< nlohmann::json > InsertBackIt (FindedMessages);
        const std::string GroupUUID = inGroupUUID.toString().toStdString(); // Единоразово запоминаем UUID
        // Копируем сообщения группы за указанный период
        std::copy_if(m_json[J_MESSAGES].cbegin(), m_json[J_MESSAGES].cend(), InsertBackIt, [&](const nlohmann::json& MessageObject)
        {
            bool FRes = false;

            std::error_code ValidError = m_validator.checkMessage(MessageObject); // Проверяем валидность объекта сообщения
            if (ValidError)
            {
                LOG_WARNING_EX(QString::fromStdString(ValidError.message()), this);
                FRes = false; // Повреждённное сообщение игнорируется
            }
            else
            {
                if (MessageObject[J_MESSAGE_GROUP_UUID].get<std::string>() != GroupUUID) // Сообщение не входит в группу
                    FRes = false;
                else // Сообщение в группе
                {
                    QDateTime TimeBuff = QDateTime::fromString(QString::fromStdString(MessageObject[J_MESSAGE_REGDATE].get<std::string>()), TIME_FORMAT); // Получаем время создания сообщения
                    FRes = (TimeBuff >= inRange.m_from) && (TimeBuff <= inRange.m_to); // Проверяем что сообщение входит во временной диапазон
                }
            }

            return FRes;
        });

        if (FindedMessages.empty()) // Сообщения не найдены
            outErrorCode = make_error_code(eDataStorageError::dsMessageNotExists);
        else // Сообщения найдены
        {
            Result.reserve(FindedMessages.size());

            for (auto& Message : FindedMessages.items())
            {   // Все сообщения в списке гарантированно валидны
                std::error_code ConvertErr;
                std::shared_ptr<hmcommon::HMGroupMessage> MSG = jsonToMessage(Message.value(), ConvertErr); // Преобразуем объект в сообщение

                if (ConvertErr)
                    LOG_WARNING_EX(QString::fromStdString(ConvertErr.message()), this);
                else
                    Result.push_back(MSG); // Помещаем сообщение в итоговый контейнер
            }
            // Сортируем результаты по времени
            std::sort(Result.begin(), Result.end(), [](std::shared_ptr<hmcommon::HMGroupMessage>& PMes1, std::shared_ptr<hmcommon::HMGroupMessage>& PMes2)
            { return PMes1->m_createTime < PMes2->m_createTime; });

            if (outErrorCode)
                Result.clear();
        }
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::removeMessage(const QUuid& inMessageUUID, const QUuid& inGroupUUID)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        const std::string UUID = inMessageUUID.toString().toStdString(); // Единоразово запоминаем UUID
        const std::string GroupUUID = inGroupUUID.toString().toStdString(); // Единоразово запоминаем UUID группы
        // Ищим сообщение
        auto MessageIt = std::find_if(m_json[J_MESSAGES].cbegin(), m_json[J_MESSAGES].cend(), [&](const nlohmann::json& MessageObject)
        {
            std::error_code ValidError = m_validator.checkMessage(MessageObject); // Проверяем валидность объекта сообщения
            if (ValidError)
            {
                LOG_WARNING_EX(QString::fromStdString(ValidError.message()), this);
                return false; // Повреждённное сообщение игнорируется
            }
            else
                return ((MessageObject[J_MESSAGE_UUID].get<std::string>() == UUID) && // Сравниваем UUID сообщения с заданым
                        (MessageObject[J_MESSAGE_GROUP_UUID].get<std::string>() == GroupUUID));
        });

        if (MessageIt != m_json[J_MESSAGES].cend()) // Если сообщение существует
            m_json[J_MESSAGES].erase(MessageIt); // Удаляем сообщение

        // Если не найден пользователь на удаление то это не ошибка
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::setUserContacts(const QUuid& inUserUUID, const std::shared_ptr<hmcommon::HMContactList> inContacts)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        if (!inContacts) // Работаем только с валидным указателем
            Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
        else
        {
            Error = checkUserContactsUnique(inUserUUID, inContacts); // Проверяем уникальность списка контактов

            if (!Error) // Если подтвержена уникальность связи
            {
                nlohmann::json NewContactList = relationUCToJson(inUserUUID, inContacts, Error); // Формируем Json объект

                if (!Error) // Если  Json объект успешно софрмирован
                    m_json[J_RELATIONS][J_REL_USER_CONTACTS].push_back(NewContactList); // Добавялем список контактов
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::addUserContact(const QUuid& inUserUUID, const std::shared_ptr<hmcommon::HMUser> inContact)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успе

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        if (!inContact) // Работаем только с валидным указателем
            Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
        else
        {
            const std::string UUID = inUserUUID.toString().toStdString(); // Единоразово запоминаем UUID
            auto FindRes = std::find_if(m_json[J_RELATIONS][J_REL_USER_CONTACTS].begin(), m_json[J_RELATIONS][J_REL_USER_CONTACTS].end(),
                                        [&](const nlohmann::json& UserContactsObject)
            {
                std::error_code ValidError = m_validator.checkRelationUC(UserContactsObject); // Проверяем валидность объекта связи
                if (ValidError)
                {
                    LOG_WARNING_EX(QString::fromStdString(ValidError.message()), this);
                    return false; // Повреждённное сообщение игнорируется
                }
                else
                    return UserContactsObject[J_REL_UC_USER_UUID].get<std::string>() == UUID; // Сравниваем UUID связи с заданым
            });

            if (FindRes == m_json[J_RELATIONS][J_REL_USER_CONTACTS].end()) // Связь не найдена
                Error = make_error_code(eDataStorageError::dsRelationUCNotExists);
            else // Связь найдена
            {
                std::string StringUUID = inContact->m_uuid.toString().toStdString();
                auto FindContact = std::find(FindRes->cbegin(), FindRes->cend(), StringUUID);

                if (FindContact != FindRes->cend())
                    Error = make_error_code(eDataStorageError::dsRelationUCContactAlredyExists); // Контакт уже в списке
                else
                    FindRes.value().push_back(StringUUID); // Добавляем UUID в список контактов
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::removeUserContact(const QUuid& inUserUUID, const QUuid& inContactUUID)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        const std::string UUID = inUserUUID.toString().toStdString(); // Единоразово запоминаем UUID
        auto FindRes = std::find_if(m_json[J_RELATIONS][J_REL_USER_CONTACTS].begin(), m_json[J_RELATIONS][J_REL_USER_CONTACTS].end(),
                                    [&](const nlohmann::json& UserContactsObject)
        {
            std::error_code ValidError = m_validator.checkRelationUC(UserContactsObject); // Проверяем валидность объекта связи
            if (ValidError)
            {
                LOG_WARNING_EX(QString::fromStdString(ValidError.message()), this);
                return false; // Повреждённное сообщение игнорируется
            }
            else
                return UserContactsObject[J_REL_UC_USER_UUID].get<std::string>() == UUID; // Сравниваем UUID связи с заданым
        });

        if (FindRes == m_json[J_RELATIONS][J_REL_USER_CONTACTS].end()) // Связь не найдена
            Error = make_error_code(eDataStorageError::dsRelationUCNotExists);
        else // Связь найдена
        {
            std::string StringUUID = inContactUUID.toString().toStdString();
            auto FindContact = std::find(FindRes->cbegin(), FindRes->cend(), StringUUID);

            if (FindContact == FindRes->cend()) // Не удалось найти пользователя
                Error = make_error_code(eDataStorageError::dsRelationUCContactNotExists); // Контакт отсутствует в списке
            else
                FindRes->erase(FindContact); // Удаляем контакт из списка
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::removeUserContacts(const QUuid& inUserUUID)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        const std::string UUID = inUserUUID.toString().toStdString(); // Единоразово запоминаем UUID
        auto FindRes = std::find_if(m_json[J_RELATIONS][J_REL_USER_CONTACTS].begin(), m_json[J_RELATIONS][J_REL_USER_CONTACTS].end(),
                                    [&](const nlohmann::json& UserContactsObject)
        {
            std::error_code ValidError = m_validator.checkRelationUC(UserContactsObject); // Проверяем валидность объекта связи
            if (ValidError)
            {
                LOG_WARNING_EX(QString::fromStdString(ValidError.message()), this);
                return false; // Повреждённное сообщение игнорируется
            }
            else
                return UserContactsObject[J_REL_UC_USER_UUID].get<std::string>() == UUID; // Сравниваем UUID связи с заданым
        });

        if (FindRes == m_json[J_RELATIONS][J_REL_USER_CONTACTS].end()) // Связь не найдена
            Error = make_error_code(eDataStorageError::dsRelationUCNotExists);
        else // Связь найдена
            m_json[J_RELATIONS][J_REL_USER_CONTACTS].erase(FindRes); // Удаляем связь из списка
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMContactList> HMJsonDataStorage::getUserContactList(const QUuid& inUserUUID, std::error_code& outErrorCode) const
{
    std::shared_ptr<hmcommon::HMContactList> Result = nullptr;
    outErrorCode = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        outErrorCode = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        const std::string UUID = inUserUUID.toString().toStdString(); // Единоразово запоминаем UUID
        auto FindRes = std::find_if(m_json[J_RELATIONS][J_REL_USER_CONTACTS].begin(), m_json[J_RELATIONS][J_REL_USER_CONTACTS].end(),
                                    [&](const nlohmann::json& UserContactsObject)
        {
            std::error_code ValidError = m_validator.checkRelationUC(UserContactsObject); // Проверяем валидность объекта связи
            if (ValidError)
            {
                LOG_WARNING_EX(QString::fromStdString(ValidError.message()), this);
                return false; // Повреждённное сообщение игнорируется
            }
            else
                return UserContactsObject[J_REL_UC_USER_UUID].get<std::string>() == UUID; // Сравниваем UUID связи с заданым
        });

        if (FindRes == m_json[J_RELATIONS][J_REL_USER_CONTACTS].end()) // Связь не найдена
            outErrorCode = make_error_code(eDataStorageError::dsRelationUCNotExists);
        else // Связь найдена
        {
            Result = std::make_shared<hmcommon::HMContactList>();
            for (auto ContactUUID : FindRes.value()[J_REL_UC_CONTACTS].items())
            {
                std::shared_ptr<hmcommon::HMUser> Contact = findUserByUUID(QUuid::fromString(QString::fromStdString(ContactUUID.value().get<std::string>())), outErrorCode);

                if (outErrorCode) // Контакт не найден
                    break;
                else // Контакт найден
                {
                    outErrorCode = Result->addContact(Contact); // Добавляем контакт в список

                    if (outErrorCode) // Контакт не найден
                        break;
                }
            }

            if (outErrorCode)
                Result = nullptr;
        }
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::makeDefault()
{
    close();

    m_json[J_VERSION] = FORMAT_VESION;              // Задаём версию формата
    m_json[J_USERS] = nlohmann::json::array();      // Формируем пользователей

    nlohmann::json AdminUser = nlohmann::json::value_t::object;
    // Создаём пользователя "адиминистратора"
    AdminUser[J_USER_UUID] = QUuid::createUuid().toString().toStdString();
    AdminUser[J_USER_REGDATE] = QDateTime::currentDateTime().toString().toStdString();
    AdminUser[J_USER_LOGIN] = "Admin";

    QByteArray Hash = QCryptographicHash::hash(QString("password").toLocal8Bit(), QCryptographicHash::Md5);
    AdminUser[J_USER_PASS] = m_validator.byteArrToJson(Hash);

    AdminUser[J_USER_NAME] = "Admin";
    AdminUser[J_USER_SEX] = static_cast<std::uint32_t>(hmcommon::eSex::sNotSpecified);
    AdminUser[J_USER_BIRTHDAY] = "";

    m_json[J_USERS].push_back(AdminUser);

    m_json[J_GROUPS] = nlohmann::json::array();     // Формируем группы
    m_json[J_MESSAGES] = nlohmann::json::array();   // Формируем сообщения

    m_json[J_RELATIONS] = nlohmann::json::object(); // Формируем связи
    m_json[J_RELATIONS][J_REL_USER_CONTACTS] = nlohmann::json::array(); // Формируем связи пользователь-контакты
    m_json[J_RELATIONS][J_REL_GROUP_USERS] = nlohmann::json::array(); // Формируем связи группа-пользователи


//    nlohmann::json TR1 = nlohmann::json::object();
//    TR1[J_REL_UC_USER_UUID] = "UUID ПОЛЬЗОВАТЕЛЯ";
//    TR1[J_REL_UC_CONTACTS] = nlohmann::json::array();

//    TR1[J_REL_UC_CONTACTS].push_back("UUID КОНТАКТА_1");
//    TR1[J_REL_UC_CONTACTS].push_back("UUID КОНТАКТА_2");
//    TR1[J_REL_UC_CONTACTS].push_back("UUID КОНТАКТА_3");

//    nlohmann::json TR2 = nlohmann::json::object();
//    TR2[J_REL_UC_USER_UUID] = "UUID ПОЛЬЗОВАТЕЛЯ";
//    TR2[J_REL_UC_CONTACTS] = nlohmann::json::array();

//    TR2[J_REL_UC_CONTACTS].push_back("UUID КОНТАКТА_1");
//    TR2[J_REL_UC_CONTACTS].push_back("UUID КОНТАКТА_2");
//    TR2[J_REL_UC_CONTACTS].push_back("UUID КОНТАКТА_3");

//    m_json[J_RELATIONS][J_REL_USER_CONTACTS].push_back(TR1);
//    m_json[J_RELATIONS][J_REL_USER_CONTACTS].push_back(TR2);

//    nlohmann::json GU1 = nlohmann::json::object();
//    GU1[J_REL_GU_GROUP_UUID] = "UUID ГРУППЫ";
//    GU1[J_REL_GU_USERS] = nlohmann::json::array();

//    GU1[J_REL_GU_USERS].push_back("UUID ПОЛЬЗОВАТЕЛЯ_1");
//    GU1[J_REL_GU_USERS].push_back("UUID ПОЛЬЗОВАТЕЛЯ_2");
//    GU1[J_REL_GU_USERS].push_back("UUID ПОЛЬЗОВАТЕЛЯ_3");

//    nlohmann::json GU2 = nlohmann::json::object();
//    GU2[J_REL_GU_GROUP_UUID] = "UUID ГРУППЫ";
//    GU2[J_REL_GU_USERS] = nlohmann::json::array();

//    GU2[J_REL_GU_USERS].push_back("UUID ПОЛЬЗОВАТЕЛЯ_1");
//    GU2[J_REL_GU_USERS].push_back("UUID ПОЛЬЗОВАТЕЛЯ_2");
//    GU2[J_REL_GU_USERS].push_back("UUID ПОЛЬЗОВАТЕЛЯ_3");

//    m_json[J_RELATIONS][J_REL_GROUP_USERS].push_back(GU1);
//    m_json[J_RELATIONS][J_REL_GROUP_USERS].push_back(GU2);

    return write(); // Пишем сформированный файл
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::checkCorrectStruct() const
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        // Проверяем версию
        if (m_json.find(J_VERSION) == m_json.end() || m_json[J_VERSION].is_null() || m_json[J_VERSION].type() != nlohmann::json::value_t::string)
            Error = make_error_code(hmcommon::eSystemErrorEx::seIncorrecVersion);
        else
        {
            Error = checkUsers(); // Проверяем структуру пользователей
            if (!Error)
            {
                Error = checkGroups(); // Проверяем структуру групп
                if (!Error)
                {
                    Error = checkMessages(); // Проверяем структуру сообщений
                    if (!Error)
                    {
                        Error = checkRelationsUC(); // Проверяем структуру связей пользователи-контакты
                        if (!Error)
                            Error = checkRelationsGU(); // Проверяем структуру связей группа-пользователи
                    }
                }
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::checkUsers() const
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess);

    // Проверяем пользователей
    if (m_json.find(J_USERS) == m_json.end() || m_json[J_USERS].is_null() || m_json[J_USERS].type() != nlohmann::json::value_t::array)
        Error = make_error_code(hmcommon::eSystemErrorEx::seIncorretData);
    else
        for (auto& User : m_json[J_USERS].items()) // Перебиреам пользователей группы
        {
            Error = m_validator.checkUser(User.value());
            if (Error)
                break;
        }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::checkGroups() const
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess);

    // Проверяем группы
    if (m_json.find(J_GROUPS) == m_json.end() || m_json[J_GROUPS].is_null() || m_json[J_GROUPS].type() != nlohmann::json::value_t::array)
        Error = make_error_code(hmcommon::eSystemErrorEx::seIncorretData);
    else
        for (auto& Group : m_json[J_GROUPS].items()) // Перебиреам пользователей группы
        {
            Error = m_validator.checkGroup(Group.value());
            if (Error)
                break;
        }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::checkMessages() const
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess);

    // Проверяем сообщения
    if (m_json.find(J_MESSAGES) == m_json.end() || m_json[J_MESSAGES].is_null() || m_json[J_MESSAGES].type() != nlohmann::json::value_t::array)
        Error = make_error_code(hmcommon::eSystemErrorEx::seIncorretData);
    else
        for (auto& Message : m_json[J_MESSAGES].items()) // Перебиреам сообщения
        {
            Error = m_validator.checkMessage(Message.value());
            if (Error)
                break;
        }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::checkRelationsUC() const
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess);

    // Проверяем связи
    if (m_json.find(J_RELATIONS) == m_json.end() || m_json[J_RELATIONS].is_null() || m_json[J_RELATIONS].type() != nlohmann::json::value_t::object)
        Error = make_error_code(eDataStorageError::dsRelationsCorrupted);
    else
    {
        // Проверяем связи пользователь-контакты
        if (m_json[J_RELATIONS].find(J_REL_USER_CONTACTS) == m_json[J_RELATIONS].end() ||
                m_json[J_RELATIONS][J_REL_USER_CONTACTS].is_null() ||
                m_json[J_RELATIONS][J_REL_USER_CONTACTS].type() != nlohmann::json::value_t::array)
            Error = make_error_code(eDataStorageError::dsRelationUCCorrupted);
        else // Проверяем структуру связи пользователь-контакты
        {
            for (const auto& User : m_json[J_RELATIONS][J_REL_USER_CONTACTS].items())
            {
                Error = m_validator.checkUserContactsRelation(User.value()); // Проверяем валидность связи
                if (Error) // Если произошла ошибка
                    break; // Остальных не смотрим
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::checkRelationsGU() const
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess);

    // Проверяем связи
    if (m_json.find(J_RELATIONS) == m_json.end() || m_json[J_RELATIONS].is_null() || m_json[J_RELATIONS].type() != nlohmann::json::value_t::object)
        Error = make_error_code(eDataStorageError::dsRelationsCorrupted);
    else
    {
        // Проверяем связи группа-пользователи
        if (m_json[J_RELATIONS].find(J_REL_GROUP_USERS) == m_json[J_RELATIONS].end() ||
                m_json[J_RELATIONS][J_REL_GROUP_USERS].is_null() ||
                m_json[J_RELATIONS][J_REL_GROUP_USERS].type() != nlohmann::json::value_t::array)
            Error = make_error_code(eDataStorageError::dsRelationGUCorrupted);
        else // Проверяем структуру связи группа-пользователи
        {
            for (const auto& Group : m_json[J_RELATIONS][J_REL_GROUP_USERS].items())
            {
                Error = m_validator.checkGroupUsersRelation(Group.value()); // Проверяем валидность связи
                if (Error) // Если произошла ошибка
                    break; // Остальных не смотрим
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::write() const
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (std::filesystem::is_directory(m_jsonPath, Error))
    {
        if (!Error) // Не не зарегестрирована внутренняя ошибка filesystem
            Error = make_error_code(hmcommon::eSystemErrorEx::seObjectNotFile); // Задаём свою (Путь указывает не на является файлом)
    }
    else // Объект файл
    {
        std::ofstream outFile(m_jsonPath, std::ios_base::out);
        outFile << m_json; // Пишем JSON в файл

        if (outFile.bad())
            Error = make_error_code(hmcommon::eSystemErrorEx::seOutputOperationFail); // Помечаем как ошибку
        else
            Error = make_error_code(eDataStorageError::dsSuccess); // Помечаем как успешную запись

        outFile.flush();
        outFile.close();
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMUser> HMJsonDataStorage::jsonToUser(const nlohmann::json& inUserObject, std::error_code& outErrorCode) const
{
    std::shared_ptr<hmcommon::HMUser> Result = nullptr;
    outErrorCode = m_validator.checkUser(inUserObject); // Проверяем валидность пользователя

    if (!outErrorCode) // Если объект валиден
    {   // Инициализируем экземпляр класса пользователя
        Result = std::make_shared<hmcommon::HMUser>(QUuid::fromString(QString::fromStdString(inUserObject[J_USER_UUID].get<std::string>())),
                                                    QDateTime::fromString(QString::fromStdString(inUserObject[J_USER_REGDATE].get<std::string>()), TIME_FORMAT));

        Result->setLogin(QString::fromStdString(inUserObject[J_USER_LOGIN].get<std::string>()));
        Result->setPasswordHash(m_validator.jsonToByteArr(inUserObject[J_USER_PASS]));
        Result->setName(QString::fromStdString(inUserObject[J_USER_NAME].get<std::string>()));
        Result->setSex(inUserObject[J_USER_SEX].get<hmcommon::eSex>());
        Result->setBirthday(QDate::fromString(QString::fromStdString(inUserObject[J_USER_BIRTHDAY].get<std::string>())));
    }

    return Result;
}
//-----------------------------------------------------------------------------
nlohmann::json HMJsonDataStorage::userToJson(std::shared_ptr<hmcommon::HMUser> inUser, std::error_code& outErrorCode) const
{
    nlohmann::json Result = nlohmann::json::value_type::object();
    outErrorCode = make_error_code(hmcommon::eSystemErrorEx::seSuccess);

    if (!inUser) // Проверяем валидность указателя
        outErrorCode = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
    else // Объект валиден
    {
        Result[J_USER_UUID] = inUser->m_uuid.toString().toStdString();
        Result[J_USER_REGDATE] = inUser->m_registrationDate.toString(TIME_FORMAT).toStdString();
        Result[J_USER_LOGIN] = inUser->getLogin().toStdString();
        Result[J_USER_PASS] = m_validator.byteArrToJson(inUser->getPasswordHash());
        Result[J_USER_NAME] = inUser->getName().toStdString();
        Result[J_USER_SEX] = static_cast<std::uint32_t>(inUser->getSex());
        Result[J_USER_BIRTHDAY] = inUser->getBirthday().toString().toStdString();

        outErrorCode = m_validator.checkUser(Result); // Заранее проверяем корректность создаваемого пользователя

        if (outErrorCode)
            Result.clear();
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMGroup> HMJsonDataStorage::jsonToGroup(const nlohmann::json& inGroupObject, std::error_code& outErrorCode) const
{
    std::shared_ptr<hmcommon::HMGroup> Result = nullptr;
    outErrorCode = m_validator.checkGroup(inGroupObject); // Проверяем валидность группы

    if (!outErrorCode) // Если объект валиден
    {
        Result = std::make_shared<hmcommon::HMGroup>(QUuid::fromString(QString::fromStdString(inGroupObject[J_GROUP_UUID].get<std::string>())),
                                                     QDateTime::fromString(QString::fromStdString(inGroupObject[J_GROUP_REGDATE].get<std::string>()), TIME_FORMAT));

        Result->setName(QString::fromStdString(inGroupObject[J_GROUP_NAME].get<std::string>()));

        for (auto& UserUUID : inGroupObject[J_GROUP_USERS].items()) // Перебиреам пользователей группы
            Result->addUser(QUuid::fromString(QString::fromStdString(UserUUID.value().get<std::string>()))); // Добавляем пользователя в группу
    }

    return Result;
}
//-----------------------------------------------------------------------------
nlohmann::json HMJsonDataStorage::groupToJson(std::shared_ptr<hmcommon::HMGroup> inGroup, std::error_code& outErrorCode) const
{
    nlohmann::json Result = nlohmann::json::value_type::object();
    outErrorCode = make_error_code(hmcommon::eSystemErrorEx::seSuccess);

    if (!inGroup) // Проверяем валидность указателя
        outErrorCode = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
    else // Объект валиден
    {
        Result[J_GROUP_UUID] = inGroup->m_uuid.toString().toStdString();
        Result[J_GROUP_REGDATE] = inGroup->m_registrationDate.toString(TIME_FORMAT).toStdString();
        Result[J_GROUP_NAME] = inGroup->getName().toStdString();
        Result[J_GROUP_USERS] = nlohmann::json::value_type::array();

        for (std::size_t Index = 0; Index < inGroup->usersCount(); ++Index)
            Result[J_GROUP_USERS].push_back(inGroup->getUser(Index, outErrorCode).toString().toStdString());

        outErrorCode = m_validator.checkGroup(Result); // Заранее проверяем корректность создаваемой группы

        if (outErrorCode)
            Result.clear();
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMGroupMessage> HMJsonDataStorage::jsonToMessage(const nlohmann::json& inMessageObject, std::error_code& outErrorCode) const
{
    std::shared_ptr<hmcommon::HMGroupMessage> Result = nullptr;
    outErrorCode = m_validator.checkMessage(inMessageObject); // Проверяем валидность сообщения

    if (!outErrorCode) // Если объект валиден
    {
        Result = std::make_shared<hmcommon::HMGroupMessage>(QUuid::fromString(QString::fromStdString(inMessageObject[J_MESSAGE_UUID].get<std::string>())),
                                                            QUuid::fromString(QString::fromStdString(inMessageObject[J_MESSAGE_GROUP_UUID].get<std::string>())),
                                                            QDateTime::fromString(QString::fromStdString(inMessageObject[J_MESSAGE_REGDATE].get<std::string>()), TIME_FORMAT));

        hmcommon::MsgData Data(inMessageObject[J_MESSAGE_TYPE].get<hmcommon::eMsgType>(), m_validator.jsonToByteArr(inMessageObject[J_MESSAGE_DATA]));
        outErrorCode = Result->setMessage(Data);

        if (outErrorCode)
            Result = nullptr;
    }

    return Result;
}
//-----------------------------------------------------------------------------
nlohmann::json HMJsonDataStorage::messageToJson(std::shared_ptr<hmcommon::HMGroupMessage> inMessage, std::error_code& outErrorCode) const
{
    nlohmann::json Result = nlohmann::json::value_type::object();
    outErrorCode = make_error_code(hmcommon::eSystemErrorEx::seSuccess);

    if (!inMessage) // Проверяем валидность указателя
        outErrorCode = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
    else // Объект валиден
    {
        Result[J_MESSAGE_UUID] = inMessage->m_uuid.toString().toStdString();
        Result[J_MESSAGE_GROUP_UUID] = inMessage->m_group.toString().toStdString();
        Result[J_MESSAGE_REGDATE] = inMessage->m_createTime.toString(TIME_FORMAT).toStdString();

        hmcommon::MsgData Data = inMessage->getMesssage();

        Result[J_MESSAGE_TYPE] = static_cast<std::uint32_t>(Data.m_type);
        Result[J_MESSAGE_DATA] = m_validator.byteArrToJson(Data.m_data);

        outErrorCode = m_validator.checkMessage(Result); // Заранее проверяем корректность создаваемого сообщения

        if (outErrorCode)
            Result.clear();
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMContactList> HMJsonDataStorage::jsonToRelationUC(const nlohmann::json& inRelationUCObject, std::error_code& outErrorCode) const
{
    std::shared_ptr<hmcommon::HMContactList> Result = nullptr;
    outErrorCode = m_validator.checkRelationUC(inRelationUCObject); // Проверяем валидность связи

    if (!outErrorCode) // Если связь валидна
    {
        Result = std::make_shared<hmcommon::HMContactList>();

        for (auto& ContactUUID : inRelationUCObject[J_REL_UC_CONTACTS].items())
        {   // Перебираем все UUID'ы контактов
            std::shared_ptr<hmcommon::HMUser> Contact = findUserByUUID(QUuid::fromString(QString::fromStdString(ContactUUID.value().get<std::string>())), outErrorCode); // Поулчаем контакт

            if (outErrorCode) // Если не удалось получить контакт
                break; // Сбрасываем перебор
            else // Контакт успешно получен
            {
                outErrorCode = Result->addContact(Contact); // Добавляем контакт в список

                if (outErrorCode) // Если не удалось добавить контакт
                    break; // Сбрасываем перебор
            }
        }

        if (outErrorCode)
            Result = nullptr;
    }

    return Result;
}
//-----------------------------------------------------------------------------
nlohmann::json HMJsonDataStorage::relationUCToJson(const QUuid& inUserUUID, const std::shared_ptr<hmcommon::HMContactList> inRelationUC, std::error_code& outErrorCode) const
{
    nlohmann::json Result = nlohmann::json::value_type::object();
    outErrorCode = make_error_code(hmcommon::eSystemErrorEx::seSuccess);

    if (!inRelationUC) // Проверяем валидность указателя
        outErrorCode = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
    else // Объект валиден
    {
        Result[J_REL_UC_USER_UUID] = inUserUUID.toString().toStdString(); // Задаём UUID владельца
        Result[J_REL_UC_CONTACTS] = nlohmann::json::array(); // Создаём перечень контактов

        for (std::size_t Index = 0; Index < inRelationUC->contactsCount(); ++Index)
        {
            std::shared_ptr<hmcommon::HMUser> Contact = inRelationUC->getContact(Index, outErrorCode); // Пытаемся получит контакт

            if (outErrorCode)
                break;
            else
                Result[J_REL_UC_CONTACTS].push_back(Contact->m_uuid.toString().toStdString());
        }

        outErrorCode = m_validator.checkRelationUC(Result); // Заранее проверяем корректность создаваемой связи

        if (outErrorCode)
            Result.clear();
    }

    return Result;
}
//-----------------------------------------------------------------------------
