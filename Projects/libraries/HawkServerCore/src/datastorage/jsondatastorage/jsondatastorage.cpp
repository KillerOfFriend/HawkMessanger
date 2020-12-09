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
    HMAbstractDataStorageFunctional(true), // Инициализируем предка с гарантированным хешированием
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
                makeDefault(); // Создадим пустой файл
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
                    else // Структура корректна
                    {
                        std::error_code CacheError = cache()->open(); // Пытаемся открыть кеш
                        if (CacheError) // Ошибки кеша обрабатывам отдельно
                            LOG_WARNING_EX(QString::fromStdString(CacheError.message()), this);
                    }
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

        cache()->close(); // Закрываем кеш
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

            if (!Error) // Если проверка на уникальность прошла успешно
            {   // Будем добавлять
                nlohmann::json NewUser = m_validator.userToJson(inUser, Error); // Формируем объект пользователя

                if (!Error) // Если объект сформирован корректно
                {
                    m_json[J_USERS].push_back(NewUser); // Добавляем пользователя

                    std::error_code CacheError = cache()->addUser(inUser);  // Довавляем пользователя в кеш
                    if (CacheError) // Ошибки кеша обрабатывам отдельно
                        LOG_WARNING_EX(QString::fromStdString(CacheError.message()), this);
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
                {
                    *UserIt = UpdateUser; // Обновляем данные пользователя

                    std::error_code CacheError = cache()->updateUser(inUser); // Обновляем данные о пользователе в кеше
                    if (CacheError) // Ошибки кеша обрабатывам отдельно
                        LOG_WARNING_EX(QString::fromStdString(CacheError.message()), this);
                }
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMUser> HMJsonDataStorage::findUserByUUID(const QUuid &inUserUUID, std::error_code &outErrorCode, const bool inWithContacts) const
{
    std::shared_ptr<hmcommon::HMUser> Result = nullptr;
    outErrorCode = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open())
        outErrorCode = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        std::error_code CacheError = make_error_code(eDataStorageError::dsSuccess); // Отдельный результат для поиска в хеше
        Result = cache()->findUserByUUID(inUserUUID, CacheError, inWithContacts); // Сначала ищим пользователя в кеше

        if (CacheError.value() == static_cast<int32_t>(eDataStorageError::dsUserNotExists)) // Если в кеше не удалось найти пользователя
        {   // Ищим в самом хранилище
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
            else // Пользователь найден
            {
                Result = m_validator.jsonToUser(*UserIt, outErrorCode); // Преобразуем JSON объект в пользователя

                if (inWithContacts) // Если требуется подготовить контакты для пользователя
                    outErrorCode = buildUserContacts(*UserIt, Result);

                if (outErrorCode) // Если ошибка построения пользователя
                    Result = nullptr;
                else // Если пользователь построен успешно
                {   // Добавим его в кеш
                    CacheError = cache()->addUser(Result);
                    if (CacheError) // Ошибки кеша обрабатывам отдельно
                        LOG_WARNING_EX(QString::fromStdString(CacheError.message()), this);
                }
            }
        }   // Поиск в самом хранилище
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMUser> HMJsonDataStorage::findUserByAuthentication(const QString &inLogin, const QByteArray &inPasswordHash, std::error_code &outErrorCode, const bool inWithContacts) const
{
    std::shared_ptr<hmcommon::HMUser> Result = nullptr;
    outErrorCode = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open())
        outErrorCode = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        std::error_code CacheError = make_error_code(eDataStorageError::dsSuccess); // Отдельный результат для поиска в хеше
        Result = cache()->findUserByAuthentication(inLogin, inPasswordHash, CacheError, inWithContacts); // Сначала ищим пользователя в кеше

        if (CacheError.value() == static_cast<int32_t>(eDataStorageError::dsUserNotExists)) // Если в кеше не удалось найти пользователя
        {   // Ищим в самом хранилище
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
                {
                    return UserObject[J_USER_LOGIN].get<std::string>() == Login &&      // Сравниваем Login пользователя с заданым
                            m_validator.jsonToByteArr(UserObject[J_USER_PASS]) == inPasswordHash;   // Срваниваем PasswordHash с заданным
                }
            });

            if (UserIt == m_json[J_USERS].cend()) // Если пользователь не найден
                outErrorCode = make_error_code(eDataStorageError::dsUserNotExists);
            else // Пользователь найден
            {
                Result = m_validator.jsonToUser(*UserIt, outErrorCode); // Преобразуем JSON объект в пользователя

                if (inWithContacts) // Если требуется подготовить контакты для пользователя
                    outErrorCode = buildUserContacts(*UserIt, Result);

                if (outErrorCode)
                    Result = nullptr;
                else // Если пользователь построен успешно
                {   // Добавим его в кеш
                    CacheError = cache()->addUser(Result);
                    if (CacheError) // Ошибки кеша обрабатывам отдельно
                        LOG_WARNING_EX(QString::fromStdString(CacheError.message()), this);
                }
            }
        }   // Поиск в самом хранилище
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
        {
            m_json[J_USERS].erase(UserIt); // Удаляем пользователя

            std::error_code CacheError = cache()->removeUser(inUserUUID); // Удаляем пользователя из кеша
            if (CacheError) // Ошибки кеша обрабатывам отдельно
                LOG_WARNING_EX(QString::fromStdString(CacheError.message()), this);
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
                nlohmann::json NewGroup = m_validator.groupToJson(inGroup, Error); // Формируем объект группы

                if (!Error) // Если объект сформирован корректно
                {
                    m_json[J_GROUPS].push_back(NewGroup); // Добавляем группу

                    std::error_code CacheError = cache()->addGroup(inGroup);  // Довавляем группу в кеш
                    if (CacheError) // Ошибки кеша обрабатывам отдельно
                        LOG_WARNING_EX(QString::fromStdString(CacheError.message()), this);
                }
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
                {
                    *UserIt = UpdateGroup; // Обновляем данные группы

                    std::error_code CacheError = cache()->updateGroup(inGroup); // Обновляем данные о группе в кеше
                    if (CacheError) // Ошибки кеша обрабатывам отдельно
                        LOG_WARNING_EX(QString::fromStdString(CacheError.message()), this);
                }
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
        std::error_code CacheError = make_error_code(eDataStorageError::dsSuccess); // Отдельный результат для поиска в хеше
        Result = cache()->findGroupByUUID(inGroupUUID, CacheError); // Сначала ищим группу в кеше

        if (CacheError.value() == static_cast<int32_t>(eDataStorageError::dsGroupNotExists)) // Если в кеше не удалось найти группу
        {   // Ищим в самом хранилище
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
                else // Если группа построена успешно
                {   // Добавим её в кеш
                    CacheError = cache()->addGroup(Result);
                    if (CacheError) // Ошибки кеша обрабатывам отдельно
                        LOG_WARNING_EX(QString::fromStdString(CacheError.message()), this);
                }
            }
        }   // Поиск в самом хранилище
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
        {
            m_json[J_GROUPS].erase(UserIt); // Удаляем группу

            std::error_code CacheError = cache()->removeGroup(inGroupUUID); // Удаляем группу из кеша
            if (CacheError) // Ошибки кеша обрабатывам отдельно
                LOG_WARNING_EX(QString::fromStdString(CacheError.message()), this);
        }
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
                if (findGroupByUUID(inMessage->m_group, Error)) // Добавляем только для существующей группы
                {
                    nlohmann::json NewMessage = m_validator.messageToJson(inMessage, Error); // Формируем объект сообщения

                    if (!Error) // Если объект сформирован корректно
                    {
                        m_json[J_MESSAGES].push_back(NewMessage); // Добавляем сообщение

                        std::error_code CacheError = cache()->addMessage(inMessage);  // Довавляем сообщение в кеш
                        if (CacheError) // Ошибки кеша обрабатывам отдельно
                            LOG_WARNING_EX(QString::fromStdString(CacheError.message()), this);
                    }
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
                {
                    *MessageIt = UpdateMessage; // Обновляем данные сообщения

                    std::error_code CacheError = cache()->updateMessage(inMessage);  // Обновляем сообщение в кеше
                    if (CacheError) // Ошибки кеша обрабатывам отдельно
                        LOG_WARNING_EX(QString::fromStdString(CacheError.message()), this);
                }
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMGroupMessage> HMJsonDataStorage::findMessage(const QUuid inMessageUUID, std::error_code& outErrorCode) const
{
    std::shared_ptr<hmcommon::HMGroupMessage> Result = nullptr;
    outErrorCode = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open())
        outErrorCode = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        std::error_code CacheError = make_error_code(eDataStorageError::dsSuccess); // Отдельный результат для поиска в хеше
        Result = cache()->findMessage(inMessageUUID, CacheError); // Сначала ищим сообщение в кеше

        if (CacheError.value() == static_cast<int32_t>(eDataStorageError::dsMessageNotExists)) // Если в кеше не удалось найти сообщение
        {   // Ищим в самом хранилище
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
                else // Если сообщение построено успешно
                {   // Добавим его в кеш
                    CacheError = cache()->addMessage(Result);
                    if (CacheError) // Ошибки кеша обрабатывам отдельно
                        LOG_WARNING_EX(QString::fromStdString(CacheError.message()), this);
                }
            }
        }   // Посик в самом хранилище
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::vector<std::shared_ptr<hmcommon::HMGroupMessage>> HMJsonDataStorage::findMessages(const QUuid inGroupUUID, const hmcommon::MsgRange& inRange,  std::error_code& outErrorCode) const
{
    std::vector<std::shared_ptr<hmcommon::HMGroupMessage>> Result;
    outErrorCode = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open())
        outErrorCode = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        std::error_code CacheError = make_error_code(eDataStorageError::dsSuccess); // Отдельный результат для поиска в хеше
        Result = cache()->findMessages(inGroupUUID, inRange, CacheError); // Сначала ищим сообщения в кеше

        if (CacheError.value() == static_cast<int32_t>(eDataStorageError::dsMessageNotExists)) // Если в кеше не удалось найти сообщение
        {   // Ищим в самом хранилище
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
                else // Если сообщения построены успешно
                {   // Добавим их в кеш
                    for (const auto& Message : Result)
                    {
                        CacheError = cache()->addMessage(Message);
                        if (CacheError) // Ошибки кеша обрабатывам отдельно
                            LOG_WARNING_EX(QString::fromStdString(CacheError.message()), this);
                    }
                }
            }
        }   // Поиск в самом хранилище
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::removeMessage(const QUuid inMessageUUID, const QUuid inGroupUUID)
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
        {
            m_json[J_MESSAGES].erase(MessageIt); // Удаляем сообщение

            std::error_code CacheError = cache()->removeMessage(inMessageUUID, inGroupUUID); // Удаляем сообщение из кеша
            if (CacheError) // Ошибки кеша обрабатывам отдельно
                LOG_WARNING_EX(QString::fromStdString(CacheError.message()), this);
        }
        // Если не найден пользователь на удаление то это не ошибка
    }

    return Error;
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
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::write() const
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (std::filesystem::is_directory(m_jsonPath, Error))
    {
        if (!Error) // Не зарегестрирована внутренняя ошибка filesystem
            Error = make_error_code(hmcommon::eSystemErrorEx::seObjectNotFile); // Задаём свою (Путь указывает не на является файлом)
    }
    else // Объект файл
    {
        std::ofstream outFile(m_jsonPath, std::ios_base::out);
        outFile << m_json; // Пишем JSON в файл
        outFile.flush();
        outFile.close();
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::buildUserContacts(const nlohmann::json& inJsonUser, std::shared_ptr<hmcommon::HMUser> outUser) const
{
    std::error_code Error = m_validator.checkUser(inJsonUser);

    if (!Error) // Если объект JSON валиден
    {
        if (!outUser) // Проверяем валидность указателя на возвращаемый объект
            Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
        else
        {
            for (auto& ContactUUID : inJsonUser[J_USER_CONTACTS].items()) // Перебираем все контакты пользователя
            {
                std::error_code ContactError = make_error_code(eDataStorageError::dsSuccess);

                QUuid Uuid(QString::fromStdString(ContactUUID.value().get<std::string>())); // Получаем UUID пользователя
                std::shared_ptr<hmcommon::HMUser> Contact = findUserByUUID(Uuid, ContactError, false); // Запрашиваем пользователя БЕЗ КОНТАКТОВ

                if (ContactError) // Проверяем каждый полученый контакт
                    LOG_ERROR_EX(QString::fromStdString(ContactError.message()), this); // Невалидные контакты будут проигнорированны
                else
                {
                    ContactError = outUser->m_contactList.addContact(Contact); // Добавляем контакт пользователю
                    if (ContactError)
                        LOG_ERROR_EX(QString::fromStdString(ContactError.message()), this);
                }
            }
        }
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
    AdminUser[J_USER_SEX] = hmcommon::eSex::sNotSpecified;
    AdminUser[J_USER_BIRTHDAY] = "";

    m_json[J_USERS].push_back(AdminUser);

    m_json[J_GROUPS] = nlohmann::json::array();     // Формируем группы
    m_json[J_MESSAGES] = nlohmann::json::array();   // Формируем сообщения

    return write(); // Пишем сформированный файл
}
//-----------------------------------------------------------------------------
