#ifndef HMABSTRACTDATASTORAGEFUNCTIONAL_H
#define HMABSTRACTDATASTORAGEFUNCTIONAL_H

/**
 * @file cacheddatastorage.h
 * @brief Содержит описание абстрактного класса дополнительного функционала хранилища данных
 */

#include <memory>

#include "datastorageinterface.h"

namespace hmservcommon::datastorage
{
//-----------------------------------------------------------------------------
/**
 * @brief The HMAbstractDataStorageFunctional class - Класс, описывающий абстрактное хранилище даннх с дополнительным внутренним функционалом
 *
 * @authors Alekseev_s
 * @date 07.12.2020
 */
class HMAbstractDataStorageFunctional : public HMDataStorage
{
public:

    /**
     * @brief HMAbstractDataStorageFunctional - Конструктор по умолчанию
     */
    HMAbstractDataStorageFunctional();

    /**
     * @brief ~HMAbstractDataStorageFunctional - Виртуальный деструктор по умолчанию
     */
    virtual ~HMAbstractDataStorageFunctional() = default;

protected:

    /**
     * @brief checkNewUserUnique - Метод проверит необходимую уникальность параметров нового пользователя
     * @param inUser - Проверяемый пользователь
     * @return Вернёт признак ошибки
     */
    virtual std::error_code checkNewUserUnique(const std::shared_ptr<hmcommon::HMUserInfo> inUser) const;

    /**
     * @brief checkNewGroupUnique - Метод проверит необходимую уникальность параметров новой группы
     * @param inGroup - Проверяемая группа
     * @return Вернёт признак ошибки
     */
    virtual std::error_code checkNewGroupUnique(const std::shared_ptr<hmcommon::HMGroupInfo> inGroup) const;

//    /**
//     * @brief checkUserContactsUnique - Метод проверит необходимую уникальность параметров нового списка контактов
//     * @param inUserUUID - Uuid пользователя
//     * @param inContacts - Список контактов
//     * @return Вернёт признак ошибки
//     */
//    virtual std::error_code checkUserContactsUnique(const QUuid& inUserUUID, const std::shared_ptr<hmcommon::HMUserInfoList> inContacts) const;

};
//-----------------------------------------------------------------------------
} // namespace hmservcommon::datastorage

#endif // HMABSTRACTDATASTORAGEFUNCTIONAL_H
