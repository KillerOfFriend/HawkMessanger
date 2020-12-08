#ifndef HMJSONDATASTORAGEVALIDATOR_H
#define HMJSONDATASTORAGEVALIDATOR_H

/**
 * @file jsondatastoragevalidator.h
 * @brief Содержит описание класса, контролирующего JSON для HMJsonDataStorage
 */

#include <nlohmann/json.hpp>

#include "user.h"
#include "group.h"
#include "groupmessage.h"

namespace hmservcommon::datastorage
{
//-----------------------------------------------------------------------------
/**
 * @brief The HMJsonDataStorageValidator class - Коасс, контролирующий JSON для HMJsonDataStorage
 */
class HMJsonDataStorageValidator
{
public:

    /**
     * @brief HMJsonDataStorageValidator - Конструктор по умолчанию
     */
    HMJsonDataStorageValidator() = default;

    /**
     * @brief HMJsonDataStorageValidator - Деструктор по умолчанию
     */
    ~HMJsonDataStorageValidator() = default;

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

    /**
     * @brief checkMessage - Метод проверит валидность JSON объекта сообщения
     * @param inMesssageObject - Проверяемый объект
     * @return Вернёт признак ошибки
     */
    std::error_code checkMessage(const nlohmann::json& inMesssageObject) const;

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
     * @brief jsonToMessage - Метод преобразует Json объект в экземпляр сообщение
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
     * @brief jsonToByteArr - Функция преобразует JSON в QByteArray
     * @param inJson - Обрабатываемый JSON
     * @return Вернёт байтовую последовательность
     */
    QByteArray jsonToByteArr(const nlohmann::json& inJson) const;

    /**
     * @brief byteArrToJson - Функция преобразует QByteArray в JSON
     * @param inByteArr - Обрабатываемый QByteArray
     * @return Вернёт JSON array содержащий байтовую последоваельность
     */
    nlohmann::json byteArrToJson(const QByteArray& inByteArr) const;

};
//-----------------------------------------------------------------------------
} // namespace hmservcommon::datastorage

#endif // HMJSONDATASTORAGEVALIDATOR_H
