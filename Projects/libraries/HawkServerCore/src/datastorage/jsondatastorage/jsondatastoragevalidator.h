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
#include "contactlist.h"

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
     * @brief checkRelationUC - Метод проверит валидность JSON объекта связи пользователь-контакты
     * @param inRelationUCObject - Проверяемый объект
     * @return Вернёт признак ошибки
     */
    std::error_code checkRelationUC(const nlohmann::json& inRelationUCObject) const;

    /**
     * @brief checkUserContactsRelation - Метод проверит валидность JSON объекта связи пользователь-контакты
     * @param inUCRelation - Проверяемый объект
     * @return Вернёт признак ошибки
     */
    std::error_code checkUserContactsRelation(const nlohmann::json& inUCRelation) const;

    /**
     * @brief checkUserContactRelations - Метод проверит валидность JSON объекта связи группа-пользователи
     * @param inGURelation - Проверяемый объект
     * @return Вернёт признак ошибки
     */
    std::error_code checkGroupUsersRelation(const nlohmann::json& inGURelation) const;

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
