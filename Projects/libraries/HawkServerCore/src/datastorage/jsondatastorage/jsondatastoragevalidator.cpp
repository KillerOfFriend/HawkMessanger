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

    if (inUserObject.find(J_USER_CONTACTS) == inUserObject.end() || inUserObject[J_USER_CONTACTS].is_null() || inUserObject[J_USER_CONTACTS].type() != nlohmann::json::value_t::array)
        Error = make_error_code(eDataStorageError::dsUserContactsCorrupted);
    else
        for (const auto& ContactUUID : inUserObject[J_USER_CONTACTS].items()) // Перебираем все контакты пользователя
            if (ContactUUID.value().is_null() || ContactUUID.value().type() != nlohmann::json::value_t::string)
            { Error = make_error_code(eDataStorageError::dsUserContactsCorrupted); break; } // Если хоть один повреждён, метим польхователя как повреждённого

    if (inUserObject.find(J_USER_GROUPS) == inUserObject.end() || inUserObject[J_USER_GROUPS].is_null() || inUserObject[J_USER_GROUPS].type() != nlohmann::json::value_t::array)
        Error = make_error_code(eDataStorageError::dsUserGroupsCorrupted);
    else
        for (const auto& GroupUUID : inUserObject[J_USER_GROUPS].items()) // Перебираем все группы пользователя
            if (GroupUUID.value().is_null() || GroupUUID.value().type() != nlohmann::json::value_t::string)
            { Error = make_error_code(eDataStorageError::dsUserGroupsCorrupted); break; } // Если хоть одна повреждена, метим польхователя как повреждённого

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
        for (auto& UserUUID : inGroupObject[J_GROUP_USERS].items()) // Перебираем всех пользователй группы
            if (UserUUID.value().is_null() || UserUUID.value().type() != nlohmann::json::value_t::string)
            { Error = make_error_code(eDataStorageError::dsGroupUsersCorrupted); break; } // Если хоть один повреждён, метим группу как повреждённую

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
