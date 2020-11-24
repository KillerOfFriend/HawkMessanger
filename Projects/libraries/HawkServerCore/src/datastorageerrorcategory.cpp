#include "datastorageerrorcategory.h"

using namespace hmservcommon;

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

    switch (static_cast<eDataStoragError>(inCode))
    {
        case eDataStoragError::dsSuccess:                           { Result = tr("Успех").toStdString(); break; }

        case eDataStoragError::dsNotOpen:                           { Result = tr("Хранилище не открыто").toStdString(); break; }

        case eDataStoragError::dsUserNotExists:                     { Result = tr("Пользователь не существует").toStdString(); break; }
        case eDataStoragError::dsUserAlreadyExists:                 { Result = tr("Пользователь уже существует").toStdString(); break; }
        case eDataStoragError::dsUserUUIDCorrupted:                 { Result = tr("UUID пользователя повреждён").toStdString(); break; }
        case eDataStoragError::dsUserRegistrationDateCorrupted:     { Result = tr("Дата регистрации пользователя повреждена").toStdString(); break; }
        case eDataStoragError::dsUserLoginCorrupted:                { Result = tr("Логин пользователя повреждён").toStdString(); break; }
        case eDataStoragError::dsUserPasswordHashCorrupted:         { Result = tr("Хеш пароля пользователя повреждён").toStdString(); break; }
        case eDataStoragError::dsUserNameCorrupted:                 { Result = tr("Имя пользователя повреждено").toStdString(); break; }
        case eDataStoragError::dsUserSexCorrupted:                  { Result = tr("Пол пользователя повреждён").toStdString(); break; }
        case eDataStoragError::dsUserBirthday:                      { Result = tr("День рождения пользователя повреждён").toStdString(); break; }

        case eDataStoragError::dsGroupNotExists:                    { Result = tr("Группа не существует").toStdString(); break; }
        case eDataStoragError::dsGroupAlreadyExists:                { Result = tr("Группа уже существует").toStdString(); break; }
        case eDataStoragError::dsGroupUUIDCorrupted:                { Result = tr("UUID группы повреждён").toStdString(); break; }
        case eDataStoragError::dsGroupNameCorrupted:                { Result = tr("Имя группы повреждено").toStdString(); break; }
        case eDataStoragError::dsGroupRegistrationDateCorrupted:    { Result = tr("Дата группы пользователя повреждена").toStdString(); break; }

        default: Result = ( tr("Не известная ошибка с кодом: ") + QString::number(inCode) ).toStdString();
    }

    return Result;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
DATASTORAGE_API_DECL const hmservcommon::DataStorageErrorCategory &hmservcommon::ConversionErrc_category()
{
  static hmservcommon::DataStorageErrorCategory category;
  return category;
}
//-----------------------------------------------------------------------------
std::error_code make_error_code(hmservcommon::eDataStoragError inErrCode)
{
  return {static_cast<int>(inErrCode), hmservcommon::ConversionErrc_category()};
}
//-----------------------------------------------------------------------------
