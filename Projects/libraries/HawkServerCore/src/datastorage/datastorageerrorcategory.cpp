#include "datastorageerrorcategory.h"

using namespace hmservcommon::datastorage;

//-----------------------------------------------------------------------------
DataStorageErrorCategory::DataStorageErrorCategory() :
    QObject(nullptr),
    std::error_category()
{

}
//-----------------------------------------------------------------------------
const char* DataStorageErrorCategory::name() const noexcept
{
    static const std::string CategoryName = tr("Ошибки хранилища данных").toStdString();
    return CategoryName.c_str();
}
//-----------------------------------------------------------------------------
std::string DataStorageErrorCategory::message(int inCode) const
{
    std::string Result;

    switch (static_cast<eDataStorageError>(inCode))
    {
        case eDataStorageError::dsSuccess:                          { Result = tr("Успех").toStdString(); break; }

        case eDataStorageError::dsNotOpen:                          { Result = tr("Хранилище не открыто").toStdString(); break; }

        case eDataStorageError::dsUserLoginAlreadyRegistered:       { Result = tr("Пользователь с таким логином уже зарегистрирован").toStdString(); break; }
        case eDataStorageError::dsUserPasswordIncorrect:            { Result = tr("Не верный пароль").toStdString(); break; }
        case eDataStorageError::dsUserNotExists:                    { Result = tr("Пользователь не существует").toStdString(); break; }
        case eDataStorageError::dsUserAlreadyExists:                { Result = tr("Пользователь уже существует").toStdString(); break; }
        case eDataStorageError::dsUserUUIDCorrupted:                { Result = tr("UUID пользователя повреждён").toStdString(); break; }
        case eDataStorageError::dsUserRegistrationDateCorrupted:    { Result = tr("Дата регистрации пользователя повреждена").toStdString(); break; }
        case eDataStorageError::dsUserLoginCorrupted:               { Result = tr("Логин пользователя повреждён").toStdString(); break; }
        case eDataStorageError::dsUserPasswordHashCorrupted:        { Result = tr("Хеш пароля пользователя повреждён").toStdString(); break; }
        case eDataStorageError::dsUserNameCorrupted:                { Result = tr("Имя пользователя повреждено").toStdString(); break; }
        case eDataStorageError::dsUserSexCorrupted:                 { Result = tr("Пол пользователя повреждён").toStdString(); break; }
        case eDataStorageError::dsUserBirthdayCorrupted:            { Result = tr("День рождения пользователя повреждён").toStdString(); break; }
        case eDataStorageError::dsUserContactsCorrupted:            { Result = tr("Контакты пользователя повреждены").toStdString(); break; }
        case eDataStorageError::dsUserGroupsCorrupted:              { Result = tr("Группы пользователя повреждены").toStdString(); break; }
        case eDataStorageError::dsUserContactRelationAlredyExists:  { Result = tr("Связь пользователь-контакт уже существует").toStdString(); break; }
        case eDataStorageError::dsUserContactRelationNotExists:     { Result = tr("Связь пользователь-контакт не существует").toStdString(); break; }
        case eDataStorageError::dsUserContactAlredyExists:          { Result = tr("Контакт пользователя уже существует").toStdString(); break; }
        case eDataStorageError::dsUserContactNotExists:             { Result = tr("Контакт пользователя не существует").toStdString(); break; }
        case eDataStorageError::dsUserGroupsRelationAlredyExists:   { Result = tr("Связь пользователь-группы уже существует").toStdString(); break; }
        case eDataStorageError::dsUserGroupsRelationNotExists:      { Result = tr("Связь пользователь-группы не существует").toStdString(); break; }

        case eDataStorageError::dsGroupUUIDAlreadyRegistered:       { Result = tr("Группа с таким UUID уже зарегистрирована").toStdString(); break; }
        case eDataStorageError::dsGroupNotExists:                   { Result = tr("Группа не существует").toStdString(); break; }
        case eDataStorageError::dsGroupAlreadyExists:               { Result = tr("Группа уже существует").toStdString(); break; }
        case eDataStorageError::dsGroupUUIDCorrupted:               { Result = tr("UUID группы повреждён").toStdString(); break; }
        case eDataStorageError::dsGroupNameCorrupted:               { Result = tr("Имя группы повреждено").toStdString(); break; }
        case eDataStorageError::dsGroupRegistrationDateCorrupted:   { Result = tr("Дата группы повреждена").toStdString(); break; }
        case eDataStorageError::dsGroupUsersCorrupted:              { Result = tr("Участники группы повреждены").toStdString(); break; }
        case eDataStorageError::dsGroupUserRelationAlredyExists:    { Result = tr("Связь группа-пользователь уже существует").toStdString(); break; }
        case eDataStorageError::dsGroupUserRelationNotExists:       { Result = tr("Связь группа-пользователь не существует").toStdString(); break; }

        case eDataStorageError::dsMessageNotExists:                 { Result = tr("Сообщение не существует").toStdString(); break; }
        case eDataStorageError::dsMessageAlreadyExists:             { Result = tr("Сообщение уже существует").toStdString(); break; }
        case eDataStorageError::dsMessageUUIDCorrupted:             { Result = tr("UUID сообщения повреждён").toStdString(); break; }
        case eDataStorageError::dsMessageGroupUUIDCorrupted:        { Result = tr("UUID группы сообщения повреждён").toStdString(); break; }
        case eDataStorageError::dsMessageRegistrationDateCorrupted: { Result = tr("Дата сообщения повреждена").toStdString(); break; }
        case eDataStorageError::dsMessageTypeCorrupted:             { Result = tr("Тип сообщения повреждён").toStdString(); break; }
        case eDataStorageError::dsMessageDataCorrupted:             { Result = tr("Данные сообщения повреждены").toStdString(); break; }

        default: Result = ( tr("Не известная ошибка с кодом: ") + QString::number(inCode) ).toStdString();
}

    return Result;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
DATASTORAGE_API_DECL const hmservcommon::datastorage::DataStorageErrorCategory &hmservcommon::datastorage::ConversionErrc_category()
{
  static hmservcommon::datastorage::DataStorageErrorCategory category;
  return category;
}
//-----------------------------------------------------------------------------
std::error_code make_error_code(hmservcommon::datastorage::eDataStorageError inErrCode)
{
  return {static_cast<int>(inErrCode), hmservcommon::datastorage::ConversionErrc_category()};
}
//-----------------------------------------------------------------------------
