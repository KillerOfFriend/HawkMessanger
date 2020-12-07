#ifndef HMABSTRACTDATASTORAGEFUNCTIONAL_H
#define HMABSTRACTDATASTORAGEFUNCTIONAL_H

/**
 * @file cacheddatastorage.h
 * @brief Содержит описание абстрактного класса дополнительного функционала хранилища данных
 */

#include <memory>

#include "datastorage.h"

namespace hmservcommon
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
    HMAbstractDataStorageFunctional(const bool inMakeCache);

    /**
     * @brief ~HMAbstractDataStorageFunctional - Виртуальный деструктор по умолчанию
     */
    virtual ~HMAbstractDataStorageFunctional() = default;

protected:

    /**
     * @brief makeDefault - Метод сформирует дефолтную структуру хранилища
     * @return Вернёт признак ошибки
     */
    virtual std::error_code makeDefault() = 0;

    /**
     * @brief cache - Метод вернёт указатель на кеширующее хранилище
     * @return Вернёт указатель на кеш
     */
    virtual std::shared_ptr<HMDataStorage> cache() const;

    /**
     * @brief checkNewUserUnique - Метод проверит необходимую уникальность параметров нового пользователя
     * @param inUser - Проверяемый пользователь
     * @return Вернёт признак ошибки
     */
    virtual std::error_code checkNewUserUnique(const std::shared_ptr<hmcommon::HMUser> inUser) const;

    /**
     * @brief checkNewGroupUnique - Метод проверит необходимую уникальность параметров новой группы
     * @param inGroup - Проверяемая группа
     * @return Вернёт признак ошибки
     */
    virtual std::error_code checkNewGroupUnique(const std::shared_ptr<hmcommon::HMGroup> inGroup) const;

private:

    std::shared_ptr<HMDataStorage> m_cache = nullptr; ///< Кешированные данные хранилища

};
//-----------------------------------------------------------------------------
} // namespace hmservcommon

#endif // HMABSTRACTDATASTORAGEFUNCTIONAL_H
