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
     * @param inWithContacts - Флаг "Вернуть со списокм контактов"
     * @return Вернёт указатель на экземпляр пользователя или nullptr
     */
    virtual std::shared_ptr<hmcommon::HMUser> findUserByAuthentication(const QString& inLogin, const QByteArray& inPasswordHash, std::error_code& outErrorCode) const override;

    /**
     * @brief removeUser - Метод удалит пользователя
     * @param inUserUUID - Uuid удаляемого пользователя
     * @return Вернёт признак ошибки
     */
    virtual std::error_code removeUser(const QUuid& inUserUUID) override;

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

    // Связи

    /**
     * @brief getUserContactsIDList - Метод вернёт контакты пользователя в виде перечня UUID
     * @param inUserUUID - UUID пользователя
     * @param outErrorCode - Признак ошибки
     * @return Вернёт перечент контактов в виде списка UUID
     */
    virtual std::vector<QUuid> getUserContactsIDList(const QUuid& inUserUUID, std::error_code& outErrorCode) const override;

    /**
     * @brief getGroupUserIDList - Метод вернёт пользователей группы в виде перечня UUID
     * @param inGroupUUID - UUID группы
     * @param outErrorCode - Признак ошибки
     * @return  Вернёт перечент пользователей в виде списка UUID
     */
    virtual std::vector<QUuid> getGroupUserIDList(const QUuid& inGroupUUID, std::error_code& outErrorCode) const override;

};
//-----------------------------------------------------------------------------
} // namespace hmservcommon::datastorage

#endif // HMCOMBINEDDATASTORAGE_H
