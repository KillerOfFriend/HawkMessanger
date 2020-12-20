#ifndef JSONDATASTORAGE_H
#define JSONDATASTORAGE_H

/**
 * @file jsondatastorage.h
 * @brief Содержит описание класса хранилища данных в файле JSON
 */

#include <filesystem>

#include <nlohmann/json.hpp>

#include "jsondatastoragevalidator.h"
#include "datastorage/interface/abstractharddatastorage.h"

namespace hmservcommon::datastorage
{
//-----------------------------------------------------------------------------
/**
 * @brief The HMJsonDataStorage class - Класс, описывающий хранилище данных сервера в файле JSON
 *
 * @authors Alekseev_s
 * @date 21.11.2020
 */
class HMJsonDataStorage : public HMAbstractHardDataStorage
{
private:

    const std::filesystem::path m_jsonPath;                     ///< Путь к json файлу
    nlohmann::json m_json;                                      ///< json файл
    nlohmann::json m_invalidObject = nlohmann::json::object();  ///< Не валидный json объект

    HMJsonDataStorageValidator m_validator;                     ///< Валидатор формата данных

public:

    /**
     * @brief HMJsonDataStorage - Инициализирующий конструктор
     * @param inJsonPath - Путь к файлу JSON
     */
    HMJsonDataStorage(const std::filesystem::path& inJsonPath);

    /**
     * @brief ~HMJsonDataStorage - Виртуальный деструктор
     */
    virtual ~HMJsonDataStorage() override;


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

    // Связи [Пользователь]

    /**
     * @brief setUserContacts - Метод задаст пользователю список контактов
     * @param inUserUUID - Uuid пользователья
     * @param inContacts - Список контактов
     * @return Вернёт признак ошибки
     */
    virtual std::error_code setUserContacts(const QUuid& inUserUUID, const std::shared_ptr<hmcommon::HMContactList> inContacts) override;

    /**
     * @brief addUserContact - Метод добавит контакт пользователю
     * @param inUserUUID - Uuid пользователя
     * @param inContact - Новый контакт
     * @return Вернёт признак ошибки
     */
    virtual std::error_code addUserContact(const QUuid& inUserUUID, const std::shared_ptr<hmcommon::HMUser> inContact) override;

    /**
     * @brief removeUserContact - Метод удалит контакт пользователя
     * @param inUserUUID - Uuid пользователя
     * @param inContactUUID - Uuid контакта
     * @return Вернёт признак ошибки
     */
    virtual std::error_code removeUserContact(const QUuid& inUserUUID, const QUuid& inContactUUID) override;

    /**
     * @brief removeUserContacts - Метод удалит контакты пользователя
     * @param inUserUUID - Uuid пользователя
     * @return Вернёт признак ошибки
     */
    virtual std::error_code removeUserContacts(const QUuid& inUserUUID) override;

    /**
     * @brief getUserContactList - Метод вернёт список контактов пользователя
     * @param inUserUUID - Uuid пользователя
     * @param inContactUUID - Uuid контакта
     * @return Вернёт признак ошибки
     */
    virtual std::shared_ptr<hmcommon::HMContactList> getUserContactList(const QUuid& inUserUUID, std::error_code& outErrorCode) const override;

protected:

    /**
     * @brief makeDefault - Метод сформирует дефолтную структуру хранилища
     * @return Вернёт признак ошибки
     */
    virtual std::error_code makeDefault() override;

private:

    /**
     * @brief addContactUC - Метод добавит контакт пользователю
     * @param inUserUUID - Uuid пользователя
     * @param inContactUUID - Uuid контакта
     * @return Вернёт признак ошибки
     */
    std::error_code addContactUC(const QUuid& inUserUUID, const QUuid& inContactUUID);

    /**
     * @brief removeContactUC - Метод удалит контакт пользователя
     * @param inUserUUID - Uuid пользователя
     * @param inContactUUID - Uuid контакта
     * @return Вернёт признак ошибки
     */
    std::error_code removeContactUC(const QUuid& inUserUUID, const QUuid& inContactUUID);

    /**
     * @brief checkCorrectStruct - Метод проверит корректность структуры файла
     * @return Вернёт признак ошибки
     */
    std::error_code checkCorrectStruct() const;

    /**
     * @brief checkUsers - Метод проверит корректность структуры пользователей
     * @return Вернёт признак ошибки
     */
    std::error_code checkUsers() const;

    /**
     * @brief checkGroups - Метод проверит корректность структуры групп
     * @return Вернёт признак ошибки
     */
    std::error_code checkGroups() const;

    /**
     * @brief checkMessages - Метод проверит корректность структуры сообщений
     * @return Вернёт признак ошибки
     */
    std::error_code checkMessages() const;

    /**
     * @brief checkRelationsUC - Метод проверит корректность структуры связей пользователь-контакты
     * @return Вернёт признак ошибки
     */
    std::error_code checkRelationsUC() const;

    /**
     * @brief checkRelationsGU - Метод проверит корректность структуры связй группа-пользователи
     * @return Вернёт признак ошибки
     */
    std::error_code checkRelationsGU() const;

    /**
     * @brief write - Метод запишет изменения в JSON файл
     * @return Вернёт признак ошибки
     */
    std::error_code write() const;

    /**
     * @brief jsonToUser - Метод преобразует Json объект в экземпляр пользователя
     * @param inUserObject - Объект Json содержащий пользователя
     * @param outErrorCode - Признак ошибки
     * @return Вернёт указатель на экземпляр пользователя или nullptr
     */
    std::shared_ptr<hmcommon::HMUser> jsonToUser(const nlohmann::json& inUserObject, std::error_code& outErrorCode) const;

    /**
     * @brief userToJson - Метод преобразует пользователя в объект Json
     * @param inUser - Указатель на пользователя
     * @param outErrorCode - Признак ошибки
     * @return Вернёт объект Json
     */
    nlohmann::json userToJson(std::shared_ptr<hmcommon::HMUser> inUser, std::error_code& outErrorCode) const;

    /**
     * @brief jsonToGroup - Метод преобразует Json объект в экземпляр группы
     * @param inGroupObject - Объект Json содержащий группу
     * @param outErrorCode - Признак ошибки
     * @return Вернёт указатель на экземпляр группы или nullptr
     */
    std::shared_ptr<hmcommon::HMGroup> jsonToGroup(const nlohmann::json& inGroupObject, std::error_code& outErrorCode) const;

    /**
     * @brief groupToJson - Метод преобразует группу в объект Json
     * @param inGroup - Указатель на группу
     * @param outErrorCode - Признак ошибки
     * @return Вернёт объект Json
     */
    nlohmann::json groupToJson(std::shared_ptr<hmcommon::HMGroup> inGroup, std::error_code& outErrorCode) const;

    /**
     * @brief jsonToMessage - Метод преобразует Json объект в экземпляр сообщения
     * @param inMessageObject - Объект Json содержащий сообщение
     * @param outErrorCode - Признак ошибки
     * @return Вернёт указатель на экземпляр сообщения или nullptr
     */
    std::shared_ptr<hmcommon::HMGroupMessage> jsonToMessage(const nlohmann::json& inMessageObject, std::error_code& outErrorCode) const;

    /**
     * @brief messageToJson - Метод преобразует сообщение в объект Json
     * @param inMessage - Указатель на сообщение
     * @param outErrorCode - Признак ошибки
     * @return Вернёт объект Json
     */
    nlohmann::json messageToJson(std::shared_ptr<hmcommon::HMGroupMessage> inMessage, std::error_code& outErrorCode) const;

    /**
     * @brief jsonToRelationUC - Метод преобразует Json объект в экземпляр связи пользователь-контакты
     * @param inRelationUCObject - Объект Json содержащий связь пользователь-контакты
     * @param outErrorCode - Признак ошибки
     * @return Вернёт объект Json
     */
    std::shared_ptr<hmcommon::HMContactList> jsonToRelationUC(const nlohmann::json& inRelationUCObject, std::error_code& outErrorCode) const;

    /**
     * @brief relationUCToJson - Метод преобразует связь пользователь-контакты в объект Json
     * @param inUserUUID - UUID пользователя
     * @param inRelationUC - Указатель на связь пользователь-контакты
     * @param outErrorCode - Признак ошибки
     * @return Вернёт объект Json
     */
    nlohmann::json relationUCToJson(const QUuid& inUserUUID, const std::shared_ptr<hmcommon::HMContactList> inRelationUC, std::error_code& outErrorCode) const;

};
//-----------------------------------------------------------------------------
} // namespace hmservcommon::datastorage

#endif // JSONDATASTORAGE_H
