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

    switch (static_cast<DataStoragError>(inCode))
    {
        case DataStoragError::dsSuccess:                    { Result = tr("Успех").toStdString(); break; }

        case DataStoragError::dsNotOpen:                    { Result = tr("Хранилище не открыто").toStdString(); break; }

        case DataStoragError::dsUserNotExists:              { Result = tr("Пользователь не существует").toStdString(); break; }
        case DataStoragError::dsUserAlreadyExists:          { Result = tr("Пользователь уже существует").toStdString(); break; }
        case DataStoragError::dsUserUUIDCorrupted:          { Result = tr("UUID пользователя повреждён").toStdString(); break; }
        case DataStoragError::dsUserRegistrationDate:       { Result = tr("Дата регистрации пользователя повреждена").toStdString(); break; }
        case DataStoragError::dsUserLoginCorrupted:         { Result = tr("Логин пользователя повреждён").toStdString(); break; }
        case DataStoragError::dsUserPasswordHashCorrupted:  { Result = tr("Хеш пароля пользователя повреждён").toStdString(); break; }
        case DataStoragError::dsUserNameCorrupted:          { Result = tr("Имя пользователя повреждено").toStdString(); break; }
        case DataStoragError::dsUserSexCorrupted:           { Result = tr("Пол пользователя повреждён").toStdString(); break; }
        case DataStoragError::dsUserBirthday:               { Result = tr("День рождения пользователя повреждён").toStdString(); break; }

        case DataStoragError::dsGroupNotExists:             { Result = tr("Группа не существует").toStdString(); break; }
        case DataStoragError::dsGroupAlreadyExists:         { Result = tr("Группа уже существует").toStdString(); break; }
        case DataStoragError::dsGroupUUIDCorrupted:         { Result = tr("UUID группы повреждён").toStdString(); break; }

        default: Result = ( tr("Не известная ошибка с кодом: ") + QString::number(inCode) ).toStdString();
    }

    return Result;
}
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//DATASTORAGE_API_DECL
const hmservcommon::DataStorageErrorCategory &hmservcommon::ConversionErrc_category()
{
  static hmservcommon::DataStorageErrorCategory category;
  return category;
}
//-----------------------------------------------------------------------------
std::error_code make_error_code(hmservcommon::DataStoragError inErrCode)
{
  return {static_cast<int>(inErrCode), hmservcommon::ConversionErrc_category()};
}
//-----------------------------------------------------------------------------
