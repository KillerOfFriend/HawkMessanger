#ifndef HMUserInfoList_H
#define HMUserInfoList_H

/**
 * @file userlist.h
 * @brief Содержит описание контактов пользователя системы
 */

#include <memory>
#include <system_error>
#include <unordered_set>

#include <QUuid>

#include "userinfo.h"

namespace hmcommon
{
//-----------------------------------------------------------------------------
/**
 * @brief The ContactMakeHash struct - Структура, реализующая оператор взятия хеша от пользователя
 */
struct ContactMakeHash
{
    /**
     * @brief operator () - Оператор сравнения пользователей
     * @param inContact - Указатель на контакт
     * @return Вернёт результат хеш функции сравнения
     */
    std::size_t operator() (const std::shared_ptr<HMUserInfo>& inContact) const noexcept;
};
//-----------------------------------------------------------------------------
/**
 * @brief The ContactCheckEqual struct - Структура, реализующая оператор сравнения пользователей
 */
struct ContactCheckEqual
{
    bool operator()(const std::shared_ptr<HMUserInfo>& inLeftContact, const std::shared_ptr<HMUserInfo>& inRightContact) const noexcept;
};
//-----------------------------------------------------------------------------
/**
 * @brief The HMUserInfoList class - Класс, содержащий список пользователей системы
 *
 * @authors Alekseev_s
 * @date 05.12.2020
 */
class HMUserInfoList
{
private:

    std::unordered_set<std::shared_ptr<HMUserInfo>, ContactMakeHash, ContactCheckEqual> m_contacts; ///< Контейнер, содержащий перечень пользователей

public:

    /**
     * @brief HMUserInfoList - Конструктор по умолчанию
     */
    HMUserInfoList() = default;

    /**
     * @brief ~HMUserInfoList - Деструктор по умолчанию
     */
    ~HMUserInfoList() = default;

    /**
     * @brief isEmpty - Метод вернёт признак того, что список пользователей пуст
     * @return
     */
    bool isEmpty() const;

    /**
     * @brief count - Метод вернёт количество пользователей в списке
     * @return Вернёт количество пользователей в списке
     */
    std::size_t count() const;

    /**
     * @brief contain - Метод проверит существование пользователей по UUID
     * @param inUserUUID - UUID контакта
     * @return Вернёт признак существования пользователей
     */
    bool contain(const QUuid& inUserUUID) const;

    /**
     * @brief contain - Метод проверит существование пользователей
     * @param inUser - Пользователь
     * @return Вернёт признак существования пользователей
     */
    bool contain(const std::shared_ptr<HMUserInfo> inUser) const;

    /**
     * @brief add - Метод добавит нового пользователя в перечень
     * @param inNewUser - Новый пользователь
     * @return Вернёт признак ошибки
     */
    std::error_code add(const std::shared_ptr<HMUserInfo> inNewUser);

    /**
     * @brief get - Метод вернёт пользователя по его порядковому номеру
     * @param inIndex - Порядковый номер пользователя
     * @param outErrorCode - Признак ошибки
     * @return Вернёт указатель на пользователя или null_ptr
     */
    std::shared_ptr<HMUserInfo> get(const std::size_t inIndex, std::error_code& outErrorCode) const;

    /**
     * @brief getContact - Метод вернёт пользователя по его UUID'у
     * @param inUserUuid - UUID пользователя
     * @param outErrorCode - Признак ошибки
     * @return Вернёт указатель на пользователя или null_ptr
     */
    std::shared_ptr<HMUserInfo> get(const QUuid inUserUuid, std::error_code& outErrorCode) const;

    /**
     * @brief remove - Метод удалит пользователя по его порядквому номеру
     * @param inIndex - Порядковый номер пользователя
     * @return Вернёт признак ошибки
     */
    std::error_code remove(const std::size_t inIndex);

    /**
     * @brief remove - Метод удалит пользователя по его UUID'у
     * @param inUserUuid - UUID пользователя
     * @return Вернёт признак ошибки
     */
    std::error_code remove(const QUuid inUserUuid);

};
//-----------------------------------------------------------------------------
}

#endif // HMUserInfoList_H
