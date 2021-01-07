#ifndef HMGROUPINFOLIST_H
#define HMGROUPINFOLIST_H

/**
 * @file grouplist.h
 * @brief Содержит описание участников групп
 */

#include <system_error>
#include <unordered_set>

#include <QUuid>

#include "group.h"

namespace hmcommon
{
//-----------------------------------------------------------------------------
/**
 * @brief The GroupMakeHash struct - Структура, реализующая оператор взятия хеша от группы
 */
struct GroupMakeHash
{
    /**
     * @brief operator () - Оператор сравнения пользователей
     * @param inGroup - Указатель на группу
     * @return Вернёт хеш
     */
    std::size_t operator() (const std::shared_ptr<HMGroup>& inGroup) const noexcept;
};
//-----------------------------------------------------------------------------
/**
 * @brief The GroupsCheckEqual struct - Структура, реализующая оператор сравнения групп
 */
struct GroupsCheckEqual
{
    /**
     * @brief operator () - Оператор сравнения групп
     * @param inFirstGroup - Первая группа
     * @param inSecondGroup - Вторая группа
     * @return Вернёт признак равенства
     */
    bool operator()(const std::shared_ptr<HMGroup>& inFirstGroup, const std::shared_ptr<HMGroup>& inSecondGroup) const noexcept;
};
//-----------------------------------------------------------------------------
class HMGroupList
{   
private:

    std::unordered_set<std::shared_ptr<HMGroup>, GroupMakeHash, GroupsCheckEqual> m_contacts; ///< Контейнер, содержащий перечень групп

public:

    /**
     * @brief HMGroupList - Конструктор по умолчанию
     */
    HMGroupList() = default;

    /**
     * @brief ~HMGroupList - Деструктор по умолчанию
     */
    ~HMGroupList() = default;

    /**
     * @brief isEmpty - Метод вернёт признак того, что список групп пуст
     * @return Вернёт признак того, что список групп пуст
     */
    bool isEmpty() const;

    /**
     * @brief count - Метод вернёт количество групп в списке
     * @return Вернёт количество групп в списке
     */
    std::size_t count() const;

    /**
     * @brief contain - Метод проверит существование группы по UUID
     * @param inUserUUID - UUID группы
     * @return Вернёт признак существования группы
     */
    bool contain(const QUuid& inUserUUID) const;

    /**
     * @brief contain - Метод проверит существование группы
     * @param inGroup - Группа
     * @return Вернёт признак существования группы
     */
    bool contain(const std::shared_ptr<HMGroup> inGroup) const;

    /**
     * @brief add - Метод добавит новую группу в перечень
     * @param inNewGroup - Новая группа
     * @return Вернёт признак ошибки
     */
    std::error_code add(const std::shared_ptr<HMGroup> inNewGroup);

    /**
     * @brief get - Метод вернёт группу по её порядковому номеру
     * @param inIndex - Порядковый номер группы
     * @param outErrorCode - Признак ошибки
     * @return Вернёт указатель на группу или null_ptr
     */
    std::shared_ptr<HMGroup> get(const std::size_t inIndex, std::error_code& outErrorCode) const;

    /**
     * @brief getContact - Метод вернёт группу по её UUID'у
     * @param inUserUuid - UUID группы
     * @param outErrorCode - Признак ошибки
     * @return Вернёт указатель на группу или null_ptr
     */
    std::shared_ptr<HMGroup> get(const QUuid inGroupUuid, std::error_code& outErrorCode) const;

    /**
     * @brief remove - Метод удалит группу по её порядквому номеру
     * @param inIndex - Порядковый номер группы
     * @return Вернёт признак ошибки
     */
    std::error_code remove(const std::size_t inIndex);

    /**
     * @brief remove - Метод удалит группу по её UUID'у
     * @param inUserUuid - UUID группы
     * @return Вернёт признак ошибки
     */
    std::error_code remove(const QUuid inUserUuid);
};
//-----------------------------------------------------------------------------
}

#endif // HMGROUPINFOLIST_H
