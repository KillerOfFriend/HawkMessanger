#ifndef HMCACHED_H
#define HMCACHED_H

#include "user.h"
#include "group.h"

namespace hmservcommon
{
//-----------------------------------------------------------------------------
struct HMCachedUser
{
    /**
     * @brief HMCachedUser - Инициализирующий конструктор
     * @param inUser - Указатель на пользователя
     * @param inCreateTime - Время создания
     */
    HMCachedUser(const std::shared_ptr<hmcommon::HMUser> inUser, const QTime& inCreateTime = QTime::currentTime());

    /**
     * @brief HMCachedUser - Конструкто копирования (Удалён)
     * @param inOther - Копируемый объект
     */
    HMCachedUser(const HMCachedUser& inOther) = delete;

    /**
     * @brief HMCachedUser - Оператор перемещения
     * @param inOther - Перемещаемый объект
     */
    HMCachedUser(HMCachedUser&& inOther);

    // Операторы

    /**
     * @brief operator = - Оператор копирования (Удалён)
     * @param inOther - Копируемый объект
     * @return Вернёт копию объекта
     */
    HMCachedUser& operator = (const HMCachedUser& inOther) noexcept = delete;

    /**
     * @brief operator == - Оператор сравнения
     * @param inOther - Сравниваемый объект
     * @return Вернёт результат сравнения
     */
    bool operator == (const HMCachedUser& inOther) const noexcept;

    // Данные

    std::shared_ptr<hmcommon::HMUser> m_user = nullptr; ///< Пользователь
    mutable QTime m_lastRequest = QTime::currentTime(); ///< Время последнего запроса
};
//-----------------------------------------------------------------------------
struct HMCachedGroup
{
    /**
     * @brief HMCachedGroup - Инициализирующий конструктор
     * @param inGroup - Указатель на группу
     * @param inCreateTime - Время создания
     */
    HMCachedGroup(const std::shared_ptr<hmcommon::HMGroup> inGroup, const QTime& inCreateTime = QTime::currentTime());

    /**
     * @brief HMCachedGroup - Конструкто копирования (Удалён)
     * @param inOther - Копируемый объект
     */
    HMCachedGroup(const HMCachedGroup& inOther) = delete;

    /**
     * @brief HMCachedGroup - Оператор перемещения
     * @param inOther - Перемещаемый объект
     */
    HMCachedGroup(HMCachedGroup&& inOther);

    // Операторы

    /**
     * @brief operator = - Оператор копирования (Удалён)
     * @param inOther - Копируемый объект
     * @return Вернёт копию объекта
     */
    HMCachedUser& operator = (const HMCachedGroup& inOther) noexcept = delete;

    /**
     * @brief operator == - Оператор сравнения
     * @param inOther - Сравниваемый объект
     * @return Вернёт результат сравнения
     */
    bool operator == (const HMCachedGroup& inOther) const noexcept;

    // Данные

    std::shared_ptr<hmcommon::HMGroup> m_group = nullptr;   ///< Группа
    mutable QTime m_lastRequest = QTime::currentTime();     ///< Время последнего запроса
};
//-----------------------------------------------------------------------------
}

namespace std
{
//-----------------------------------------------------------------------------
/**
 * @brief The hash<hmservcommon::HMCachedUser> struct - Структура, определяющая функцию взятия хеша от хешируемого пользователя
 */
template <>
struct hash<hmservcommon::HMCachedUser>
{
    std::size_t operator()(const hmservcommon::HMCachedUser& inCachedUser) const
    {
        if (!inCachedUser.m_user)
            return 0;
        else
            return std::hash<std::string>{}(inCachedUser.m_user->m_uuid.toString().toStdString());
    }
};
//-----------------------------------------------------------------------------
/**
 * @brief The hash<hmservcommon::HMCachedGroup> struct - Структура, определяющая функцию взятия хеша от хешируемой группы
 */
template <>
struct hash<hmservcommon::HMCachedGroup>
{
    std::size_t operator()(const hmservcommon::HMCachedGroup& inCachedGroup) const
    {
        if (!inCachedGroup.m_group)
            return 0;
        else
            return std::hash<std::string>{}(inCachedGroup.m_group->m_uuid.toString().toStdString());
    }
};
//-----------------------------------------------------------------------------
}

#endif // HMCACHED_H
