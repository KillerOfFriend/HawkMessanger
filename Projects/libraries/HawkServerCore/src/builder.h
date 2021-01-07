#ifndef HMBUILDER_H
#define HMBUILDER_H

/**
 * @file HMBuilder.h
 * @brief Содержит описание сборщика
 */

#include <HawkCommon.h>

#include "datastorage/interface/datastorageinterface.h"

namespace hmservcommon
{
//-----------------------------------------------------------------------------
/**
 * @brief The HMBuilder class - Класс, описывающий сборщик
 *
 * @authors Alekseev_s
 * @date 29.12.2020
 */
class HMBuilder
{
private:

    std::shared_ptr<datastorage::HMDataStorage> m_storage = nullptr; ///< Хранилище данных

public:

    /**
     * @brief HMBuilder - Инициализирующий конструктор
     * @param inStorage - Хранилище данных
     */
    HMBuilder(const std::shared_ptr<datastorage::HMDataStorage> inStorage);

    /**
     * @brief HMBuilder - Дефолтный деструктор по умолчанию
     */
    ~HMBuilder() = default;

    /**
     * @brief buildGroup - Метод соберёт экземпляр класса данных группы
     * @param inGroupUUID - UUID группы
     * @param outErrorCode - Признак ошибки
     * @return Вернёт указатель на экземпляр группы или nullptr
     */
    std::shared_ptr<hmcommon::HMGroup> buildGroup(const QUuid& inGroupUUID, std::error_code& outErrorCode);

    /**
     * @brief buildAccount - Метод соберёт экземпляр класса аккаунта пользователя
     * @param inUserUUID - UUID пользователя
     * @param outErrorCode - Признак ошибки
     * @return Вернёт указатель на экземпляр аккаунта или nullptr
     */
    std::shared_ptr<hmcommon::HMUser> buildUser(const QUuid& inUserUUID, std::error_code& outErrorCode);
};
//-----------------------------------------------------------------------------
}

#endif // HMBUILDER_H
