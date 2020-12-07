#ifndef JSONDATASTORAGE_H
#define JSONDATASTORAGE_H

/**
 * @file jsondatastorage.h
 * @brief Содержит описание класса хранилища данных в файле JSON
 */

#include <filesystem>

#include <nlohmann/json.hpp>

#include "jsondatastoragevalidator.h"
#include "abstractdatastoragefunctional.h"

namespace hmservcommon
{
//-----------------------------------------------------------------------------
/**
 * @brief The HMJsonDataStorage class - Класс, описывающий хранилище данных сервера в файле JSON
 *
 * @authors Alekseev_s
 * @date 21.11.2020
 */
class HMJsonDataStorage : public HMAbstractDataStorageFunctional
{
private:

    const std::filesystem::path m_jsonPath; ///< Путь к json файлу
    nlohmann::json m_json;                  ///< json файл

    HMJsonDataStorageValidator m_validator; ///< Валидатор формата данных

    /**
     * @brief checkCorrectStruct - Метод проверит корректность структуры файла
     * @return Вернёт признак ошибки
     */
    std::error_code checkCorrectStruct() const;

    /**
     * @brief write - Метод запишет изменения в JSON файл
     * @return Вернёт признак ошибки
     */
    std::error_code write() const;

    /**
     * @brief buildUserContacts - Метод инициализирует контакты пользователя
     * @param inJsonUser - Объект Json содержащий пользователя
     * @param outUser - Пользоваетль, для которого инициализируются контакты
     * @return Вернёт признак ошибки
     */
    std::error_code buildUserContacts(const nlohmann::json& inJsonUser, std::shared_ptr<hmcommon::HMUser> outUser) const;


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
     * @param inWithContacts - Флаг "Вернуть со списокм контактов"
     * @return Вернёт указатель на экземпляр пользователя или nullptr
     */
    virtual std::shared_ptr<hmcommon::HMUser> findUserByUUID(const QUuid& inUserUUID, std::error_code& outErrorCode, const bool inWithContacts = true) const override;

    /**
     * @brief findUserByAuthentication - Метод найдёт пользователя по его данным аутентификации
     * @param inLogin - Логин пользователя
     * @param inPasswordHash - Хеш пароля пользователя
     * @param outErrorCode - Признак ошибки
     * @param inWithContacts - Флаг "Вернуть со списокм контактов"
     * @return Вернёт указатель на экземпляр пользователя или nullptr
     */
    virtual std::shared_ptr<hmcommon::HMUser> findUserByAuthentication(const QString& inLogin, const QByteArray& inPasswordHash, std::error_code& outErrorCode, const bool inWithContacts = true) const override;

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
    virtual std::shared_ptr<hmcommon::HMGroupMessage> findMessage(const QUuid inMessageUUID, std::error_code& outErrorCode) const override;

    /**
     * @brief findMessages - Метод вернёт перечень сообщений группы за куазаный промежуток времени
     * @param inGroupUUID - Uuid группы, которой пренадлежат сообщения
     * @param inRange - Временной диапозон
     * @param outErrorCode - Признак ошибки
     * @return Вернёт перечень сообщений
     */
    virtual std::vector<std::shared_ptr<hmcommon::HMGroupMessage>> findMessages(const QUuid inGroupUUID, const hmcommon::MsgRange& inRange,  std::error_code& outErrorCode) const override;

    /**
     * @brief removeMessage - Метод удалит сообщение
     * @param inMessageUUID - Uuid сообщения
     * @param inGroupUUID - Uuid группы
     * @return Вернёт признак ошибки
     */
    virtual std::error_code removeMessage(const QUuid inMessageUUID, const QUuid inGroupUUID) override;


protected:

    /**
     * @brief makeDefault - Метод сформирует дефолтную структуру хранилища
     * @return Вернёт признак ошибки
     */
    virtual std::error_code makeDefault() override;

};
//-----------------------------------------------------------------------------
} // namespace hmservcommon

#endif // JSONDATASTORAGE_H
