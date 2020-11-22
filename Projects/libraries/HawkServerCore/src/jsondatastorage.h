#ifndef JSONDATASTORAGE_H
#define JSONDATASTORAGE_H

/**
 * @file jsondatastorage.h
 * @brief Содержит описание класса хранилища данных в файле JSON
 */

#include <filesystem>

#include <nlohmann/json.hpp>

#include "datastorage.h"

namespace hmservcommon
{
//-----------------------------------------------------------------------------
/**
 * @brief The HMJsonDataStorage class - Класс, описывающий хранилище данных сервера в файле JSON
 *
 * @authors Alekseev_s
 * @date 21.11.2020
 */
class HMJsonDataStorage : public HMDataStorage
{
private:

    const std::filesystem::path m_jsonPath; ///< Путь к json файлу
    nlohmann::json m_json;                  ///< json файл

    /**
     * @brief makeDefault - Метод сформирует дефолтную структуру файла
     * @return Вернёт признак ошибки
     */
    std::error_code makeDefault();

    /**
     * @brief write - Метод запишет изменения в JSON файл
     * @return Вернёт признак ошибки
     */
    std::error_code write() const;

    /**
     * @brief checkUser - Метод проверит валидность JSON объекта пользователя
     * @param inUserObject - Проверяемый объект
     * @return Вернёт признак ошибки
     */
    std::error_code checkUser(const nlohmann::json& inUserObject) const;

    /**
     * @brief checkGroup - Метод проверит валидность JSON объекта группы
     * @param inGroupObject - Проверяемый объект
     * @return Вернёт признак ошибки
     */
    std::error_code checkGroup(const nlohmann::json& inGroupObject) const;

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
    virtual std::shared_ptr<hmcommon::HMUser> findUserByUUID(const QUuid& inUserUUID, std::error_code& outErrorCode) override;

    /**
     * @brief findUserByAuthentication - Метод найдёт пользователя по его данным аутентификации
     * @param inLogin - Логин пользователя
     * @param inPasswordHash - Хеш пароля пользователя
     * @param outErrorCode - Признак ошибки
     * @return Вернёт указатель на экземпляр пользователя или nullptr
     */
    virtual std::shared_ptr<hmcommon::HMUser> findUserByAuthentication(const QString& inLogin, const QByteArray& inPasswordHash, std::error_code& outErrorCode) override;

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
    virtual std::shared_ptr<hmcommon::HMUser> findGroupByUUID(const QUuid& inGroupUUID, std::error_code& outErrorCode) override;

};
//-----------------------------------------------------------------------------
} // namespace hmservcommon

#endif // JSONDATASTORAGE_H
