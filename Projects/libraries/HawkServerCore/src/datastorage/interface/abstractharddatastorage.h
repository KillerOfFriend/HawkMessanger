#ifndef HMABSTRACTHARDDATASTORAGE_H
#define HMABSTRACTHARDDATASTORAGE_H

/**
 * @file abstractharddatastorage.h
 * @brief Содержит описание абстрактного класса физического хранилища данных
 */

#include "datastorage/interface/abstractdatastoragefunctional.h"

namespace hmservcommon::datastorage
{
//-----------------------------------------------------------------------------
/**
 * @brief The HMAbstractHardDataStorage class - Класс, описывающий абстракцию физического хранилища данных
 *
 * @authors Alekseev_s
 * @date 12.12.2020
 */
class HMAbstractHardDataStorage : public HMAbstractDataStorageFunctional
{
public:

    /**
     * @brief HMAbstractHardDataStorage - Конструктор по умолчанию
     */
    HMAbstractHardDataStorage() = default;

    /**
     * @brief ~HMAbstractHardDataStorage - Виртуальный деструктор по умолчанию
     */
    virtual ~HMAbstractHardDataStorage() override = default;

protected:

    /**
     * @brief makeDefault - Метод сформирует дефолтную структуру хранилища
     * @return Вернёт признак ошибки
     */
    virtual hmcommon::error_code makeDefault() = 0;

};
//-----------------------------------------------------------------------------
} // namespace hmservcommon::datastorage

#endif // HMABSTRACTHARDDATASTORAGE_H
