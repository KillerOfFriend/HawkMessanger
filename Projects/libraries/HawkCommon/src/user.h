#ifndef HMUSER_H
#define HMUSER_H

/**
 * @file user.h
 * @brief Содержит описание пользователя системы
 */

#include <memory>

#include "userinfo.h"
#include "userlist.h"
#include "grouplist.h"

namespace hmcommon
{
//-----------------------------------------------------------------------------
/**
 * @brief The HMUser class - Класс, описывающий пользователя системы
 *
 * @authors Alekseev_s
 * @date 07.01.2021
 */
class HMUser
{
public:

    /**
     * @brief HMUser - Конструктор по умолчанию
     */
    HMUser() = default;

    /**
     * @brief ~HMUser - Деструктор по умолчанию
     */
    ~HMUser() = default;

    std::shared_ptr<HMUserInfo> m_info = nullptr;   ///< Информация о пользователе
    HMUserInfoList m_contacts;                      ///< Перечень контатков пользователя
    HMGroupList m_groups;                           ///< Перечегь групп, в которые входит пользователь
};
//-----------------------------------------------------------------------------
}

#endif // HMUSER_H
