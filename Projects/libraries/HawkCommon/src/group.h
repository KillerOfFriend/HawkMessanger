#ifndef HMGROUP_H
#define HMGROUP_H

/**
 * @file group.h
 * @brief Содержит описание группы\чата
 */

#include <memory>

#include "groupinfo.h"
#include "userlist.h"

namespace hmcommon
{
//-----------------------------------------------------------------------------
/**
 * @brief The HMGroup class - Класс, описывающий группу\чат
 *
 * @authors Alekseev_s
 * @date 07.01.2021
 */
class HMGroup
{
public:

    /**
     * @brief HMGroup - Конструктор по умолчанию
     */
    HMGroup() = default;

    /**
     * @brief ~HMGroup - Деструктор по умолчанию
     */
    ~HMGroup() = default;

    std::shared_ptr<HMGroupInfo> m_info = nullptr;  ///< Информация о группе
    HMUserInfoList m_users;                         ///< Перечень участников группы

};
//-----------------------------------------------------------------------------
}

#endif // HMGROUP_H
