#include "datastorageerrorcategory.h"

using namespace errors;

//-----------------------------------------------------------------------------
DataStorageErrorCategory::DataStorageErrorCategory() : std::error_category()
{

}
//-----------------------------------------------------------------------------
const char* DataStorageErrorCategory::name() const noexcept
{
    return C_CATEGORY_DATASTORAGE_NAME.c_str();
}
//-----------------------------------------------------------------------------
std::string DataStorageErrorCategory::message(int inCode) const
{
    std::string Result;

    switch (static_cast<eDataStorageError>(inCode))
    {
        case eDataStorageError::dsSuccess:                          { Result = C_ERROR_SUCESS_TEXT; break; }

        case eDataStorageError::dsNotOpen:                          { Result = "Хранилище не открыто"; break; }

        case eDataStorageError::dsUserLoginAlreadyRegistered:       { Result = "Пользователь с таким логином уже зарегистрирован"; break; }
        case eDataStorageError::dsUserPasswordIncorrect:            { Result = "Не верный пароль"; break; }
        case eDataStorageError::dsUserNotExists:                    { Result = "Пользователь не существует"; break; }
        case eDataStorageError::dsUserAlreadyExists:                { Result = "Пользователь уже существует"; break; }
        case eDataStorageError::dsUserUUIDCorrupted:                { Result = "UUID пользователя повреждён"; break; }
        case eDataStorageError::dsUserRegistrationDateCorrupted:    { Result = "Дата регистрации пользователя повреждена"; break; }
        case eDataStorageError::dsUserLoginCorrupted:               { Result = "Логин пользователя повреждён"; break; }
        case eDataStorageError::dsUserPasswordHashCorrupted:        { Result = "Хеш пароля пользователя повреждён"; break; }
        case eDataStorageError::dsUserNameCorrupted:                { Result = "Имя пользователя повреждено"; break; }
        case eDataStorageError::dsUserSexCorrupted:                 { Result = "Пол пользователя повреждён"; break; }
        case eDataStorageError::dsUserBirthdayCorrupted:            { Result = "День рождения пользователя повреждён"; break; }
        case eDataStorageError::dsUserContactsCorrupted:            { Result = "Контакты пользователя повреждены"; break; }
        case eDataStorageError::dsUserGroupsCorrupted:              { Result = "Группы пользователя повреждены"; break; }
        case eDataStorageError::dsUserContactRelationAlredyExists:  { Result = "Связь пользователь-контакт уже существует"; break; }
        case eDataStorageError::dsUserContactRelationNotExists:     { Result = "Связь пользователь-контакт не существует"; break; }
        case eDataStorageError::dsUserContactAlredyExists:          { Result = "Контакт пользователя уже существует"; break; }
        case eDataStorageError::dsUserContactNotExists:             { Result = "Контакт пользователя не существует"; break; }
        case eDataStorageError::dsUserGroupsRelationAlredyExists:   { Result = "Связь пользователь-группы уже существует"; break; }
        case eDataStorageError::dsUserGroupsRelationNotExists:      { Result = "Связь пользователь-группы не существует"; break; }

        case eDataStorageError::dsGroupUUIDAlreadyRegistered:       { Result = "Группа с таким UUID уже зарегистрирована"; break; }
        case eDataStorageError::dsGroupNotExists:                   { Result = "Группа не существует"; break; }
        case eDataStorageError::dsGroupAlreadyExists:               { Result = "Группа уже существует"; break; }
        case eDataStorageError::dsGroupUUIDCorrupted:               { Result = "UUID группы повреждён"; break; }
        case eDataStorageError::dsGroupNameCorrupted:               { Result = "Имя группы повреждено"; break; }
        case eDataStorageError::dsGroupRegistrationDateCorrupted:   { Result = "Дата группы повреждена"; break; }
        case eDataStorageError::dsGroupUsersCorrupted:              { Result = "Участники группы повреждены"; break; }
        case eDataStorageError::dsGroupUserRelationAlredyExists:    { Result = "Связь группа-пользователь уже существует"; break; }
        case eDataStorageError::dsGroupUserRelationNotExists:       { Result = "Связь группа-пользователь не существует"; break; }

        case eDataStorageError::dsMessageNotExists:                 { Result = "Сообщение не существует"; break; }
        case eDataStorageError::dsMessageAlreadyExists:             { Result = "Сообщение уже существует"; break; }
        case eDataStorageError::dsMessageUUIDCorrupted:             { Result = "UUID сообщения повреждён"; break; }
        case eDataStorageError::dsMessageGroupUUIDCorrupted:        { Result = "UUID группы сообщения повреждён"; break; }
        case eDataStorageError::dsMessageRegistrationDateCorrupted: { Result = "Дата сообщения повреждена"; break; }
        case eDataStorageError::dsMessageTypeCorrupted:             { Result = "Тип сообщения повреждён"; break; }
        case eDataStorageError::dsMessageDataCorrupted:             { Result = "Данные сообщения повреждены"; break; }

        default: Result = C_ERROR_UNKNOWN_TEXT + std::to_string(inCode);
    }

    return Result;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
extern inline const errors::DataStorageErrorCategory &errors::ConversionDataStorageError_category()
{
  static errors::DataStorageErrorCategory category;
  return category;
}
//-----------------------------------------------------------------------------
errors::error_code make_error_code(errors::eDataStorageError inErrCode)
{
  return { static_cast<int>(inErrCode), errors::ConversionDataStorageError_category() };
}
//-----------------------------------------------------------------------------
