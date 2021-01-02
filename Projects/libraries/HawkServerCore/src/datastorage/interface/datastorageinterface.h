#ifndef DATASTORAGEINTERFACE_H
#define DATASTORAGEINTERFACE_H

/**
 * @file datastorage.h
 * @brief Содержит описание интерфейса хранилища данных
 */

#include <set>
#include <vector>
#include <memory>
#include <system_error>

#include <HawkCommon.h>

namespace hmservcommon::datastorage
{
//-----------------------------------------------------------------------------
/**
 * @brief The HMDataStorage class - Интерфейс, описывающий хранилище данных сервера
 *
 * @details Хранит информацию о:
 * 1) Польхователях системы.
 * 2) Группах системы.
 * 3) Переписку групп системы.
 *
 * @authors Alekseev_s
 * @date 08.11.2020
 */
class HMDataStorage
{
public:

    /**
     * @brief HMDataStorage - Конструктор по умолчанию
     */
    HMDataStorage() = default;

    /**
     * @brief ~HMDataStorage - Виртуальный деструктор по умолчанию
     */
    virtual ~HMDataStorage() = default;

    // Хранилище

    /**
     * @brief open - Метод откроет хранилище данных
     * @return Вернёт признак ошибки
     */
    virtual std::error_code open() = 0;

    /**
     * @brief is_open - Метод вернёт признак открытости хранилища данных
     * @return Вернёт признак открытости
     */
    virtual bool is_open() const = 0;

    /**
     * @brief close - Метод закроет хранилище данных
     */
    virtual void close() = 0;

    // Пользователи

    /**
     * @brief addUser - Метод добавит нового пользователя
     * @param inUser - Добавляемый пользователь
     * @return Вернёт признак ошибки
     */
    virtual std::error_code addUser(const std::shared_ptr<hmcommon::HMUser> inUser) = 0;

    /**
     * @brief updateUser - Метод обновит данные пользователя
     * @param inUser - Обновляемый пользователь
     * @return Вернёт признак ошибки
     */
    virtual std::error_code updateUser(const std::shared_ptr<hmcommon::HMUser> inUser) = 0;

    /**
     * @brief findUserByUUID - Метод найдёт пользователя по его uuid
     * @param inUserUUID - Uuid пользователя
     * @param outErrorCode - Признак ошибки
     * @return Вернёт указатель на экземпляр пользователя или nullptr
     */
    virtual std::shared_ptr<hmcommon::HMUser> findUserByUUID(const QUuid& inUserUUID, std::error_code& outErrorCode) const = 0;

    /**
     * @brief findUserByAuthentication - Метод найдёт пользователя по его данным аутентификации
     * @param inLogin - Логин пользователя
     * @param inPasswordHash - Хеш пароля пользователя
     * @param outErrorCode - Признак ошибки
     * @return Вернёт указатель на экземпляр пользователя или nullptr
     */
    virtual std::shared_ptr<hmcommon::HMUser> findUserByAuthentication(const QString& inLogin, const QByteArray& inPasswordHash, std::error_code& outErrorCode) const = 0;

    /**
     * @brief removeUser - Метод удалит пользователя
     * @param inUserUUID - Uuid удаляемого пользователя
     * @return Вернёт признак ошибки
     */
    virtual std::error_code removeUser(const QUuid& inUserUUID) = 0;

    /**
     * @brief getUserGroups - Метод вернёт список групп пользователя
     * @param inUserUUID - Uuid пользователя
     * @param outErrorCode - Признак ошибки
     * @return Вернёт список UUID'ов групп пользователя
     */
    virtual std::shared_ptr<std::set<QUuid>> getUserGroups(const QUuid& inUserUUID, std::error_code& outErrorCode) const = 0;

    // Группы

    /**
     * @brief addGroup - Метод добавит новую группу
     * @param inGroup - Добавляемая группа
     * @return Вернёт признак ошибки
     */
    virtual std::error_code addGroup(const std::shared_ptr<hmcommon::HMGroup> inGroup) = 0;

    /**
     * @brief updateGroup - Метод обновит данные группы
     * @param inGroup - Обновляемая группа
     * @return Вернёт признак ошибки
     */
    virtual std::error_code updateGroup(const std::shared_ptr<hmcommon::HMGroup> inGroup) = 0;

    /**
     * @brief findGroupByUUID - Метод найдёт пользователя по его uuid
     * @param inGroupUUID - Uuid группы
     * @param outErrorCode - Признак ошибки
     * @return Вернёт указатель на экземпляр группы или nullptr
     */
    virtual std::shared_ptr<hmcommon::HMGroup> findGroupByUUID(const QUuid& inGroupUUID, std::error_code& outErrorCode) const = 0;

    /**
     * @brief removeGroup - Метод удалит группу
     * @param inGroupUUID - Uuid удаляемой группы
     * @return Вернёт признак ошибки
     */
    virtual std::error_code removeGroup(const QUuid& inGroupUUID) = 0;

    /**
     * @brief setGroupUsers - Метод задаст список членов группы
     * @param inGroupUUID - Uuid группы
     * @param inUsers - Список пользователей группы (UUID'ы)
     * @return Вернёт признак ошибки
     */
    virtual std::error_code setGroupUsers(const QUuid& inGroupUUID, const std::shared_ptr<std::set<QUuid>> inUsers) = 0;

    /**
     * @brief setGroupUsers - Метод добавит пользователя в группу
     * @param inGroupUUID - Uuid группы
     * @param inUserUUID - UUID пользователя
     * @return Вернёт признак ошибки
     */
    virtual std::error_code addGroupUser(const QUuid& inGroupUUID, const QUuid& inUserUUID) = 0;

    /**
     * @brief removeGroupUser - Метод удалит пользователя из группы
     * @param inGroupUUID - Uuid группы
     * @param inUserUUID - Uuid пользователя
     * @return Вернёт признак ошибки
     */
    virtual std::error_code removeGroupUser(const QUuid& inGroupUUID, const QUuid& inUserUUID) = 0;

    /**
     * @brief clearGroupUsers - Метод очистит список членов группы
     * @param inGroupUUID - Uuid группы
     * @return Вернёт признак ошибки
     */
    virtual std::error_code clearGroupUsers(const QUuid& inGroupUUID) = 0;

    /**
     * @brief getGroupUserList - Метод вернёт список UUID'ов членов группы
     * @param inGroupUUID - Uuid группы
     * @param outErrorCode - Признак ошибки
     * @return Вернёт список UUID'ов пользователей группы
     */
    virtual std::shared_ptr<std::set<QUuid>> getGroupUserList(const QUuid& inGroupUUID, std::error_code& outErrorCode) const = 0;

    // Сообщения

    /**
     * @brief addMessage - Метод добавит новое сообщение
     * @param inMessage - Добавляемое сообщение
     * @return Вернёт признак ошибки
     */
    virtual std::error_code addMessage(const std::shared_ptr<hmcommon::HMGroupMessage> inMessage) = 0;

    /**
     * @brief updateMessage - Метод обновит данные сообщения
     * @param inMessage - Обновляемое сообщение
     * @return Вернёт признак ошибки
     */
    virtual std::error_code updateMessage(const std::shared_ptr<hmcommon::HMGroupMessage> inMessage) = 0;

    /**
     * @brief findMessages - Метод найдёт сообщение по его uuid
     * @param inMessageUUID - Uuid сообщения
     * @param outErrorCode - Признак ошибки
     * @return Вернёт указатель на экземпляр сообщения или nullptr
     */
    virtual std::shared_ptr<hmcommon::HMGroupMessage> findMessage(const QUuid& inMessageUUID, std::error_code& outErrorCode) const = 0;

    /**
     * @brief findMessages - Метод вернёт перечень сообщений группы за куазаный промежуток времени
     * @param inGroupUUID - Uuid группы, которой пренадлежат сообщения
     * @param inRange - Временной диапозон
     * @param outErrorCode - Признак ошибки
     * @return Вернёт перечень сообщений
     */
    virtual std::vector<std::shared_ptr<hmcommon::HMGroupMessage>> findMessages(const QUuid& inGroupUUID, const hmcommon::MsgRange& inRange, std::error_code& outErrorCode) const = 0;

    /**
     * @brief removeMessage - Метод удалит сообщение
     * @param inMessageUUID - Uuid сообщения
     * @param inGroupUUID - Uuid группы
     * @return Вернёт признак ошибки
     */
    virtual std::error_code removeMessage(const QUuid& inMessageUUID, const QUuid& inGroupUUID) = 0;

    // Связи [Пользователь]

    /**
     * @brief setUserContacts - Метод задаст пользователю список контактов
     * @param inUserUUID - Uuid пользователья
     * @param inContacts - Список контактов
     * @return Вернёт признак ошибки
     */
    virtual std::error_code setUserContacts(const QUuid& inUserUUID, const std::shared_ptr<hmcommon::HMUserList> inContacts) = 0;

    /**
     * @brief addUserContact - Метод добавит контакт пользователю
     * @param inUserUUID - Uuid пользователя
     * @param inContact - Новый контакт
     * @return Вернёт признак ошибки
     */
    virtual std::error_code addUserContact(const QUuid& inUserUUID, const std::shared_ptr<hmcommon::HMUser> inContact) = 0;

    /**
     * @brief removeUserContact - Метод удалит контакт пользователя
     * @param inUserUUID - Uuid пользователя
     * @param inContactUUID - Uuid контакта
     * @return Вернёт признак ошибки
     */
    virtual std::error_code removeUserContact(const QUuid& inUserUUID, const QUuid& inContactUUID) = 0;

    /**
     * @brief removeUserContacts - Метод удалит контакты пользователя
     * @param inUserUUID - Uuid пользователя
     * @return Вернёт признак ошибки
     */
    virtual std::error_code removeUserContacts(const QUuid& inUserUUID) = 0;

    /**
     * @brief getUserContactList - Метод вернёт список контактов пользователя
     * @param inUserUUID - Uuid пользователя
     * @param outErrorCode - Признак ошибки
     * @return Вернёт список контактов пользователя
     */
    virtual std::shared_ptr<hmcommon::HMUserList> getUserContactList(const QUuid& inUserUUID, std::error_code& outErrorCode) const = 0;

};
//-----------------------------------------------------------------------------
} // namespace hmservcommon::datastorage

#endif // DATASTORAGEINTERFACE_H
