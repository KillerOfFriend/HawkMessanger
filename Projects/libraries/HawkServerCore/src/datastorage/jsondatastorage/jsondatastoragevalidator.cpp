#include "jsondatastoragevalidator.h"

#include <systemerrorex.h>

#include "jsondatastorageconst.h"
#include "datastorage/datastorageerrorcategory.h"

using namespace hmservcommon::datastorage;

//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorageValidator::checkUser(const nlohmann::json& inUserObject) const
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (inUserObject.find(J_USER_UUID) == inUserObject.end() || inUserObject[J_USER_UUID].is_null() || inUserObject[J_USER_UUID].type() != nlohmann::json::value_t::string)
        Error = make_error_code(eDataStorageError::dsUserUUIDCorrupted);

    if (inUserObject.find(J_USER_REGDATE) == inUserObject.end() || inUserObject[J_USER_REGDATE].is_null() || inUserObject[J_USER_REGDATE].type() != nlohmann::json::value_t::string)
        Error = make_error_code(eDataStorageError::dsUserRegistrationDateCorrupted);

    if (inUserObject.find(J_USER_LOGIN) == inUserObject.end() || inUserObject[J_USER_LOGIN].is_null() || inUserObject[J_USER_LOGIN].type() != nlohmann::json::value_t::string)
        Error = make_error_code(eDataStorageError::dsUserLoginCorrupted);

    if (inUserObject.find(J_USER_PASS) == inUserObject.end() || inUserObject[J_USER_PASS].is_null() || inUserObject[J_USER_PASS].type() != nlohmann::json::value_t::array)
        Error = make_error_code(eDataStorageError::dsUserPasswordHashCorrupted);

    if (inUserObject.find(J_USER_NAME) == inUserObject.end() || inUserObject[J_USER_NAME].is_null() || inUserObject[J_USER_NAME].type() != nlohmann::json::value_t::string)
        Error = make_error_code(eDataStorageError::dsUserNameCorrupted);

    if (inUserObject.find(J_USER_SEX) == inUserObject.end() || inUserObject[J_USER_SEX].is_null() || inUserObject[J_USER_SEX].type() != nlohmann::json::value_t::number_unsigned)
        Error = make_error_code(eDataStorageError::dsUserSexCorrupted);

    if (inUserObject.find(J_USER_BIRTHDAY) == inUserObject.end() || /*inUserObject[J_USER_BIRTHDAY].is_null() ||*/ inUserObject[J_USER_BIRTHDAY].type() != nlohmann::json::value_t::string)
        Error = make_error_code(eDataStorageError::dsUserBirthdayCorrupted);

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorageValidator::checkGroup(const nlohmann::json& inGroupObject) const
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (inGroupObject.find(J_GROUP_UUID) == inGroupObject.end() || inGroupObject[J_GROUP_UUID].is_null() || inGroupObject[J_GROUP_UUID].type() != nlohmann::json::value_t::string)
        Error = make_error_code(eDataStorageError::dsGroupUUIDCorrupted);

    if (inGroupObject.find(J_GROUP_REGDATE) == inGroupObject.end() || inGroupObject[J_GROUP_REGDATE].is_null() || inGroupObject[J_GROUP_REGDATE].type() != nlohmann::json::value_t::string)
        Error = make_error_code(eDataStorageError::dsGroupRegistrationDateCorrupted);

    if (inGroupObject.find(J_GROUP_NAME) == inGroupObject.end() || inGroupObject[J_GROUP_NAME].is_null() || inGroupObject[J_GROUP_NAME].type() != nlohmann::json::value_t::string)
        Error = make_error_code(eDataStorageError::dsGroupNameCorrupted);

    if (inGroupObject.find(J_GROUP_USERS) == inGroupObject.end() || inGroupObject[J_GROUP_USERS].is_null() || inGroupObject[J_GROUP_USERS].type() != nlohmann::json::value_t::array)
        Error = make_error_code(eDataStorageError::dsGroupUsersCorrupted);
    else // Список пользователей группы валиден
    {
        for (auto& UserUUID : inGroupObject[J_GROUP_USERS].items())
        {   // Перебираем всех пользователй группы
            if (!UserUUID.value().is_null() && UserUUID.value().type() != nlohmann::json::value_t::string)
            {   // Если хоть один повреждён, метим группу как повреждённую
                Error = make_error_code(eDataStorageError::dsGroupUsersCorrupted);
                break;
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorageValidator::checkMessage(const nlohmann::json& inMesssageObject) const
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (inMesssageObject.find(J_MESSAGE_UUID) == inMesssageObject.end() || inMesssageObject[J_MESSAGE_UUID].is_null() || inMesssageObject[J_MESSAGE_UUID].type() != nlohmann::json::value_t::string)
        Error = make_error_code(eDataStorageError::dsMessageUUIDCorrupted);

    if (inMesssageObject.find(J_MESSAGE_GROUP_UUID) == inMesssageObject.end() || inMesssageObject[J_MESSAGE_GROUP_UUID].is_null() || inMesssageObject[J_MESSAGE_GROUP_UUID].type() != nlohmann::json::value_t::string)
        Error = make_error_code(eDataStorageError::dsMessageGroupUUIDCorrupted);

    if (inMesssageObject.find(J_MESSAGE_REGDATE) == inMesssageObject.end() || inMesssageObject[J_MESSAGE_REGDATE].is_null() || inMesssageObject[J_MESSAGE_REGDATE].type() != nlohmann::json::value_t::string)
        Error = make_error_code(eDataStorageError::dsMessageRegistrationDateCorrupted);

    if (inMesssageObject.find(J_MESSAGE_TYPE) == inMesssageObject.end() || inMesssageObject[J_MESSAGE_TYPE].is_null() || inMesssageObject[J_MESSAGE_TYPE].type() != nlohmann::json::value_t::number_unsigned)
        Error = make_error_code(eDataStorageError::dsMessageTypeCorrupted);

    if (inMesssageObject.find(J_MESSAGE_DATA) == inMesssageObject.end() || inMesssageObject[J_MESSAGE_DATA].is_null() || inMesssageObject[J_MESSAGE_DATA].type() != nlohmann::json::value_t::array)
        Error = make_error_code(eDataStorageError::dsMessageDataCorrupted);

    return Error;
}
//-----------------------------------------------------------------------------
#include <HawkLog.h>
std::error_code HMJsonDataStorageValidator::checkUserContactsRelation(const nlohmann::json& inUCRelation) const
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    std::string Dump = inUCRelation.dump();
    LOG_DEBUG(QString::fromStdString(Dump));

    // Проверяем UUID пользователя
    if (inUCRelation.find(J_REL_UC_USER_UUID) == inUCRelation.end() ||
            inUCRelation[J_REL_UC_USER_UUID].is_null() ||
            inUCRelation[J_REL_UC_USER_UUID].type() != nlohmann::json::value_t::string)
    {
        Error = make_error_code(eDataStorageError::dsRelationUCUserUUIDCorrupted); // UUID пользователя повреждён
    }
    else // UUID пользователя валиден
    {
        // Проверяем список контактов
        if (inUCRelation.find(J_REL_UC_CONTACTS) == inUCRelation.end() ||
                /*User.value()[J_REL_UC_CONTACTS].is_null() ||*/
                inUCRelation[J_REL_UC_CONTACTS].type() != nlohmann::json::value_t::array)
        {
            Error = make_error_code(eDataStorageError::dsRelationUCContactsCorrupted); // Список контактов повреждён
        }
        else
        {
            // Проверяем каждый контакт пользователя
            for (const auto& Contact : inUCRelation[J_REL_UC_CONTACTS].items())
            {
                // Проверяем UUID контакта
                if (Contact.value().is_null() || Contact.value().type() != nlohmann::json::value_t::string)
                {
                    Error = make_error_code(eDataStorageError::dsRelationUCContactUUIDCorrupted); // UUID контакта повреждён
                    break; // Остальных не смотрим
                }
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorageValidator::checkGroupUsersRelation(const nlohmann::json& inGURelation) const
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    // Проверяем UUID группы
    if (inGURelation.find(J_REL_GU_GROUP_UUID) == inGURelation.end() ||
            inGURelation[J_REL_GU_GROUP_UUID].is_null() ||
            inGURelation[J_REL_GU_GROUP_UUID].type() != nlohmann::json::value_t::string)
    {
        Error = make_error_code(eDataStorageError::dsRelationGUGroupUUIDCorrupted); // UUID группы повреждён
    }
    else // UUID группы валиден
    {
        // Проверяем список пользователей
        if (inGURelation.find(J_REL_GU_USERS) == inGURelation.end() ||
                inGURelation[J_REL_GU_USERS].is_null() ||
                inGURelation[J_REL_GU_USERS].type() != nlohmann::json::value_t::array)
        {
            Error = make_error_code(eDataStorageError::dsRelationGUUsersCorrupted); // Список пользователей повреждён
        }
        else
        {
            // Проверяем каждого пользователя группы
            for (const auto& User : inGURelation[J_REL_GU_USERS].items())
            {
                // Проверяем UUID пользователя
                if (User.value().is_null() || User.value().type() != nlohmann::json::value_t::string)
                {
                    Error = make_error_code(eDataStorageError::dsRelationGUUserUUIDCorrupted); // UUID пользователя повреждён
                    break; // Остальных не смотрим
                }
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMUser> HMJsonDataStorageValidator::jsonToUser(const nlohmann::json& inUserObject, std::error_code& outErrorCode) const
{
    std::shared_ptr<hmcommon::HMUser> Result = nullptr;
    outErrorCode = checkUser(inUserObject); // Проверяем валидность пользователя

    if (!outErrorCode) // Если объект валиден
    {   // Инициализируем экземпляр класса пользователя
        Result = std::make_shared<hmcommon::HMUser>(QUuid::fromString(QString::fromStdString(inUserObject[J_USER_UUID].get<std::string>())),
                                                    QDateTime::fromString(QString::fromStdString(inUserObject[J_USER_REGDATE].get<std::string>()), TIME_FORMAT));

        Result->setLogin(QString::fromStdString(inUserObject[J_USER_LOGIN].get<std::string>()));
        Result->setPasswordHash(jsonToByteArr(inUserObject[J_USER_PASS]));
        Result->setName(QString::fromStdString(inUserObject[J_USER_NAME].get<std::string>()));
        Result->setSex(inUserObject[J_USER_SEX].get<hmcommon::eSex>());
        Result->setBirthday(QDate::fromString(QString::fromStdString(inUserObject[J_USER_BIRTHDAY].get<std::string>())));
    }

    return Result;
}
//-----------------------------------------------------------------------------
nlohmann::json HMJsonDataStorageValidator::userToJson(std::shared_ptr<hmcommon::HMUser> inUser, std::error_code& outErrorCode) const
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
        Result[J_USER_PASS] = byteArrToJson(inUser->getPasswordHash());
        Result[J_USER_NAME] = inUser->getName().toStdString();
        Result[J_USER_SEX] = static_cast<std::uint32_t>(inUser->getSex());
        Result[J_USER_BIRTHDAY] = inUser->getBirthday().toString().toStdString();

        outErrorCode = checkUser(Result); // Заранее проверяем корректность создаваемого пользователя

        if (outErrorCode)
            Result.clear();
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMGroup> HMJsonDataStorageValidator::jsonToGroup(const nlohmann::json& inGroupObject, std::error_code& outErrorCode) const
{
    std::shared_ptr<hmcommon::HMGroup> Result = nullptr;
    outErrorCode = checkGroup(inGroupObject); // Проверяем валидность группы

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
nlohmann::json HMJsonDataStorageValidator::groupToJson(std::shared_ptr<hmcommon::HMGroup> inGroup, std::error_code& outErrorCode) const
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

        outErrorCode = checkGroup(Result); // Заранее проверяем корректность создаваемой группы

        if (outErrorCode)
            Result.clear();
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMGroupMessage> HMJsonDataStorageValidator::jsonToMessage(const nlohmann::json& inMessageObject, std::error_code& outErrorCode) const
{
    std::shared_ptr<hmcommon::HMGroupMessage> Result = nullptr;
    outErrorCode = checkMessage(inMessageObject); // Проверяем валидность сообщения

    if (!outErrorCode) // Если объект валиден
    {
        Result = std::make_shared<hmcommon::HMGroupMessage>(QUuid::fromString(QString::fromStdString(inMessageObject[J_MESSAGE_UUID].get<std::string>())),
                                                            QUuid::fromString(QString::fromStdString(inMessageObject[J_MESSAGE_GROUP_UUID].get<std::string>())),
                                                            QDateTime::fromString(QString::fromStdString(inMessageObject[J_MESSAGE_REGDATE].get<std::string>()), TIME_FORMAT));

        hmcommon::MsgData Data(inMessageObject[J_MESSAGE_TYPE].get<hmcommon::eMsgType>(), jsonToByteArr(inMessageObject[J_MESSAGE_DATA]));
        outErrorCode = Result->setMessage(Data);

        if (outErrorCode)
            Result = nullptr;
    }

    return Result;
}
//-----------------------------------------------------------------------------
nlohmann::json HMJsonDataStorageValidator::messageToJson(std::shared_ptr<hmcommon::HMGroupMessage> inMessage, std::error_code& outErrorCode) const
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
        Result[J_MESSAGE_DATA] = byteArrToJson(Data.m_data);

        outErrorCode = checkMessage(Result); // Заранее проверяем корректность создаваемого сообщения

        if (outErrorCode)
            Result.clear();
    }

    return Result;
}
//-----------------------------------------------------------------------------
QByteArray HMJsonDataStorageValidator::jsonToByteArr(const nlohmann::json& inJson) const
{
    QByteArray Result;

    if (inJson.is_null() || inJson.type() != nlohmann::json::value_t::array)
        Result = QByteArray();
    else
    {
        std::vector<std::byte> ByteVector = inJson.get<std::vector<std::byte>>();
        Result = QByteArray(reinterpret_cast<const char*>(ByteVector.data()), ByteVector.size());
    }

    return Result;
}
//-----------------------------------------------------------------------------
nlohmann::json HMJsonDataStorageValidator::byteArrToJson(const QByteArray& inByteArr) const
{
    nlohmann::json Result = nlohmann::json::value_type::array();

    std::vector<std::byte> ByteVector;
    ByteVector.assign(reinterpret_cast<const std::byte*>(inByteArr.data()), reinterpret_cast<const std::byte*>(inByteArr.data()) + inByteArr.size());
    Result = ByteVector;

    return Result;
}
//-----------------------------------------------------------------------------
