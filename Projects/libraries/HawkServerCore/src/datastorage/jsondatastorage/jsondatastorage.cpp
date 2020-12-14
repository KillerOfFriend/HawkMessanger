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
                nlohmann::json NewUser = m_validator.userToJson(inUser, Error); // Формируем объект пользователя

                if (!Error) // Если объект сформирован корректно
                    m_json[J_USERS].push_back(NewUser); // Добавляем пользователя
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
                std::error_code Error = m_validator.checkUser(UserObject); // Проверяем валидность объекта пользователя
                if (Error)
                {
                    LOG_WARNING_EX(QString::fromStdString(Error.message()), this);
                    return false; // Повреждённый пользователь игнорируется
                }
                else
                    return UserObject[J_USER_UUID].get<std::string>() == UUID; // Сравниваем UUID пользователя с заданым
            });

            if (UserIt == m_json[J_USERS].end()) // Если пользователь не найден
                Error = make_error_code(eDataStorageError::dsUserNotExists);
            else // Пользователь найден
            {
                nlohmann::json UpdateUser = m_validator.userToJson(inUser, Error); // Формируем объект пользователя

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
            std::error_code Error = m_validator.checkUser(UserObject); // Проверяем валидность объекта пользователя
            if (Error)
            {
                LOG_WARNING_EX(QString::fromStdString(Error.message()), this);
                return false; // Повреждённый пользователь игнорируется
            }
            else
                return UserObject[J_USER_UUID].get<std::string>() == UUID; // Сравниваем UUID пользователя с заданым
        });

        if (UserIt == m_json[J_USERS].cend()) // Если пользователь не найден
            outErrorCode = make_error_code(eDataStorageError::dsUserNotExists);
        else // Пользователь успешно найден
            Result = m_validator.jsonToUser(*UserIt, outErrorCode); // Преобразуем JSON объект в пользователя
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
            std::error_code Error = m_validator.checkUser(UserObject); // Проверяем валидность объекта пользователя
            if (Error)
            {
                LOG_WARNING_EX(QString::fromStdString(Error.message()), this);
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
                Result = m_validator.jsonToUser(*UserIt, outErrorCode); // Преобразуем JSON объект в пользователя

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
            std::error_code Error = m_validator.checkUser(UserObject); // Проверяем валидность объекта пользователя
            if (Error)
            {
                LOG_WARNING_EX(QString::fromStdString(Error.message()), this);
                return false; // Повреждённый пользователь игнорируется
            }
            else
                return UserObject[J_USER_UUID].get<std::string>() == UUID; // Сравниваем UUID пользователя с заданым
        });

        if (UserIt != m_json[J_USERS].cend()) // Если пользователь существует
            m_json[J_USERS].erase(UserIt); // Удаляем пользователя

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
                nlohmann::json NewGroup = m_validator.groupToJson(inGroup, Error); // Формируем объект группы

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
                std::error_code Error = m_validator.checkGroup(GroupObject); // Проверяем валидность объекта группы
                if (Error)
                {
                    LOG_WARNING_EX(QString::fromStdString(Error.message()), this);
                    return false; // Повреждённая группа игнорируется
                }
                else
                    return GroupObject[J_GROUP_UUID].get<std::string>() == UUID; // Сравниваем UUID группы с заданым
            });

            if (UserIt == m_json[J_GROUPS].end()) // Если группа не найдена
                Error = make_error_code(eDataStorageError::dsGroupNotExists);
            else // Пользователь найден
            {
                nlohmann::json UpdateGroup = m_validator.groupToJson(inGroup, Error); // Формируем объект группы

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
            std::error_code Error = m_validator.checkGroup(GroupObject); // Проверяем валидность объекта группы
            if (Error)
            {
                LOG_WARNING_EX(QString::fromStdString(Error.message()), this);
                return false; // Повреждённая группа игнорируется
            }
            else
                return GroupObject[J_GROUP_UUID].get<std::string>() == UUID; // Сравниваем UUID группы с заданым
        });

        if (GroupIt == m_json[J_GROUPS].cend()) // Если группа не найдена
            outErrorCode = make_error_code(eDataStorageError::dsGroupNotExists);
        else // Группа найдена
        {
            Result = m_validator.jsonToGroup(*GroupIt, outErrorCode); // Преобразуем JSON объект в группы

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
            std::error_code Error = m_validator.checkGroup(GroupObject); // Проверяем валидность объекта группы
            if (Error)
            {
                LOG_WARNING_EX(QString::fromStdString(Error.message()), this);
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
                    nlohmann::json NewMessage = m_validator.messageToJson(inMessage, Error); // Формируем объект сообщения

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
                std::error_code Error = m_validator.checkMessage(MessageObject); // Проверяем валидность объекта сообщения
                if (Error)
                {
                    LOG_WARNING_EX(QString::fromStdString(Error.message()), this);
                    return false; // Повреждённное сообщение игнорируется
                }
                else
                    return MessageObject[J_MESSAGE_UUID].get<std::string>() == UUID; // Сравниваем UUID сообщения с заданым
            });

            if (MessageIt == m_json[J_MESSAGES].end()) // Если сообщение не найдено
                Error = make_error_code(eDataStorageError::dsMessageNotExists);
            else // Сообщение найдено
            {
                nlohmann::json UpdateMessage = m_validator.messageToJson(inMessage, Error); // Формируем объект сообщения

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
            std::error_code Error = m_validator.checkMessage(MessageObject); // Проверяем валидность объекта сообщения
            if (Error)
            {
                LOG_WARNING_EX(QString::fromStdString(Error.message()), this);
                return false; // Повреждённное сообщение игнорируется
            }
            else
                return MessageObject[J_MESSAGE_UUID].get<std::string>() == UUID; // Сравниваем UUID сообщения с заданым
        });

        if (MessageIt == m_json[J_MESSAGES].cend()) // Если сообщение не найдено
            outErrorCode = make_error_code(eDataStorageError::dsMessageNotExists);
        else // Сообщение найдено
        {
            Result = m_validator.jsonToMessage(*MessageIt, outErrorCode); // Преобразуем JSON объект в сообщение

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

            std::error_code Error = m_validator.checkMessage(MessageObject); // Проверяем валидность объекта сообщения
            if (Error)
            {
                LOG_WARNING_EX(QString::fromStdString(Error.message()), this);
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
                std::shared_ptr<hmcommon::HMGroupMessage> MSG = m_validator.jsonToMessage(Message.value(), ConvertErr); // Преобразуем объект в сообщение

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
            std::error_code Error = m_validator.checkMessage(MessageObject); // Проверяем валидность объекта сообщения
            if (Error)
            {
                LOG_WARNING_EX(QString::fromStdString(Error.message()), this);
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
std::vector<QUuid> HMJsonDataStorage::getUserContactsIDList(const QUuid& inUserUUID,  std::error_code& outErrorCode) const
{
    std::vector<QUuid> Result;
    outErrorCode = make_error_code(eDataStorageError::dsSuccess);

    const std::string UUID = inUserUUID.toString().toStdString(); // Единоразово запоминаем UUID
    // Ищим связи пользователя по UUID
    auto RelationIt = std::find_if(m_json[J_RELATIONS][J_REL_USER_CONTACTS].cbegin(), m_json[J_RELATIONS][J_REL_USER_CONTACTS].cend(), [&](const nlohmann::json& UserRelationObject)
    {
        std::error_code Error = m_validator.checkUserContactsRelation(UserRelationObject);
        if (Error)
        {
            LOG_WARNING_EX(QString::fromStdString(Error.message()), this);
            return false; // Повреждённая связь игнорируется
        }
        else
            return UserRelationObject[J_REL_UC_USER_UUID].get<std::string>() == UUID;
    });

    if (RelationIt == m_json[J_RELATIONS][J_REL_USER_CONTACTS].cend()) // Связь не найдена
        outErrorCode = make_error_code(eDataStorageError::dsRelationUCNotExists);
    else // Связь найдена
        for (const auto& Contact : RelationIt->items()) // Перебираем все контакты связи
            Result.emplace_back(QUuid::fromString(QString::fromStdString(Contact.value().get<std::string>())));

    return Result;
}
//-----------------------------------------------------------------------------
std::vector<QUuid> HMJsonDataStorage::getGroupUserIDList(const QUuid& inGroupUUID,  std::error_code& outErrorCode) const
{
    std::vector<QUuid> Result;
    outErrorCode = make_error_code(eDataStorageError::dsSuccess);

    const std::string UUID = inGroupUUID.toString().toStdString(); // Единоразово запоминаем UUID
    // Ищим связи пользователя по UUID
    auto RelationIt = std::find_if(m_json[J_RELATIONS][J_REL_GROUP_USERS].cbegin(), m_json[J_RELATIONS][J_REL_GROUP_USERS].cend(), [&](const nlohmann::json& GroupRelationObject)
    {
        std::error_code Error = m_validator.checkGroupUsersRelation(GroupRelationObject);
        if (Error)
        {
            LOG_WARNING_EX(QString::fromStdString(Error.message()), this);
            return false; // Повреждённая связь игнорируется
        }
        else
            return GroupRelationObject[J_REL_GU_GROUP_UUID].get<std::string>() == UUID;
    });

    if (RelationIt == m_json[J_RELATIONS][J_REL_GROUP_USERS].cend()) // Связь не найдена
        outErrorCode = make_error_code(eDataStorageError::dsRelationGUNotExists);
    else // Связь найдена
        for (const auto& User : RelationIt->items()) // Перебираем всех поьльзователей связи
            Result.emplace_back(QUuid::fromString(QString::fromStdString(User.value().get<std::string>())));

    return Result;
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
