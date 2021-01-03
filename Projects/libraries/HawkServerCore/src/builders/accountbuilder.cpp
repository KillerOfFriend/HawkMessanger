#include "accountbuilder.h"

#include <cassert>

#include <HawkLog.h>

#include "systemerrorex.h"

using namespace hmservcommon;
using namespace hmservcommon::builders;

//-----------------------------------------------------------------------------
HMAccountBuilder::HMAccountBuilder(const std::shared_ptr<datastorage::HMDataStorage> inStorage) : m_storage(inStorage)
{
    assert(m_storage != nullptr);

    if (!m_storage->is_open()) // Если хранилище не открыто
    {
        std::error_code Error = m_storage->open(); // Пытаемся открыть самостоятельно

        if (Error)
            LOG_WARNING_EX(QString::fromStdString(Error.message()), this);
    }
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMGroup> HMAccountBuilder::buildGroup(const QUuid& inGroupUUID, std::error_code& outErrorCode)
{
    std::shared_ptr<hmcommon::HMGroup> Result = nullptr;
    outErrorCode = make_error_code(hmcommon::eSystemErrorEx::seSuccess); // Изначально помечаем как успех

    Result = m_storage->findGroupByUUID(inGroupUUID, outErrorCode);

    if (!outErrorCode) // Информация о группе сформирована успешно
    {
        // TODO Получить список саязей группа-пользователи и сформировать список польователей через findUserByUUID
    }

    if (outErrorCode) // Если при построении произошла ошибка
        Result = nullptr; // Сбрасываем результат

    return Result;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMAccount> HMAccountBuilder::buildAccount(const QUuid& inUserUUID, std::error_code& outErrorCode)
{
    std::shared_ptr<hmcommon::HMAccount> Result = nullptr;
    outErrorCode = make_error_code(hmcommon::eSystemErrorEx::seSuccess); // Изначально помечаем как успех

    /*
     * 1) Получить информацию о пользователе
     * 2) Получить список контактов
     * 3) Получить список групп
     */

    Result = std::make_shared<hmcommon::HMAccount>(); // Создаём экземпляр аккаунта

    Result->m_userInfo = m_storage->findUserByUUID(inUserUUID, outErrorCode); // Ищим данные о пользователе в хранилище

    if (!outErrorCode) // Информация о пользователе сформирована успешно
    {
        std::shared_ptr<std::set<QUuid>> UserContact = m_storage->getUserContactList(inUserUUID, outErrorCode); // Ищим список контактов пользователя

        if (!outErrorCode)
        {
            for (const QUuid& ContactUUID : *UserContact) // Перебираем список контактов
            {
                std::shared_ptr<hmcommon::HMUser> Contact = m_storage->findUserByUUID(ContactUUID, outErrorCode); // Ищим контакт

                if (outErrorCode) // Если ошибка при получении контакта
                    break; // Останавливаем перебор
                else // Контакт получен успешно
                    Result->m_cotacts.add(Contact); // Добавляем контакт в список
            }

            if (!outErrorCode) // Если список контактов сформирован успешно
            {
                std::shared_ptr<std::set<QUuid>> UserGroups = m_storage->getUserGroups(inUserUUID, outErrorCode); // Получаем список UUID'ов групп пользователя

                if (!outErrorCode) // Если UUID'ы группы успешно получены
                {
                    std::error_code FindGroupError = make_error_code(hmcommon::eSystemErrorEx::seSuccess); // Ошибки поиска групп обрабатываем отдельно

                    for (const QUuid& GroupUUID : *UserGroups)
                    {
                        std::shared_ptr<hmcommon::HMGroup> Group = m_storage->findGroupByUUID(GroupUUID, FindGroupError);

                        if (FindGroupError) // Если не улаось обнаружить группу
                            LOG_WARNING_EX(QString::fromStdString(FindGroupError.message()), this); // Сообщим об этом в логах
                        else // Валидную группу добавляем в аккаунт
                            Result->m_groups.emplace_back(std::move(Group));
                    }
                }
            }
        }

        if (outErrorCode) // Если при построении произошла ошибка
            Result = nullptr; // Сбрасываем результат
    }

    return Result;
}
//-----------------------------------------------------------------------------

