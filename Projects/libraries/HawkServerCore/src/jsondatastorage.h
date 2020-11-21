#ifndef JSONDATASTORAGE_H
#define JSONDATASTORAGE_H

#include <filesystem>

#include <nlohmann/json.hpp>

#include "datastorage.h"

class HMJsonDataStorage : public HMDataStorage
{
private:

    const std::filesystem::path m_jsonPath; ///< Путь к json файлу
    nlohmann::json m_json; ///< json файл

    std::error_code write() const;

public:

    HMJsonDataStorage(const std::filesystem::path& inJsonPath);
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
    virtual std::error_code addUser(const std::shared_ptr<HMUser> inUser) override;

    /**
     * @brief updateUser - Метод обновит данные пользователя
     * @param inUser - Обновляемый пользователь
     * @return Вернёт признак ошибки
     */
    virtual std::error_code updateUser(const std::shared_ptr<HMUser> inUser) override;

    /**
     * @brief findUserByUUID - Метод найдёт пользователя по его uuid
     * @param inUserUUID - Uuid пользователя
     * @param outErrorCode - Признак ошибки
     * @return Вернёт указатель на экземпляр пользователя или nullptr
     */
    virtual std::shared_ptr<HMUser> findUserByUUID(const QUuid& inUserUUID, std::error_code& outErrorCode) override;

    /**
     * @brief findUserByAuthentication - Метод найдёт пользователя по его данным аутентификации
     * @param inLogin - Логин пользователя
     * @param inPasswordHash - Хеш пароля пользователя
     * @param outErrorCode - Признак ошибки
     * @return Вернёт указатель на экземпляр пользователя или nullptr
     */
    virtual std::shared_ptr<HMUser> findUserByAuthentication(const QString& inLogin, const QByteArray& inPasswordHash, std::error_code& outErrorCode) override;

    // Группы

    /**
     * @brief addGroup - Метод добавит новую группу
     * @param inGroup - Добавляемая группа
     * @return Вернёт признак ошибки
     */
    virtual std::error_code addGroup(const std::shared_ptr<HMGroup> inGroup) override;

    /**
     * @brief updateGroup - Метод обновит данные группы
     * @param inGroup - Обновляемая группа
     * @return Вернёт признак ошибки
     */
    virtual std::error_code updateGroup(const std::shared_ptr<HMGroup> inGroup) override;

    /**
     * @brief findGroupByUUID - Метод найдёт пользователя по его uuid
     * @param inGroupUUID - Uuid группы
     * @param outErrorCode - Признак ошибки
     * @return Вернёт указатель на экземпляр группы или nullptr
     */
    virtual std::shared_ptr<HMUser> findGroupByUUID(const QUuid& inGroupUUID, std::error_code& outErrorCode) override;

};

#endif // JSONDATASTORAGE_H
