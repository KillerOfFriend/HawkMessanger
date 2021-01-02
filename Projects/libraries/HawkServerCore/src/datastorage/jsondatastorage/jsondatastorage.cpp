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

static nlohmann::json INVALID_NODE = nlohmann::json::object(); // Служебный пустой объект

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
                    m_json[J_USERS].push_back(NewUser); // Добавляем пользователя в конец

                    Error = onCreateUser(inUser->m_uuid);

                    if (Error) // Если при создании списка контактов поисходит ошибка
                        m_json[J_USERS].erase(m_json[J_USERS].size() - 1); // Удаляем полседнего добавленного пользователя
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
            nlohmann::json& User = findUser(inUser->m_uuid, Error); // Запрашиваем пользователя из хранилища

            if (!Error) // Если пользователь успешно найден
            {
                nlohmann::json UpdateUser = userToJson(inUser, Error); // Формируем объект пользователя
                if (!Error) // Если объект сформирован корректно
                    User.update(UpdateUser); // Обновляем данные пользователя
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
        const nlohmann::json& User = findConstUser(inUserUUID, outErrorCode); // Запрашиваем пользователя из хранилища
        if (!outErrorCode) // Если пользователь успешно найден
            Result = jsonToUser(User, outErrorCode); // Преобразуем JSON объект в пользователя
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
        const std::string UserUUID = inUserUUID.toString().toStdString(); // Единоразово запоминаем UUID
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
                return UserObject[J_USER_UUID].get<std::string>() == UserUUID; // Сравниваем UUID пользователя с заданым
        });

        if (UserIt != m_json[J_USERS].cend()) // Если пользователь существует
        {
            Error = onRemoveUser(inUserUUID);
            if (!Error) // Если список контактов пользователей корректо удалён
                m_json[J_USERS].erase(UserIt); // Удаляем пользователя
        }
        // Если не найден пользователь на удаление то это не ошибка
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::shared_ptr<std::set<QUuid>> HMJsonDataStorage::getUserGroups(const QUuid& inUserUUID, std::error_code& outErrorCode) const
{
    std::shared_ptr<std::set<QUuid>> Result = nullptr;
    outErrorCode = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        outErrorCode = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        const nlohmann::json& User = findConstUser(inUserUUID, outErrorCode); // Ищим пользователя

        if (!outErrorCode) // Если пользователь успешно найден
        {
            Result = std::make_shared<std::set<QUuid>>(); // Инициализируем результат
            for (const auto& GroupUUID : User[J_USER_GROUPS].items()) // Перебираем все группы пользователя
                Result->insert(QUuid::fromString(QString::fromStdString(GroupUUID.value().get<std::string>()))); // Добавляем UUID группы в результирующий перечень
        }
    }

    return Result;
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
            nlohmann::json& Group = findGroup(inGroup->m_uuid, Error);

            if (!Error)
            {
                nlohmann::json UpdateGroup = groupToJson(inGroup, Error); // Формируем объект группы
                if (!Error) // Если объект сформирован корректно
                    Group.update(UpdateGroup);
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
        const nlohmann::json& Group = findConstGroup(inGroupUUID, outErrorCode);

        if (!outErrorCode)
        {
            Result = jsonToGroup(Group, outErrorCode); // Преобразуем JSON объект в группы
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
        const std::string GroupUUID = inGroupUUID.toString().toStdString(); // Единоразово запоминаем UUID
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
                return GroupObject[J_GROUP_UUID].get<std::string>() == GroupUUID; // Сравниваем UUID группы с заданым
        });

        if (UserIt != m_json[J_GROUPS].cend()) // Если группа существует
            m_json[J_GROUPS].erase(UserIt); // Удаляем группу
        // Если не найдена группа на удаление то это не ошибка
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::setGroupUsers(const QUuid& inGroupUUID, const std::shared_ptr<std::set<QUuid>> inUsers)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        if (!inUsers) // Работаем только с валидным указателем
            Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
        else
        {
            Error = clearGroupUsers(inGroupUUID);

            if (!Error && !inUsers->empty()) // Если группа успешно очищена и перечень пользователей не пуст
            {
                std::vector<QUuid> SuccessfullyAdded(inUsers->size()); // Перечень успешно добавленых участников

                for (const QUuid& UserUUID : *inUsers)
                {
                    Error = addGroupUser(inGroupUUID, UserUUID); // Добавляем пользователя в группу

                    if (Error) // При добавлении произошла ошибка
                    {
                        while(!SuccessfullyAdded.empty()) // Пока перечень успешных добавлений не опустеет
                        {
                            std::error_code RemoveError = removeGroupUser(inGroupUUID, SuccessfullyAdded.back()); // Удаляем добавленную связь

                            if (!RemoveError) // Ошибку удаления обрабатываем отдельно
                                LOG_ERROR_EX(QString::fromStdString(RemoveError.message()), this);

                            SuccessfullyAdded.pop_back(); // Выкидываем удалённый UUID
                        }

                        break; // Останавливаем добавление учасников
                    }
                    else // Участник успешно добавлен в группу
                        SuccessfullyAdded.push_back(UserUUID);
                }
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::addGroupUser(const QUuid& inGroupUUID, const QUuid& inUserUUID)
{
    /*
     * Требуется
     * 1) Добавить пользователя в группу
     * 2) Добаветь группу в список групп пользователя
     */

    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        nlohmann::json& Group = findGroup(inGroupUUID, Error); // Ищим группу

        if (!Error) // Группа успешно найдена
        {
            std::string UserUUID = inUserUUID.toString().toStdString();
            // Ищим пользователя в группе
            auto FindUserRes = std::find_if(Group[J_GROUP_USERS].begin(), Group[J_GROUP_USERS].end(), [&UserUUID](const nlohmann::json& Val)
            { return Val.get<std::string>() == UserUUID; });

            if (FindUserRes != Group[J_GROUP_USERS].end()) // Если пользователь уже в группе
                Error = make_error_code(eDataStorageError::dsGroupUserRelationAlredyExists);
            else // Если пользователья в группе нет
            {
                nlohmann::json& User = findUser(inUserUUID, Error); // Ищим пользователя

                if (!Error) // Пользователь успешно найден
                {
                    std::string GroupUUID = inGroupUUID.toString().toStdString();

                    Group[J_GROUP_USERS].push_back(UserUUID); // Добавляем пользователя в группу
                    // Ищим группу в списке пользователя
                    auto FindGroupRes = std::find_if(User[J_USER_GROUPS].begin(), User[J_USER_GROUPS].end(), [&GroupUUID](const nlohmann::json& Val)
                    { return Val.get<std::string>() == GroupUUID; });

                    if (FindGroupRes == User[J_USER_GROUPS].end()) // Если группа не находится в списке пользователя (по другому быть не должно)
                        User[J_USER_GROUPS].push_back(GroupUUID); // Добавляем группу в список групп пользователя
                }
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::removeGroupUser(const QUuid& inGroupUUID, const QUuid& inUserUUID)
{
    /*
     * Требуется
     * 1) Удалить группу из списка групп пользователя
     * 2) Удалить пользователя из группы
     */

    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        nlohmann::json& Group = findGroup(inGroupUUID, Error); // Ищим группу

        if (!Error) // Группа успешно найдена
        {
            std::string UserUUID = inUserUUID.toString().toStdString();
            // Ищим пользователя в группе
            auto FindUserRes = std::find_if(Group[J_GROUP_USERS].begin(), Group[J_GROUP_USERS].end(), [&UserUUID](const nlohmann::json& Val)
            { return Val.get<std::string>() == UserUUID; });

            if (FindUserRes == Group[J_GROUP_USERS].end()) // Если пользователь уже в группе
                Error = make_error_code(eDataStorageError::dsGroupUserRelationNotExists);
            else // Если пользователья в группе нет
            {
                nlohmann::json& User = findUser(inUserUUID, Error); // Ищим пользователя

                if (!Error) // Пользователь успешно найден
                {
                    std::string GroupUUID = inGroupUUID.toString().toStdString();
                    // Ищим группу в списке пользователя
                    auto FindGroupRes = std::find_if(User[J_USER_GROUPS].begin(), User[J_USER_GROUPS].end(), [&GroupUUID](const nlohmann::json& Val)
                    { return Val.get<std::string>() == GroupUUID; });

                    if (FindGroupRes != User[J_USER_GROUPS].end()) // Если группа находится в списке пользователя (по другому быть не должно)
                        User[J_USER_GROUPS].erase(FindGroupRes); // Удаляем группу из списка групп пользователей

                    Group[J_GROUP_USERS].erase(FindUserRes); // Удаляем пользователя из группы
                }
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::clearGroupUsers(const QUuid& inGroupUUID)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        nlohmann::json& Group = findGroup(inGroupUUID, Error); // Ищим группу

        if (!Error && !Group[J_GROUP_USERS].empty()) // Группа успешно найдена и в ней есть участники
        {
            std::vector<QUuid> SuccessfullyRemoved(Group[J_GROUP_USERS].size()); // Перечень успешно удалённых участников

            while (!Group[J_GROUP_USERS].empty()) // Пока не удалим всех участников группы
            {
                QUuid UserUUID = QUuid::fromString(QString::fromStdString(Group[J_GROUP_USERS].items().begin().value().get<std::string>()));
                Error = removeGroupUser(inGroupUUID, UserUUID);

                if (Error) // Если не удалось удалить участника группы
                {
                    while(!SuccessfullyRemoved.empty()) // Пока перечень успешных удалённых не опустеет
                    {
                        std::error_code AddError = addGroupUser(inGroupUUID, SuccessfullyRemoved.back()); // Возвращаем удалённую связь

                        if (!AddError) // Ошибку добавления обрабатываем отдельно
                            LOG_ERROR_EX(QString::fromStdString(AddError.message()), this);

                        SuccessfullyRemoved.pop_back(); // Выкидываем восстановленный UUID
                    }

                    break; // Останавливаем удаление учасников
                }
                else // Участник успешно удалён
                    SuccessfullyRemoved.push_back(UserUUID);
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::shared_ptr<std::set<QUuid>> HMJsonDataStorage::getGroupUserList(const QUuid& inGroupUUID, std::error_code& outErrorCode) const
{
    std::shared_ptr<std::set<QUuid>> Result = nullptr;
    outErrorCode = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        outErrorCode = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        const nlohmann::json& Group = findConstGroup(inGroupUUID, outErrorCode); // Ищим группу

        if (!outErrorCode) // Если пользователь успешно найден
        {
            Result = std::make_shared<std::set<QUuid>>(); // Инициализируем результат
            for (const auto& UserUUID : Group[J_GROUP_USERS].items()) // Перебираем всех участников группы
                Result->insert(QUuid::fromString(QString::fromStdString(UserUUID.value().get<std::string>()))); // Добавляем UUID группы в результирующий перечень
        }
    }

    return Result;
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
            const std::string MessageUUID = inMessage->m_uuid.toString().toStdString(); // Единоразово запоминаем UUID
            auto MessageIt = std::find_if(m_json[J_MESSAGES].begin(), m_json[J_MESSAGES].end(), [&](const nlohmann::json& MessageObject)
            {
                std::error_code ValidError = m_validator.checkMessage(MessageObject); // Проверяем валидность объекта сообщения
                if (ValidError)
                {
                    LOG_WARNING_EX(QString::fromStdString(ValidError.message()), this);
                    return false; // Повреждённное сообщение игнорируется
                }
                else
                    return MessageObject[J_MESSAGE_UUID].get<std::string>() == MessageUUID; // Сравниваем UUID сообщения с заданым
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
        const std::string MessageUUID = inMessageUUID.toString().toStdString(); // Единоразово запоминаем UUID
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
                return MessageObject[J_MESSAGE_UUID].get<std::string>() == MessageUUID; // Сравниваем UUID сообщения с заданым
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
        const std::string MessageUUID = inMessageUUID.toString().toStdString(); // Единоразово запоминаем UUID
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
                return ((MessageObject[J_MESSAGE_UUID].get<std::string>() == MessageUUID) && // Сравниваем UUID сообщения с заданым
                        (MessageObject[J_MESSAGE_GROUP_UUID].get<std::string>() == GroupUUID));
        });

        if (MessageIt != m_json[J_MESSAGES].cend()) // Если сообщение существует
            m_json[J_MESSAGES].erase(MessageIt); // Удаляем сообщение

        // Если не найден пользователь на удаление то это не ошибка
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::setUserContacts(const QUuid& inUserUUID, const std::shared_ptr<hmcommon::HMUserList> inContacts)
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
            std::shared_ptr<hmcommon::HMUser> User = findUserByUUID(inUserUUID, Error); // Ищим пользователя, для которого добавляем связь

            if (!Error) // Если пользователь найден
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
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::addUserContact(const QUuid& inUserUUID, const std::shared_ptr<hmcommon::HMUser> inContact)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        if (!inContact) // Работаем только с валидным указателем
            Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
        else
        {
            if (inUserUUID == inContact->m_uuid) // Если пользователю пытаемся добавить в контакты его самого
                Error = make_error_code(hmcommon::eSystemErrorEx::seIncorretData);
            else // UUID'ы пользователя
            {
                Error = addContactUC(inUserUUID, inContact->m_uuid); // Связываем пользователя с контактом

                if (!Error) // Связали успешно
                {
                    Error = addContactUC(inContact->m_uuid, inUserUUID); // Связываем контакт с пользователем

                    if (Error) // Если вторая связь прошла с ошибкой
                        Error = removeContactUC(inUserUUID, inContact->m_uuid); // Рвём ПЕРВУЮ успешную связь
                }
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
        Error = removeContactUC(inUserUUID, inContactUUID); // Удаляем связь пользователя с контактом

        if (!Error) // Связь разорвана успешно
        {
            Error = removeContactUC(inContactUUID, inUserUUID); // Удаляем связь контакта с пользователем

            if (Error) // Если второе удаление связи прошло с ошибкой
                Error = addContactUC(inUserUUID, inContactUUID); // Восстанавливаем ПЕРВУЮ успешную связь
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
        const std::string UserUUID = inUserUUID.toString().toStdString(); // Единоразово запоминаем UUID
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
                return UserContactsObject[J_REL_UC_USER_UUID].get<std::string>() == UserUUID; // Сравниваем UUID связи с заданым
        });

        if (FindRes == m_json[J_RELATIONS][J_REL_USER_CONTACTS].end()) // Связь не найдена
            Error = make_error_code(eDataStorageError::dsRelationUCNotExists);
        else // Связь найдена
            m_json[J_RELATIONS][J_REL_USER_CONTACTS].erase(FindRes); // Удаляем связь из списка
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMUserList> HMJsonDataStorage::getUserContactList(const QUuid& inUserUUID, std::error_code& outErrorCode) const
{
    std::shared_ptr<hmcommon::HMUserList> Result = nullptr;
    outErrorCode = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        outErrorCode = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        const std::string UserUUID = inUserUUID.toString().toStdString(); // Единоразово запоминаем UUID
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
                return UserContactsObject[J_REL_UC_USER_UUID].get<std::string>() == UserUUID; // Сравниваем UUID связи с заданым
        });

        if (FindRes == m_json[J_RELATIONS][J_REL_USER_CONTACTS].end()) // Связь не найдена
            outErrorCode = make_error_code(eDataStorageError::dsRelationUCNotExists);
        else // Связь найдена
        {
            Result = std::make_shared<hmcommon::HMUserList>();

            for (auto ContactUUID : FindRes.value()[J_REL_UC_CONTACTS].items())
            {
                std::shared_ptr<hmcommon::HMUser> Contact = findUserByUUID(QUuid::fromString(QString::fromStdString(ContactUUID.value().get<std::string>())), outErrorCode);

                if (outErrorCode) // Контакт не найден
                    break;
                else // Контакт найден
                {
                    outErrorCode = Result->add(Contact); // Добавляем контакт в список

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
nlohmann::json& HMJsonDataStorage::findUser(const QUuid &inUserUUID, std::error_code& outErrorCode)
{
    outErrorCode = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open())
        outErrorCode = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        const std::string UserUUID = inUserUUID.toString().toStdString(); // Единоразово запоминаем UUID
        // Ищим пользователя
        auto UserIt = std::find_if(m_json[J_USERS].begin(), m_json[J_USERS].end(), [&](const nlohmann::json& UserObject)
        {
            std::error_code ValidError = m_validator.checkUser(UserObject); // Проверяем валидность объекта пользователя
            if (ValidError)
            {
                LOG_WARNING_EX(QString::fromStdString(ValidError.message()), this);
                return false; // Повреждённый пользователь игнорируется
            }
            else
                return UserObject[J_USER_UUID].get<std::string>() == UserUUID; // Сравниваем UUID пользователя с заданым
        });

        if (UserIt == m_json[J_USERS].end()) // Если пользователь не найден
            outErrorCode = make_error_code(eDataStorageError::dsUserNotExists);
        else // Пользователь успешно найден
            return UserIt.value(); // ЕДИНСТВЕННЫЙ УСПЕШНЫЙ СЛУЧАЙ
    }

    return INVALID_NODE; // ВО ВСЕХ ПРОВАЛЬНЫХ СЛУЧАЯХ ВЕРНЁМ НЕ ВАЛИДНЫЙ ОБЪЕКТ
}
//-----------------------------------------------------------------------------
const nlohmann::json& HMJsonDataStorage::findConstUser(const QUuid &inUserUUID, std::error_code& outErrorCode) const
{
    outErrorCode = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open())
        outErrorCode = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        const std::string UserUUID = inUserUUID.toString().toStdString(); // Единоразово запоминаем UUID
        // Ищим пользователя
        const auto UserIt = std::find_if(m_json[J_USERS].cbegin(), m_json[J_USERS].cend(), [&](const nlohmann::json& UserObject)
        {
            std::error_code ValidError = m_validator.checkUser(UserObject); // Проверяем валидность объекта пользователя
            if (ValidError)
            {
                LOG_WARNING_EX(QString::fromStdString(ValidError.message()), this);
                return false; // Повреждённый пользователь игнорируется
            }
            else
                return UserObject[J_USER_UUID].get<std::string>() == UserUUID; // Сравниваем UUID пользователя с заданым
        });

        if (UserIt == m_json[J_USERS].cend()) // Если пользователь не найден
            outErrorCode = make_error_code(eDataStorageError::dsUserNotExists);
        else // Пользователь успешно найден
            return UserIt.value(); // ЕДИНСТВЕННЫЙ УСПЕШНЫЙ ВАРИАНТ
    }

    return INVALID_NODE; // ВО ВСЕХ ПРОВАЛЬНЫХ СЛУЧАЯХ ВЕРНЁМ НЕ ВАЛИДНЫЙ ОБЪЕКТ
}
//-----------------------------------------------------------------------------
nlohmann::json& HMJsonDataStorage::findGroup(const QUuid &inGroupUUID, std::error_code& outErrorCode)
{
    outErrorCode = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open())
        outErrorCode = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        const std::string GroupUUID = inGroupUUID.toString().toStdString(); // Единоразово запоминаем UUID
        // Ищим пользователя
        auto GroupIt = std::find_if(m_json[J_GROUPS].begin(), m_json[J_GROUPS].end(), [&](const nlohmann::json& GroupObject)
        {
            std::error_code ValidError = m_validator.checkGroup(GroupObject); // Проверяем валидность объекта группы
            if (ValidError)
            {
                LOG_WARNING_EX(QString::fromStdString(ValidError.message()), this);
                return false; // Повреждённая группа игнорируется
            }
            else
                return GroupObject[J_GROUP_UUID].get<std::string>() == GroupUUID; // Сравниваем UUID группы с заданым
        });

        if (GroupIt == m_json[J_GROUPS].end()) // Если группа не найдена
            outErrorCode = make_error_code(eDataStorageError::dsGroupNotExists);
        else // Группа найдена
            return GroupIt.value();
    }

    return INVALID_NODE; // ВО ВСЕХ ПРОВАЛЬНЫХ СЛУЧАЯХ ВЕРНЁМ НЕ ВАЛИДНЫЙ ОБЪЕКТ
}
//-----------------------------------------------------------------------------
const nlohmann::json& HMJsonDataStorage::findConstGroup(const QUuid &inGroupUUID, std::error_code& outErrorCode) const
{
    outErrorCode = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open())
        outErrorCode = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        const std::string GroupUUID = inGroupUUID.toString().toStdString(); // Единоразово запоминаем UUID
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
                return GroupObject[J_GROUP_UUID].get<std::string>() == GroupUUID; // Сравниваем UUID группы с заданым
        });

        if (GroupIt == m_json[J_GROUPS].cend()) // Если группа не найдена
            outErrorCode = make_error_code(eDataStorageError::dsGroupNotExists);
        else // Группа найдена
            return GroupIt.value();
    }

    return INVALID_NODE; // ВО ВСЕХ ПРОВАЛЬНЫХ СЛУЧАЯХ ВЕРНЁМ НЕ ВАЛИДНЫЙ ОБЪЕКТ
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::onCreateUser(const QUuid &inUserUUID)
{
    /*
     * При создании пользователя требуется:
     * 1) Сформировать пустой список контактов
     */

    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    Error = setUserContacts(inUserUUID, std::make_shared<hmcommon::HMUserList>()); // Пытаемся сформировать пустой список контактов пользователя

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::onRemoveUser(const QUuid &inUserUUID)
{
    /*
     * При удалении пользователя требуется:
     * 1) Удалить список контактов (и пользователя из контактов контактов =) )
     * 2) Удалить пользователя из всех групп, в которых он соcтоит
     */

    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    std::shared_ptr<hmcommon::HMUserList> ContactList = getUserContactList(inUserUUID, Error); // Получаем список контактов

    if (Error) // Если не удалось получить список контактов
        LOG_WARNING_EX(QString::fromStdString(Error.message()), this);
    else // Список контактов успешно плучен
    {
        for (size_t Index = 0; Index < ContactList->count(); ++Index) // Перебираем все контакты пользователя
        {
            std::shared_ptr<hmcommon::HMUser> Contact = ContactList->get(Index, Error); // Получаем контакт
            if (Error) // Если не удалось получить контакт
                LOG_WARNING_EX(QString::fromStdString(Error.message()), this);
            else // Контакт успешно получен
                Error = removeUserContact(inUserUUID, Contact->m_uuid); // Удаляем контакт (с обеих сторон)
        }
    }

    Error = removeUserContacts(inUserUUID); // Пытаемся удалить список контактов пользователя

    // TODO Реализовать получение всех групп, в которых пользователь состоит
    // TODO Реализовать удаление пользователя из групп, в которых он состоит

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::makeDefault()
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    close();

    m_json[J_VERSION] = FORMAT_VESION;                  // Задаём версию формата

    m_json[J_USERS] = nlohmann::json::array();          // Формируем пользователей
    m_json[J_GROUPS] = nlohmann::json::array();         // Формируем группы
    m_json[J_MESSAGES] = nlohmann::json::array();       // Формируем сообщения

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

    // Формируем пользователя администратора
    std::shared_ptr<hmcommon::HMUser> AdminUser = std::make_shared<hmcommon::HMUser>(QUuid::createUuid());
    AdminUser->setName("Administrator");
    AdminUser->setLogin("Admin@gmail.com");
    AdminUser->setPassword("password");
    AdminUser->setSex(hmcommon::eSex::sNotSpecified);

    Error = addUser(AdminUser); // Добавляем администратора

    if (!Error) // Если админимтратор сформирован корректно
        Error = write(); // Пишем сформированный файл

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::addContactUC(const QUuid& inUserUUID, const QUuid& inContactUUID)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        const std::string StringUserUUID = inUserUUID.toString().toStdString(); // Единоразово запоминаем UUID
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
                return UserContactsObject[J_REL_UC_USER_UUID].get<std::string>() == StringUserUUID; // Сравниваем UUID связи с заданым
        });

        if (FindRes == m_json[J_RELATIONS][J_REL_USER_CONTACTS].end()) // Связь не найдена
            Error = make_error_code(eDataStorageError::dsRelationUCNotExists);
        else // Связь найдена
        {
            std::shared_ptr<hmcommon::HMUser> Contact = findUserByUUID(inContactUUID, Error); // Пытаемся найти пользователя-контакт

            if (!Error) // Если не удалось найти контакт среди пользователей
            {
                const std::string StringContactUUID = inContactUUID.toString().toStdString();
                auto FindContact = std::find(FindRes.value()[J_REL_UC_CONTACTS].cbegin(), FindRes.value()[J_REL_UC_CONTACTS].cend(), StringContactUUID); // Ищим контакт в списке контактов

                if (FindContact != FindRes.value()[J_REL_UC_CONTACTS].cend()) // Если контакт уже добавлен в список
                    Error = make_error_code(eDataStorageError::dsRelationUCContactAlredyExists); // Контакт уже в списке
                else
                    FindRes.value()[J_REL_UC_CONTACTS].push_back(StringContactUUID); // Добавляем UUID в список контактов
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::removeContactUC(const QUuid& inUserUUID, const QUuid& inContactUUID)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        const std::string StringUserUUID = inUserUUID.toString().toStdString(); // Единоразово запоминаем UUID
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
                return UserContactsObject[J_REL_UC_USER_UUID].get<std::string>() == StringUserUUID; // Сравниваем UUID связи с заданым
        });

        if (FindRes == m_json[J_RELATIONS][J_REL_USER_CONTACTS].end()) // Связь не найдена
            Error = make_error_code(eDataStorageError::dsRelationUCNotExists);
        else // Связь найдена
        {
            const std::string StringContactUUID = inContactUUID.toString().toStdString();
            auto FindContact = std::find(FindRes.value()[J_REL_UC_CONTACTS].cbegin(), FindRes.value()[J_REL_UC_CONTACTS].cend(), StringContactUUID); // Ищим контакт в списке контактов

            if (FindContact == FindRes.value()[J_REL_UC_CONTACTS].cend()) // Не удалось найти пользователя
                Error = make_error_code(eDataStorageError::dsRelationUCContactNotExists); // Контакт отсутствует в списке
            else
                FindRes.value()[J_REL_UC_CONTACTS].erase(FindContact); // Удаляем контакт из списка
        }
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
//                        if (!Error)
//                            Error = checkRelationsGU(); // Проверяем структуру связей группа-пользователи
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
//std::error_code HMJsonDataStorage::checkRelationsGU() const
//{
//    std::error_code Error = make_error_code(eDataStorageError::dsSuccess);

//    // Проверяем связи
//    if (m_json.find(J_RELATIONS) == m_json.end() || m_json[J_RELATIONS].is_null() || m_json[J_RELATIONS].type() != nlohmann::json::value_t::object)
//        Error = make_error_code(eDataStorageError::dsRelationsCorrupted);
//    else
//    {
//        // Проверяем связи группа-пользователи
//        if (m_json[J_RELATIONS].find(J_REL_GROUP_USERS) == m_json[J_RELATIONS].end() ||
//                m_json[J_RELATIONS][J_REL_GROUP_USERS].is_null() ||
//                m_json[J_RELATIONS][J_REL_GROUP_USERS].type() != nlohmann::json::value_t::array)
//            Error = make_error_code(eDataStorageError::dsRelationGUCorrupted);
//        else // Проверяем структуру связи группа-пользователи
//        {
//            for (const auto& Group : m_json[J_RELATIONS][J_REL_GROUP_USERS].items())
//            {
//                Error = m_validator.checkGroupUsersRelation(Group.value()); // Проверяем валидность связи
//                if (Error) // Если произошла ошибка
//                    break; // Остальных не смотрим
//            }
//        }
//    }

//    return Error;
//}
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
        // Так же создаём пустые массивы контактов и групп
        Result[J_USER_CONTACTS] = nlohmann::json::array();
        Result[J_USER_GROUPS] = nlohmann::json::array();

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
        // Так же создаём пустой массив пользователей группы
        Result[J_GROUP_USERS] = nlohmann::json::array();

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
std::shared_ptr<hmcommon::HMUserList> HMJsonDataStorage::jsonToRelationUC(const nlohmann::json& inRelationUCObject, std::error_code& outErrorCode) const
{
    std::shared_ptr<hmcommon::HMUserList> Result = nullptr;
    outErrorCode = m_validator.checkRelationUC(inRelationUCObject); // Проверяем валидность связи

    if (!outErrorCode) // Если связь валидна
    {
        Result = std::make_shared<hmcommon::HMUserList>();

        for (auto& ContactUUID : inRelationUCObject[J_REL_UC_CONTACTS].items())
        {   // Перебираем все UUID'ы контактов
            std::shared_ptr<hmcommon::HMUser> Contact = findUserByUUID(QUuid::fromString(QString::fromStdString(ContactUUID.value().get<std::string>())), outErrorCode); // Поулчаем контакт

            if (outErrorCode) // Если не удалось получить контакт
                break; // Сбрасываем перебор
            else // Контакт успешно получен
            {
                outErrorCode = Result->add(Contact); // Добавляем контакт в список

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
nlohmann::json HMJsonDataStorage::relationUCToJson(const QUuid& inUserUUID, const std::shared_ptr<hmcommon::HMUserList> inRelationUC, std::error_code& outErrorCode) const
{
    nlohmann::json Result = nlohmann::json::value_type::object();
    outErrorCode = make_error_code(hmcommon::eSystemErrorEx::seSuccess);

    if (!inRelationUC) // Проверяем валидность указателя
        outErrorCode = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
    else // Объект валиден
    {
        Result[J_REL_UC_USER_UUID] = inUserUUID.toString().toStdString(); // Задаём UUID владельца
        Result[J_REL_UC_CONTACTS] = nlohmann::json::array(); // Создаём перечень контактов

        for (std::size_t Index = 0; Index < inRelationUC->count(); ++Index)
        {
            std::shared_ptr<hmcommon::HMUser> Contact = inRelationUC->get(Index, outErrorCode); // Пытаемся получит контакт

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
