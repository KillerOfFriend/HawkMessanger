#ifndef HMCONTACTLIST_H
#define HMCONTACTLIST_H

/**
 * @file contactlist.h
 * @brief Содержит описание контактов пользователя системы
 */

#include <unordered_set>
#include <memory>
#include <system_error>

#include <QUuid>

namespace hmcommon
{
//-----------------------------------------------------------------------------
class HMUser;
//-----------------------------------------------------------------------------
/**
 * @brief The ContactCompare struct - Компаратор пользователей для перечня контактов
 */
struct ContactHashCompare
{
    /**
     * @brief operator () - Оператор сравнения пользователей
     * @param inUser - Указатель на пользователя
     * @return Вернёт результат хеш функции сравнения
     */
    std::size_t operator() (const std::shared_ptr<HMUser>& inUser) const;
};
//-----------------------------------------------------------------------------
/**
 * @brief The HMContactList class - Класс, содержащий список контактов пользователя системы
 *
 * @authors Alekseev_s
 * @date 05.12.2020
 */
class HMContactList
{
private:

    std::unordered_set<std::shared_ptr<HMUser>, ContactHashCompare> m_contacts; ///< Контейнер, содержащий перечень контактов

public:

    /**
     * @brief HMContactList - Конструктор по умолчанию
     */
    HMContactList() = default;

    /**
     * @brief ~HMContactList - Деструктор по умолчанию
     */
    ~HMContactList() = default;

    /**
     * @brief isEmpty - Метод вернёт признак того, что список контактов пуст
     * @return
     */
    bool isEmpty() const;

    /**
     * @brief contactsCount - Метод вернёт количество контактов в списке
     * @return Вернёт количество контактов в списке
     */
    std::size_t contactsCount() const;

    /**
     * @brief addContact - Метод добавит новый контакт в перечень
     * @param inNewContact - Новый контакт
     * @return Вернёт признак ошибки
     */
    std::error_code addContact(const std::shared_ptr<HMUser> inNewContact);

    /**
     * @brief getContact - Метод вернёт контакт по его порядковому номеру
     * @param inContactIndex - Порядковый номер контакта
     * @param outErrorCode - Признак ошибки
     * @return Вернёт указатель на контакт или null_ptr
     */
    std::shared_ptr<HMUser> getContact(const std::size_t inContactIndex, std::error_code& outErrorCode) const;

    /**
     * @brief getContact - Метод вернёт контакт по его UUID'у
     * @param inContactUuid - UUID контакта
     * @param outErrorCode - Признак ошибки
     * @return Вернёт указатель на контакт или null_ptr
     */
    std::shared_ptr<HMUser> getContact(const QUuid inContactUuid, std::error_code& outErrorCode) const;

    /**
     * @brief removeContact - Метод удалит контакт по его порядквому номеру
     * @param inContactIndex - Порядковый номер контакта
     * @return Вернёт признак ошибки
     */
    std::error_code removeContact(const std::size_t inContactIndex);

    /**
     * @brief removeContact - Метод удалит контакт по его UUID'у
     * @param inContactUuid - UUID контакта
     * @return Вернёт признак ошибки
     */
    std::error_code removeContact(const QUuid inContactUuid);

};
//-----------------------------------------------------------------------------
}

#endif // HMCONTACTLIST_H
