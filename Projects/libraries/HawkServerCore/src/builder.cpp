#include "builder.h"

#include <HawkLog.h>

#include <systemerrorex.h>

using namespace hmservcommon;

//-----------------------------------------------------------------------------
HMBuilder::HMBuilder(const std::shared_ptr<datastorage::HMDataStorage> inStorage) :
    m_storage(inStorage)
{
    assert(m_storage != nullptr);

    if (!m_storage->is_open()) // Если хранилище не открыто
    {
        hmcommon::error_code Error = m_storage->open(); // Пытаемся открыть самостоятельно

        if (Error)
            LOG_WARNING(Error.message_qstr());
    }
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMGroup> HMBuilder::buildGroup(const QUuid& inGroupUUID, hmcommon::error_code& outErrorCode)
{
    std::shared_ptr<hmcommon::HMGroup> Result = std::make_shared<hmcommon::HMGroup>();
    outErrorCode = make_error_code(hmcommon::eSystemErrorEx::seSuccess); // Изначально помечаем как успех

    Result->m_info = m_storage->findGroupByUUID(inGroupUUID, outErrorCode);

    if (!outErrorCode) // Информация о группе сформирована успешно
    {
        std::shared_ptr<std::set<QUuid>> Users = m_storage->getGroupUserList(inGroupUUID, outErrorCode);

        if (!outErrorCode) // Перечень UUID'ов участников группы получен успешно
        {
            for(const QUuid& UserUuid : *Users) // Перебираем UUID'ы участников группы
            {
                std::shared_ptr<hmcommon::HMUserInfo> UserInfo = m_storage->findUserByUUID(UserUuid, outErrorCode); // Запрашиваем данные участника

                if (outErrorCode) // Не удалось пролучит даныне
                    break; // Сбрасываем перебор
                else // Данные успешно получены
                {
                    hmcommon::error_code AddError = Result->m_users.add(UserInfo); // Добавляем участника
                    if (!AddError)
                        LOG_ERROR(AddError.message_qstr());
                }
            }
        }
    }

    if (outErrorCode) // Если при построении произошла ошибка
        Result = nullptr; // Сбрасываем результат

    return Result;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMUser> HMBuilder::buildUser(const QUuid& inUserUUID, hmcommon::error_code& outErrorCode)
{
    std::shared_ptr<hmcommon::HMUser> Result = std::make_shared<hmcommon::HMUser>();
    outErrorCode = make_error_code(hmcommon::eSystemErrorEx::seSuccess); // Изначально помечаем как успех

    Result->m_info = m_storage->findUserByUUID(inUserUUID, outErrorCode);

    if (!outErrorCode)
    {
        std::shared_ptr<std::set<QUuid>> Users = m_storage->getUserContactList(inUserUUID, outErrorCode);

        if (!outErrorCode) // Перечень UUID'ов контактов пользователя получен успешно
        {
            for(const QUuid& UserUuid : *Users) // Перебираем UUID'ы контактов пользователя
            {
                std::shared_ptr<hmcommon::HMUserInfo> UserInfo = m_storage->findUserByUUID(UserUuid, outErrorCode); // Запрашиваем данные участника

                if (outErrorCode) // Не удалось пролучит даныне
                    break; // Сбрасываем перебор
                else // Данные успешно получены
                {
                    hmcommon::error_code AddError = Result->m_contacts.add(UserInfo); // Добавляем контакт
                    if (!AddError)
                        LOG_ERROR(AddError.message_qstr());
                }
            }

            if (!outErrorCode) // Перечень контактов успешно получен
            {
                std::shared_ptr<std::set<QUuid>> Groups = m_storage->getUserGroups(inUserUUID, outErrorCode);

                if (!outErrorCode) // Перечень UUID'ов групп пользователя получен успешно
                {
                    for(const QUuid& GroupUuid : *Groups) // Перебираем UUID'ы групп пользователя
                    {
                        std::shared_ptr<hmcommon::HMGroup> Group = buildGroup(GroupUuid, outErrorCode);

                        if (outErrorCode) // Не удалось пролучит даныне
                            break; // Сбрасываем перебор
                        else // Данные успешно получены
                        {
                            hmcommon::error_code AddError = Result->m_groups.add(Group); // Добавляем группу
                            if (!AddError)
                                LOG_ERROR(AddError.message_qstr());
                        }
                    }
                }
            }
        }
    }

    if (outErrorCode) // Если при построении произошла ошибка
        Result = nullptr; // Сбрасываем результат

    return Result;
}
//-----------------------------------------------------------------------------
