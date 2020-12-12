#ifndef HMABSTRACTCAHCEDATASTORAGE_H
#define HMABSTRACTCAHCEDATASTORAGE_H

/**
 * @file abstractcahcedatastorage.h
 * @brief Содержит описание абстрактного класса кеширующего хранилища данных
 */

#include "datastorage/interface/abstractdatastoragefunctional.h"

namespace hmservcommon::datastorage
{
//-----------------------------------------------------------------------------
/**
 * @brief The HMAbstractCahceDataStorage class - Класс, описывающий абстракцию кеширующего хранилища данных
 *
 * @authors Alekseev_s
 * @date 12.12.2020
 */
class HMAbstractCahceDataStorage : public HMAbstractDataStorageFunctional
{
public:

    /**
     * @brief HMAbstractCahceDataStorage - Конструктор по умолчанию
     */
    HMAbstractCahceDataStorage() = default;

    /**
     * @brief ~HMAbstractCahceDataStorage - Виртуальный деструктор по умолчанию
     */
    virtual ~HMAbstractCahceDataStorage() override = default;

    // Связи

    /**
     * @brief getUserContactsIDList - Метод вернёт контакты пользователя в виде перечня UUID
     * @param inUserUUID - UUID пользователя
     * @param outErrorCode - Признак ошибки
     * @return Вернёт перечент контактов в виде списка UUID
     */
    virtual std::vector<QUuid> getUserContactsIDList(const QUuid inUserUUID,  std::error_code& outErrorCode) const override;

    /**
     * @brief getGroupUserIDList - Метод вернёт пользователей группы в виде перечня UUID
     * @param inGroupUUID - UUID группы
     * @param outErrorCode - Признак ошибки
     * @return  Вернёт перечент пользователей в виде списка UUID
     */
    virtual std::vector<QUuid> getGroupUserIDList(const QUuid inGroupUUID,  std::error_code& outErrorCode) const override;
};
//-----------------------------------------------------------------------------
} // namespace hmservcommon::datastorage

#endif // HMABSTRACTCAHCEDATASTORAGE_H
