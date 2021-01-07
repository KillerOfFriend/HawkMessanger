#ifndef HMACCOUNT_H
#define HMACCOUNT_H

/**
 * @file account.h
 * @brief Содержит описание аккаунта пользователя
 */

#include <memory>
#include <vector>

#include "userinfo.h"
#include "groupinfo.h"
#include "userlist.h"

namespace hmcommon
{
//-----------------------------------------------------------------------------
/**
 * @brief The HMAccount class - Класс, описывающий аккаунт системы
 *
 * @authors Alekseev_s
 * @date 11.12.2020
 */
class HMAccount
{
public:

    HMAccount();
    ~HMAccount() = default;

    std::shared_ptr<HMUserInfo> m_userInfo = nullptr;           ///< Данные пользователя
    HMUserInfoList m_cotacts;                                   ///< Контакты пользователя

    std::vector<std::shared_ptr<HMGroupInfo>> m_groups;         ///< Группы пользователя

};
//-----------------------------------------------------------------------------
} // namespace hmcommon

#endif // HMACCOUNT_H
