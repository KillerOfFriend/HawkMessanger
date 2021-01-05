#ifndef HMCACHED_H
#define HMCACHED_H

#include <set>
#include <memory>
#include <chrono>

#include "user.h"
#include "group.h"
#include "userlist.h"

namespace hmservcommon
{
//-----------------------------------------------------------------------------
/**
 * @brief The HMCachedUser struct - Структура, описывающая кешированного пользователя
 */
struct HMCachedUser
{
    /**
     * @brief HMCachedUser - Инициализирующий конструктор
     * @param inUser - Указатель на пользователя
     */
    HMCachedUser(const std::shared_ptr<hmcommon::HMUser> inUser);

    /**
     * @brief HMCachedUser - Конструктор копирования (Удалён)
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

    std::shared_ptr<hmcommon::HMUser> m_user = nullptr;                 ///< Пользователь
    mutable std::chrono::system_clock::time_point m_lastRequest;        ///< Время последнего запроса
};
//-----------------------------------------------------------------------------
/**
 * @brief The HMCachedUserContacts struct - Структура, описывающая кешированную связь пользователя с контактами
 */
struct HMCachedUserContacts
{
    /**
     * @brief HMCachedUserContacts - Инициализирующий конструктор
     * @param inUserUUID - Uuid пользователя
     * @param inContactList - Список контактов
     */
    HMCachedUserContacts(const QUuid& inUserUUID, const std::shared_ptr<std::set<QUuid>> inContactList);

    /**
     * @brief HMCachedUserContacts - Конструктор копирования (Удалён)
     * @param inOther - Копируемый объект
     */
    HMCachedUserContacts(const HMCachedUserContacts& inOther) = delete;

    /**
     * @brief HMCachedUserContacts - Оператор перемещения
     * @param inOther - Перемещаемый объект
     */
    HMCachedUserContacts(HMCachedUserContacts&& inOther);

    // Операторы

    /**
     * @brief operator = - Оператор копирования (Удалён)
     * @param inOther - Копируемый объект
     * @return Вернёт копию объекта
     */
    HMCachedUserContacts& operator = (const HMCachedUserContacts& inOther) noexcept = delete;

    /**
     * @brief operator == - Оператор сравнения
     * @param inOther - Сравниваемый объект
     * @return Вернёт результат сравнения
     */
    bool operator == (const HMCachedUserContacts& inOther) const noexcept;

    QUuid m_userUUID;                                                   ///< UUID пользователя
    mutable std::shared_ptr<std::set<QUuid>> m_contactList = nullptr;   ///< Перечень контактов
    mutable std::chrono::system_clock::time_point m_lastRequest;        ///< Время последнего запроса
};
//-----------------------------------------------------------------------------
/**
 * @brief The HMCachedGroup struct - Структура, описывающая кешированную группу
 */
struct HMCachedGroup
{
    /**
     * @brief HMCachedGroup - Инициализирующий конструктор
     * @param inGroup - Указатель на группу
     */
    HMCachedGroup(const std::shared_ptr<hmcommon::HMGroup> inGroup);

    /**
     * @brief HMCachedGroup - Конструктор копирования (Удалён)
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

    std::shared_ptr<hmcommon::HMGroup> m_group = nullptr;               ///< Группа
    mutable std::chrono::system_clock::time_point m_lastRequest;        ///< Время последнего запроса
};
//-----------------------------------------------------------------------------
/**
 * @brief The HMCachedGroupUsers struct - Структура, описывающая кешированный перечень участников группы
 */
struct HMCachedGroupUsers
{
    /**
     * @brief HMCachedGroupUsers - Инициализирующий конструктор
     * @param inGroupUUID - UUID группы, которой пренадлежит перечень участников
     * @param inGroupUsers - Указатель на перечень участников группы
     */
    HMCachedGroupUsers(const QUuid& inGroupUUID, const std::shared_ptr<std::set<QUuid>> inGroupUsers);

    /**
     * @brief HMCachedGroupUsers - Конструктор копирования (Удалён)
     * @param inOther - Копируемый объект
     */
    HMCachedGroupUsers(const HMCachedGroupUsers& inOther) = delete;

    /**
     * @brief HMCachedGroupUsers - Оператор перемещения
     * @param inOther - Перемещаемый объект
     */
    HMCachedGroupUsers(HMCachedGroupUsers&& inOther);

    // Операторы

    /**
     * @brief operator = - Оператор копирования (Удалён)
     * @param inOther - Копируемый объект
     * @return Вернёт копию объекта
     */
    HMCachedGroupUsers& operator = (const HMCachedGroupUsers& inOther) noexcept = delete;

    /**
     * @brief operator == - Оператор сравнения
     * @param inOther - Сравниваемый объект
     * @return Вернёт результат сравнения
     */
    bool operator == (const HMCachedGroupUsers& inOther) const noexcept;

    // Данные

    QUuid m_group;                                                      ///< UUID группы, которой пренадлежит перечень участников
    mutable std::shared_ptr<std::set<QUuid>> m_groupUsers = nullptr;    ///< Перечень участников группы
    mutable std::chrono::system_clock::time_point m_lastRequest;        ///< Время последнего запроса
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
 * @brief The hash<hmservcommon::HMCachedUserContacts> struct - Структура, определяющая функцию взятия хеша от хешируемой связи контактов пользователя
 */
template <>
struct hash<hmservcommon::HMCachedUserContacts>
{
    std::size_t operator()(const hmservcommon::HMCachedUserContacts& inCachedUserContacts) const
    {
        return std::hash<std::string>{}(inCachedUserContacts.m_userUUID.toString().toStdString());
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
/**
 * @brief The hash<hmservcommon::HMCachedGroupUsers> struct - Структура, определяющая функцию взятия хеша от хешируемого перечня участников группы
 */
template <>
struct hash<hmservcommon::HMCachedGroupUsers>
{
    std::size_t operator()(const hmservcommon::HMCachedGroupUsers& inCachedGroupUsers) const
    {
        return std::hash<std::string>{}(inCachedGroupUsers.m_group.toString().toStdString());
    }
};
//-----------------------------------------------------------------------------
}

#endif // HMCACHED_H
