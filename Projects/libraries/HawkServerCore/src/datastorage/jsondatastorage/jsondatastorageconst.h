#ifndef JSONDATASTORAGECONST_H
#define JSONDATASTORAGECONST_H

#include <string>

#include <QDateTime>

//-----------------------------------------------------------------------------
// Базовые параметры
//-----------------------------------------------------------------------------
static const std::string FORMAT_VESION              = "0.0.0.1";
static const std::string UUID                       = "UUID";
static const std::string REGDATE                    = "registration_date";
static const auto TIME_FORMAT                       = Qt::ISODateWithMs;
//-----------------------------------------------------------------------------
// Корневые параметры структуры
//-----------------------------------------------------------------------------
static const std::string J_VERSION                  = "VERSION";
static const std::string J_USERS                    = "USERS";
static const std::string J_GROUPS                   = "GROUPS";
static const std::string J_MESSAGES                 = "MESSAGES";
static const std::string J_RELATIONS                = "RELATIONS";
//-----------------------------------------------------------------------------
// Пользователи
//-----------------------------------------------------------------------------
static const std::string J_USER_UUID                = UUID;
static const std::string J_USER_REGDATE             = REGDATE;
static const std::string J_USER_LOGIN               = "login";
static const std::string J_USER_PASS                = "password_hash";
static const std::string J_USER_NAME                = "name";
static const std::string J_USER_SEX                 = "sex";
static const std::string J_USER_BIRTHDAY            = "birthday";
//-----------------------------------------------------------------------------
// Группы
//-----------------------------------------------------------------------------
static const std::string J_GROUP_UUID               = UUID;
static const std::string J_GROUP_REGDATE            = REGDATE;
static const std::string J_GROUP_NAME               = "name";
//static const std::string J_GROUP_USERS              = "users";
//-----------------------------------------------------------------------------
// Сообщения
//-----------------------------------------------------------------------------
static const std::string J_MESSAGE_UUID             = UUID;
static const std::string J_MESSAGE_GROUP_UUID       = "GROUP_" + UUID;
static const std::string J_MESSAGE_REGDATE          = REGDATE;
static const std::string J_MESSAGE_TYPE             = "type";
static const std::string J_MESSAGE_DATA             = "data";
//-----------------------------------------------------------------------------
// Связи
//-----------------------------------------------------------------------------
// Пользователи-контакты
static const std::string J_REL_USER_CONTACTS        = "USER_CONTACTS";
static const std::string J_REL_UC_USER_UUID         = "user_" + UUID;
static const std::string J_REL_UC_CONTACTS          = "contacts";
// Группы-пользователи
static const std::string J_REL_GROUP_USERS          = "GROUP_USERS";
static const std::string J_REL_GU_GROUP_UUID        = "group_" + UUID;
static const std::string J_REL_GU_USERS             = "users";
//-----------------------------------------------------------------------------

#endif // JSONDATASTORAGECONST_H
