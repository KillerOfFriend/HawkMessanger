#ifndef HMACCOUNTBUILDER_H
#define HMACCOUNTBUILDER_H

/**
 * @file accountbuilder.h
 * @brief Содержит описание билдера аккаунта
 */

#include <memory>
#include <system_error>

#include <HawkCommon.h>

#include "datastorage/interface/datastorageinterface.h"

namespace hmservcommon::builders
{
//-----------------------------------------------------------------------------
/**
 * @brief The HMAccountBuilder class - Класс, описывающий билдер аккаунтов
 *
 * @authors Alekseev_s
 * @date 29.12.2020
 */
class HMAccountBuilder
{
private:

    std::shared_ptr<datastorage::HMDataStorage> m_storage = nullptr; ///< Хранилище данных

public:

    /**
     * @brief HMAccountBuilder - Инициализирующий конструктор
     * @param inStorage - Хранилище данных
     */
    HMAccountBuilder(const std::shared_ptr<datastorage::HMDataStorage> inStorage);

    /**
     * @brief HMAccountBuilder - Дефолтный деструктор по умолчанию
     */
    ~HMAccountBuilder() = default;

    /**
     * @brief buildGroup - Метод соберёт экземпляр класса данных группы
     * @param inGroupUUID - UUID группы
     * @param outErrorCode - Признак ошибки
     * @return Вернёт указатель на экземпляр группы или nullptr
     */
    std::shared_ptr<hmcommon::HMGroupInfo> buildGroup(const QUuid& inGroupUUID, std::error_code& outErrorCode);

    /**
     * @brief buildAccount - Метод соберёт экземпляр класса аккаунта пользователя
     * @param inUserUUID - UUID пользователя
     * @param outErrorCode - Признак ошибки
     * @return Вернёт указатель на экземпляр аккаунта или nullptr
     */
    std::shared_ptr<hmcommon::HMAccount> buildAccount(const QUuid& inUserUUID, std::error_code& outErrorCode);

};
//-----------------------------------------------------------------------------
} // namespace hmservcommon::builders

#endif // HMACCOUNTBUILDER_H
