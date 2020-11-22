#ifndef GROUP_H
#define GROUP_H

/**
 * @file group.h
 * @brief Содержит описание группы\чата
 */

#include <QUuid>

#include "user.h"

namespace hmcommon
{
//-----------------------------------------------------------------------------
/**
 * @brief The HMGroup class - Класс, описывающий группу пользователей системы
 *
 * @authors Alekseev_s
 * @date 08.11.2020
 */
class HMGroup
{
public:

    /**
     * @brief HMGroup - Инициализирующий конструктор
     * @param inUuid - Uuid группы
     */
    HMGroup(const QUuid& inUuid);

    const QUuid m_uuid;              ///< UUID группы
};
//-----------------------------------------------------------------------------
} // namespace hmcommon

#endif // GROUP_H
