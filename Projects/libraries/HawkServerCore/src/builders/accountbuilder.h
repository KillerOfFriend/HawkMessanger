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
public:

    /**
     * @brief HMAccountBuilder - Дефолтный конструктор по умолчанию
     */
    HMAccountBuilder() = default;

    /**
     * @brief HMAccountBuilder - Дефолтный деструктор по умолчанию
     */
    ~HMAccountBuilder() = default;

    /**
     * @brief buildGroup - Метод соберёт экземпляр класса данных группы
     * @param inGroupUUID - UUID группы
     * @param inStorage - Указатель на хранилище данных из которого берётся информация
     * @param outErrorCode - Признак ошибки
     * @return Вернёт указатель на экземпляр группы или nullptr
     */
    static std::shared_ptr<hmcommon::HMGroup> buildGroup(const QUuid& inGroupUUID, const std::shared_ptr<datastorage::HMDataStorage> inStorage, std::error_code& outErrorCode);

    /**
     * @brief buildAccount - Метод соберёт экземпляр класса аккаунта пользователя
     * @param inUserUUID - UUID пользователя
     * @param inStorage - Указатель на хранилище данных из которого берётся информация
     * @param outErrorCode - Признак ошибки
     * @return Вернёт указатель на экземпляр аккаунта или nullptr
     */
    static std::shared_ptr<hmcommon::HMAccount> buildAccount(const QUuid& inUserUUID, const std::shared_ptr<datastorage::HMDataStorage> inStorage, std::error_code& outErrorCode);

};
//-----------------------------------------------------------------------------
} // namespace hmservcommon::builders

#endif // HMACCOUNTBUILDER_H
