#include "accountbuilder.h"

#include "systemerrorex.h"

using namespace hmservcommon;
using namespace hmservcommon::builders;

//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMGroup> buildGroup(const QUuid& inGroupUUID, const std::shared_ptr<datastorage::HMDataStorage> inStorage, std::error_code& outErrorCode)
{
    std::shared_ptr<hmcommon::HMGroup> Result = nullptr;
    outErrorCode = make_error_code(hmcommon::eSystemErrorEx::seSuccess); // Изначально помечаем как успех

    if (!inStorage) // Работаем только с валидным указателем
        outErrorCode = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
    else
    {
        Result = inStorage->findGroupByUUID(inGroupUUID, outErrorCode);

        if (!outErrorCode) // Информация о группе сформирована успешно
        {
            // TODO Получить список саязей группа-пользователи и сформировать список польователей через findUserByUUID
        }

        if (outErrorCode) // Если при построении произошла ошибка
            Result = nullptr; // Сбрасываем результат
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMAccount> buildAccount(const QUuid& inUserUUID, const std::shared_ptr<datastorage::HMDataStorage> inStorage, std::error_code& outErrorCode)
{
    std::shared_ptr<hmcommon::HMAccount> Result = nullptr;
    outErrorCode = make_error_code(hmcommon::eSystemErrorEx::seSuccess); // Изначально помечаем как успех

    /*
     * 1) Получить информацию о пользователе
     * 2) Получить список контактов
     * 3) Получить список групп
     */

    if (!inStorage) // Работаем только с валидным указателем
        outErrorCode = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr); // Формируем соответствующую ошибку
    else
    {
        Result = std::make_shared<hmcommon::HMAccount>(); // Создаём экземпляр аккаунта

        Result->m_userInfo = inStorage->findUserByUUID(inUserUUID, outErrorCode); // Ищим данные о пользователе в хранилище

        if (!outErrorCode) // Информация о пользователе сформирована успешно
        {
            Result->m_cotacts = inStorage->getUserContactList(inUserUUID, outErrorCode); // Ищим список контактов пользователя

            if (!outErrorCode) // Если список контактов сформирован успешно
            {
                // TODO Получить связи пользователь-группы и построить список групп через buildGroup
            }
        }

        if (outErrorCode) // Если при построении произошла ошибка
            Result = nullptr; // Сбрасываем результат
    }

    return Result;
}
//-----------------------------------------------------------------------------
