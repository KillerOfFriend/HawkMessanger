#ifndef HMCOMBINEDDATASTORAGE_H
#define HMCOMBINEDDATASTORAGE_H

/**
 * @file combineddatastorage.h
 * @brief Содержит описание комбинированного хранилища данных (физическое\хешированное)
 */

#include <memory>

#include "datastorage/interface/abstractharddatastorage.h"
#include "datastorage/interface/abstractcahcedatastorage.h"

namespace hmservcommon::datastorage
{
//-----------------------------------------------------------------------------
/**
 * @brief The HMCombinedDataStorage class - Класс, описывающий комбинированное хранилище данных (физическое\хешированное)
 *
 * @authors Alekseev_s
 * @date 11.12.2020
 */
class HMCombinedDataStorage : public HMDataStorage
{
private:

    std::shared_ptr<HMAbstractHardDataStorage> m_HardStorage = nullptr;     ///< Физическое хранилище данных
    std::shared_ptr<HMAbstractCahceDataStorage> m_CacheStorage = nullptr;   ///< Кеширующее хранилище данных

public:

    /**
     * @brief HMCombinedDataStorage - Инициализирующий конструктор
     * @param inHardStorage - Физическое хранилище данных
     * @param inCacheStorage - Кеширующее хранилище данных
     */
    HMCombinedDataStorage(const std::shared_ptr<HMAbstractHardDataStorage> inHardStorage, const std::shared_ptr<HMAbstractCahceDataStorage> inCacheStorage = nullptr);

    /**
     * @brief ~HMCombinedDataStorage - Виртуальный деструктор по умолчанию
     */
    virtual ~HMCombinedDataStorage() override = default;

    // Хранилище

    /**
     * @brief open - Метод откроет хранилище данных
     * @return Вернёт признак ошибки
     */
    virtual std::error_code open() override;

    /**
     * @brief is_open - Метод вернёт признак открытости хранилища данных
     * @return Вернёт признак открытости
     */
    virtual bool is_open() const override;

    /**
     * @brief close - Метод закроет хранилище данных
     */
    virtual void close() override;

    // Пользователи

    /**
     * @brief addUser - Метод добавит нового пользователя
     * @param inUser - Добавляемый пользователь
     * @return Вернёт признак ошибки
     */
    virtual std::error_code addUser(const std::shared_ptr<hmcommon::HMUser> inUser) override;

    /**
     * @brief updateUser - Метод обновит данные пользователя
     * @param inUser - Обновляемый пользователь
     * @return Вернёт признак ошибки
     */
    virtual std::error_code updateUser(const std::shared_ptr<hmcommon::HMUser> inUser) override;

    /**
     * @brief findUserByUUID - Метод найдёт пользователя по его uuid
     * @param inUserUUID - Uuid пользователя
     * @param outErrorCode - Признак ошибки
     * @return Вернёт указатель на экземпляр пользователя или nullptr
     */
    virtual std::shared_ptr<hmcommon::HMUser> findUserByUUID(const QUuid& inUserUUID, std::error_code& outErrorCode) const override;

    /**
     * @brief findUserByAuthentication - Метод найдёт пользователя по его данным аутентификации
     * @param inLogin - Логин пользователя
     * @param inPasswordHash - Хеш пароля пользователя
     * @param outErrorCode - Признак ошибки
     * @return Вернёт указатель на экземпляр пользователя или nullptr
     */
    virtual std::shared_ptr<hmcommon::HMUser> findUserByAuthentication(const QString& inLogin, const QByteArray& inPasswordHash, std::error_code& outErrorCode) const override;

    /**
     * @brief removeUser - Метод удалит пользователя
     * @param inUserUUID - Uuid удаляемого пользователя
     * @return Вернёт признак ошибки
     */
    virtual std::error_code removeUser(const QUuid& inUserUUID) override;

    /**
     * @brief setUserContacts - Метод задаст пользователю список контактов
     * @param inUserUUID - Uuid пользователья
     * @param inContacts - Список контактов
     * @return Вернёт признак ошибки
     */
    virtual std::error_code setUserContacts(const QUuid& inUserUUID, const std::shared_ptr<std::set<QUuid>> inContacts) override;

    /**
     * @brief addUserContact - Метод добавит контакт пользователю
     * @param inUserUUID - Uuid пользователя
     * @param inContactUUID - Uuid контакта
     * @return Вернёт признак ошибки
     */
    virtual std::error_code addUserContact(const QUuid& inUserUUID, const QUuid& inContactUUID) override;

    /**
     * @brief removeUserContact - Метод удалит контакт пользователя
     * @param inUserUUID - Uuid пользователя
     * @param inContactUUID - Uuid контакта
     * @return Вернёт признак ошибки
     */
    virtual std::error_code removeUserContact(const QUuid& inUserUUID, const QUuid& inContactUUID) override;

    /**
     * @brief clearUserContacts - Метод очистит контакты пользователя
     * @param inUserUUID - Uuid пользователя
     * @return Вернёт признак ошибки
     */
    virtual std::error_code clearUserContacts(const QUuid& inUserUUID) override;

    /**
     * @brief getUserContactList - Метод вернёт список контактов пользователя
     * @param inUserUUID - Uuid пользователя
     * @param outErrorCode - Признак ошибки
     * @return Вернёт список контактов пользователя
     */
    virtual std::shared_ptr<std::set<QUuid>> getUserContactList(const QUuid& inUserUUID, std::error_code& outErrorCode) const override;

    /**
     * @brief getUserGroups - Метод вернёт список групп пользователя
     * @param inUserUUID - Uuid пользователя
     * @param outErrorCode - Признак ошибки
     * @return Вернёт список UUID'ов групп пользователя
     */
    virtual std::shared_ptr<std::set<QUuid>> getUserGroups(const QUuid& inUserUUID, std::error_code& outErrorCode) const override;

    // Группы

    /**
     * @brief addGroup - Метод добавит новую группу
     * @param inGroup - Добавляемая группа
     * @return Вернёт признак ошибки
     */
    virtual std::error_code addGroup(const std::shared_ptr<hmcommon::HMGroup> inGroup) override;

    /**
     * @brief updateGroup - Метод обновит данные группы
     * @param inGroup - Обновляемая группа
     * @return Вернёт признак ошибки
     */
    virtual std::error_code updateGroup(const std::shared_ptr<hmcommon::HMGroup> inGroup) override;

    /**
     * @brief findGroupByUUID - Метод найдёт пользователя по его uuid
     * @param inGroupUUID - Uuid группы
     * @param outErrorCode - Признак ошибки
     * @return Вернёт указатель на экземпляр группы или nullptr
     */
    virtual std::shared_ptr<hmcommon::HMGroup> findGroupByUUID(const QUuid& inGroupUUID, std::error_code& outErrorCode) const override;

    /**
     * @brief removeGroup - Метод удалит группу
     * @param inGroupUUID - Uuid удаляемой группы
     * @return Вернёт признак ошибки
     */
    virtual std::error_code removeGroup(const QUuid& inGroupUUID) override;

    /**
     * @brief setGroupUsers - Метод задаст список членов группы
     * @param inGroupUUID - Uuid группы
     * @param inUsers - Список пользователей группы (UUID'ы)
     * @return Вернёт признак ошибки
     */
    virtual std::error_code setGroupUsers(const QUuid& inGroupUUID, const std::shared_ptr<std::set<QUuid>> inUsers) override;

    /**
     * @brief setGroupUsers - Метод добавит пользователя в группу
     * @param inGroupUUID - Uuid группы
     * @param inUserUUID - UUID пользователя
     * @return Вернёт признак ошибки
     */
    virtual std::error_code addGroupUser(const QUuid& inGroupUUID, const QUuid& inUserUUID) override;

    /**
     * @brief removeGroupUser - Метод удалит пользователя из группы
     * @param inGroupUUID - Uuid группы
     * @param inUserUUID - Uuid пользователя
     * @return Вернёт признак ошибки
     */
    virtual std::error_code removeGroupUser(const QUuid& inGroupUUID, const QUuid& inUserUUID) override;

    /**
     * @brief clearGroupUsers - Метод очистит список членов группы
     * @param inGroupUUID - Uuid группы
     * @return Вернёт признак ошибки
     */
    virtual std::error_code clearGroupUsers(const QUuid& inGroupUUID) override;

    /**
     * @brief getGroupUserList - Метод вернёт список UUID'ов членов группы
     * @param inGroupUUID - Uuid группы
     * @param outErrorCode - Признак ошибки
     * @return Вернёт список UUID'ов пользователей группы
     */
    virtual std::shared_ptr<std::set<QUuid>> getGroupUserList(const QUuid& inGroupUUID, std::error_code& outErrorCode) const override;

    // Сообщения

    /**
     * @brief addMessage - Метод добавит новое сообщение
     * @param inMessage - Добавляемое сообщение
     * @return Вернёт признак ошибки
     */
    virtual std::error_code addMessage(const std::shared_ptr<hmcommon::HMGroupMessage> inMessage) override;

    /**
     * @brief updateMessage - Метод обновит данные сообщения
     * @param inMessage - Обновляемое сообщение
     * @return Вернёт признак ошибки
     */
    virtual std::error_code updateMessage(const std::shared_ptr<hmcommon::HMGroupMessage> inMessage) override;

    /**
     * @brief findMessage - Метод найдёт сообщение по его uuid
     * @param inMessageUUID - Uuid сообщения
     * @param outErrorCode - Признак ошибки
     * @return Вернёт указатель на экземпляр сообщения или nullptr
     */
    virtual std::shared_ptr<hmcommon::HMGroupMessage> findMessage(const QUuid& inMessageUUID, std::error_code& outErrorCode) const override;

    /**
     * @brief findMessages - Метод вернёт перечень сообщений группы за куазаный промежуток времени
     * @param inGroupUUID - Uuid группы, которой пренадлежат сообщения
     * @param inRange - Временной диапозон
     * @param outErrorCode - Признак ошибки
     * @return Вернёт перечень сообщений
     */
    virtual std::vector<std::shared_ptr<hmcommon::HMGroupMessage>> findMessages(const QUuid& inGroupUUID, const hmcommon::MsgRange& inRange, std::error_code& outErrorCode) const override;

    /**
     * @brief removeMessage - Метод удалит сообщение
     * @param inMessageUUID - Uuid сообщения
     * @param inGroupUUID - Uuid группы
     * @return Вернёт признак ошибки
     */
    virtual std::error_code removeMessage(const QUuid& inMessageUUID, const QUuid& inGroupUUID) override;

};
//-----------------------------------------------------------------------------
} // namespace hmservcommon::datastorage

#endif // HMCOMBINEDDATASTORAGE_H
