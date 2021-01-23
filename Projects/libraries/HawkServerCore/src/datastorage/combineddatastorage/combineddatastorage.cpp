#include "combineddatastorage.h"

#include <cassert>

#include <HawkLog.h>

#include <systemerrorex.h>
#include <datastorageerrorcategory.h>

using namespace hmservcommon::datastorage;

//-----------------------------------------------------------------------------
HMCombinedDataStorage::HMCombinedDataStorage(const std::shared_ptr<HMAbstractHardDataStorage> inHardStorage, const std::shared_ptr<HMAbstractCahceDataStorage> inCacheStorage) :
    m_HardStorage(inHardStorage),
    m_CacheStorage(inCacheStorage)
{
    assert(m_HardStorage != nullptr);
}
//-----------------------------------------------------------------------------
errors::error_code HMCombinedDataStorage::open()
{
    errors::error_code Error = make_error_code(errors::eDataStorageError::dsSuccess); // Изначально метим как успех

    close();

    Error = m_HardStorage->open(); // Сначала пытваемся открыть физическое хранилище

    if (!Error && m_CacheStorage) // Если физическое хранилище открылось и кеш доступен
    {
        Error = m_CacheStorage->open();

        if (Error) // Если при открытии кеширующего хранилища произошла ошибка
            close(); // Закрываем всё
    }

    return Error;
}
//-----------------------------------------------------------------------------
bool HMCombinedDataStorage::is_open() const
{
    bool Result = m_HardStorage->is_open(); // Проверяем открытие физического хранилища

    if (m_CacheStorage) // Если кеширующее хранилище доступно
        Result &= m_CacheStorage->is_open(); // Проверяем открытие кеширующего хранилища

    return Result;
}
//-----------------------------------------------------------------------------
void HMCombinedDataStorage::close()
{
    if  (m_HardStorage->is_open()) // Если физическое хранилище открыто
        m_HardStorage->close(); // Закрываем

    if (m_CacheStorage && m_CacheStorage->is_open()) // Если кеширующее хранилище доступно и открыто
        m_CacheStorage->close(); // Закрываем
}
//-----------------------------------------------------------------------------
errors::error_code HMCombinedDataStorage::addUser(const std::shared_ptr<hmcommon::HMUserInfo> inUser)
{
    errors::error_code Error = make_error_code(errors::eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(errors::eDataStorageError::dsNotOpen);
    else
    {
        if (!inUser) // Работаем только с валидным указателем
            Error = make_error_code(errors::eSystemErrorEx::seInvalidPtr);
        else
        {
            Error = m_HardStorage->addUser(inUser); // Пытаемся добавить пользователя в физическое хранилище

            if (!Error && m_CacheStorage) // Если пользователь успешно добавлен в физическое хранилище и доступен кеш
            {
                errors::error_code CacheError = m_CacheStorage->addUser(inUser); // Добавляем пользователя в кеш
                if (CacheError) // Ошибки кеша обрабатывам отдельно
                    LOG_WARNING(CacheError.message_qstr());
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
errors::error_code HMCombinedDataStorage::updateUser(const std::shared_ptr<hmcommon::HMUserInfo> inUser)
{
    errors::error_code Error = make_error_code(errors::eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(errors::eDataStorageError::dsNotOpen);
    else
    {
        if (!inUser) // Работаем только с валидным указателем
            Error = make_error_code(errors::eSystemErrorEx::seInvalidPtr);
        else
        {
            Error = m_HardStorage->updateUser(inUser); // Обнавляем пользователя в физическом хранилище

            if (!Error && m_CacheStorage) // Если пользователь успешно обновлён в физическое хранилище и доступен кеш
            {
                errors::error_code CacheError = m_CacheStorage->updateUser(inUser); // Обновляем данные о пользователе в кеше
                if (CacheError) // Не удалось обновить пользователя в кеше
                {
                    CacheError = m_CacheStorage->addUser(inUser); // Добавляем в кеш обновлённый объект
                    if (CacheError) // Если и добавление не прошло то обрабатываем ошибку отдельно
                        LOG_WARNING(CacheError.message_qstr());
                }
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMUserInfo> HMCombinedDataStorage::findUserByUUID(const QUuid &inUserUUID, errors::error_code &outErrorCode) const
{
    std::shared_ptr<hmcommon::HMUserInfo> Result = nullptr;
    outErrorCode = make_error_code(errors::eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open())
        outErrorCode = make_error_code(errors::eDataStorageError::dsNotOpen);
    else
    {
        errors::error_code CacheError = make_error_code(errors::eDataStorageError::dsUserNotExists); // Отдельный результат для поиска в кеше

        if (m_CacheStorage) // Если доступен кеш
        {
            Result = m_CacheStorage->findUserByUUID(inUserUUID, CacheError); // Сначала ищим пользователя в кеше
            if (CacheError && CacheError.value() != static_cast<int32_t>(errors::eDataStorageError::dsUserNotExists)) // Если ошибка отличается от "объект не найден"
                 LOG_WARNING(CacheError.message_qstr()); // Пишим её в лог
        }

        if (CacheError) // Если в кеше не удалось найти пользователя
        {   // Ищим в физическом хранилище
            Result = m_HardStorage->findUserByUUID(inUserUUID, outErrorCode);

            if (!outErrorCode && m_CacheStorage) // Если пользователь успешно найден в физическое хранилище и доступен кеш
            {   // Добавим его в кеш
                CacheError = m_CacheStorage->addUser(Result);
                if (CacheError) // Ошибки кеша обрабатывам отдельно
                    LOG_WARNING(CacheError.message_qstr());
            }
        }   // Поиск в самом хранилище

        if (outErrorCode) // Если ошибка поиска пользователя
            Result = nullptr; // На всякий случай сбросим результат
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMUserInfo> HMCombinedDataStorage::findUserByAuthentication(const QString &inLogin, const QByteArray &inPasswordHash, errors::error_code &outErrorCode) const
{
    std::shared_ptr<hmcommon::HMUserInfo> Result = nullptr;
    outErrorCode = make_error_code(errors::eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open())
        outErrorCode = make_error_code(errors::eDataStorageError::dsNotOpen);
    else
    {
        errors::error_code CacheError = make_error_code(errors::eDataStorageError::dsUserNotExists); // Отдельный результат для поиска в кеше

        if (m_CacheStorage) // Если доступен кеш
        {
            Result = m_CacheStorage->findUserByAuthentication(inLogin, inPasswordHash, CacheError); // Сначала ищим пользователя в кеше
            if (CacheError && CacheError.value() != static_cast<int32_t>(errors::eDataStorageError::dsUserNotExists)) // Если ошибка отличается от "объект не найден"
                 LOG_WARNING(CacheError.message_qstr()); // Пишим её в лог
        }

        if (CacheError.value() != static_cast<int32_t>(errors::eDataStorageError::dsUserPasswordIncorrect)) // Если просто не совпал пароль
            outErrorCode = CacheError; // Вернём эту ошибку
        else // Если не ошибка не корректного пароля
        {   // Проверям что всётаки ошибка!
            if (CacheError) // Если в кеше не удалось найти пользователя
            {   // Ищим в физическом хранилище
                Result = m_HardStorage->findUserByAuthentication(inLogin, inPasswordHash, outErrorCode);

                if (!outErrorCode && m_CacheStorage) // Если пользователь успешно найден в физическое хранилище и доступен кеш
                {   // Добавим его в кеш
                    CacheError = m_CacheStorage->addUser(Result);
                    if (CacheError) // Ошибки кеша обрабатывам отдельно
                        LOG_WARNING(CacheError.message_qstr());
                }
            }   // Поиск в самом хранилище
        }

        if (outErrorCode) // Если ошибка поиска пользователя
            Result = nullptr; // На всякий случай сбросим результат
    }

    return Result;
}
//-----------------------------------------------------------------------------
errors::error_code HMCombinedDataStorage::removeUser(const QUuid& inUserUUID)
{
    errors::error_code Error = make_error_code(errors::eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(errors::eDataStorageError::dsNotOpen);
    else
    {
        Error = m_HardStorage->removeUser(inUserUUID); // Удаляем пользователя в физическом хранилище

        if (!Error && m_CacheStorage) // Если пользователь успешно удалён в физическое хранилище и доступен кеш
        {
            errors::error_code CacheError = m_CacheStorage->removeUser(inUserUUID); // Удаляем пользователя из кеша
            if (CacheError) // Ошибки кеша обрабатывам отдельно
                LOG_WARNING(CacheError.message_qstr());
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
errors::error_code HMCombinedDataStorage::setUserContacts(const QUuid& inUserUUID, const std::shared_ptr<std::set<QUuid>> inContacts)
{
    errors::error_code Error = make_error_code(errors::eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(errors::eDataStorageError::dsNotOpen);
    else
    {
        if (!inContacts) // Работаем только с валидным указателем
            Error = make_error_code(errors::eSystemErrorEx::seInvalidPtr);
        else
        {
            Error = m_HardStorage->setUserContacts(inUserUUID, inContacts); // Пытаемся добавить связь в физическое хранилище

            if (!Error && m_CacheStorage) // Если связь успешно добавлена в физическое хранилище и доступен кеш
            {
                errors::error_code CacheError = m_CacheStorage->setUserContacts(inUserUUID, inContacts); // Добавляем связь в кеш
                if (CacheError) // Ошибки кеша обрабатывам отдельно
                    LOG_WARNING(CacheError.message_qstr());
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
errors::error_code HMCombinedDataStorage::addUserContact(const QUuid& inUserUUID, const QUuid& inContactUUID)
{
    errors::error_code Error = make_error_code(errors::eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(errors::eDataStorageError::dsNotOpen);
    else
    {
        Error = m_HardStorage->addUserContact(inUserUUID, inContactUUID); // Пытаемся добавить контакт в связь в физического хранилища

        if (!Error && m_CacheStorage) // Если контакт успешно добавлен в связь физического хранилища и доступен кеш
        {
            errors::error_code CacheError = m_CacheStorage->addUserContact(inUserUUID, inContactUUID); // Добавляем контакт в связь в кеше
            if (CacheError) // Ошибки кеша обрабатывам отдельно
                LOG_WARNING(CacheError.message_qstr());
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
errors::error_code HMCombinedDataStorage::removeUserContact(const QUuid& inUserUUID, const QUuid& inContactUUID)
{
    errors::error_code Error = make_error_code(errors::eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(errors::eDataStorageError::dsNotOpen);
    else
    {
        Error = m_HardStorage->removeUserContact(inUserUUID, inContactUUID); // Пытаемся удалить контакт из связи в физическом хранилище

        if (!Error && m_CacheStorage) // Если контакт успешно удалён из связи в физическом хранилище и доступен кеш
        {
            errors::error_code CacheError = m_CacheStorage->removeUserContact(inUserUUID, inContactUUID); // Удаляем контакт из связи в кеше
            if (CacheError) // Ошибки кеша обрабатывам отдельно
                LOG_WARNING(CacheError.message_qstr());
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
errors::error_code HMCombinedDataStorage::clearUserContacts(const QUuid& inUserUUID)
{
    errors::error_code Error = make_error_code(errors::eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(errors::eDataStorageError::dsNotOpen);
    else
    {
        Error = m_HardStorage->clearUserContacts(inUserUUID); // Пытаемся удалить связь в физическом хранилище

        if (!Error && m_CacheStorage) // Если связь удалена в физическом хранилище и доступен кеш
        {
            errors::error_code CacheError = m_CacheStorage->clearUserContacts(inUserUUID); // Удаляем связь в кеше
            if (CacheError) // Ошибки кеша обрабатывам отдельно
                LOG_WARNING(CacheError.message_qstr());
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::shared_ptr<std::set<QUuid>> HMCombinedDataStorage::getUserContactList(const QUuid& inUserUUID, errors::error_code& outErrorCode) const
{
    std::shared_ptr<std::set<QUuid>> Result = nullptr;
    outErrorCode = make_error_code(errors::eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        outErrorCode = make_error_code(errors::eDataStorageError::dsNotOpen);
    else
    {
        errors::error_code CacheError = make_error_code(errors::eDataStorageError::dsUserContactRelationNotExists); // Отдельный результат для поиска в кеше

        if (m_CacheStorage) // Если доступен кеш
        {
            Result = m_CacheStorage->getUserContactList(inUserUUID, CacheError); // Сначала ищим сообщения в кеше
            if (CacheError && CacheError.value() != static_cast<int32_t>(errors::eDataStorageError::dsUserContactRelationNotExists)) // Если ошибка отличается от "объект не найден"
                 LOG_WARNING(CacheError.message_qstr()); // Пишим её в лог
        }

        if (CacheError) // Если в кеше не удалось найти связь
        {   // Ищим в физическом хранилище
            Result = m_HardStorage->getUserContactList(inUserUUID, outErrorCode);

            if (!outErrorCode && m_CacheStorage) // Если список контактов успешно найден в физическом хранилище и доступен кеш
            {
                CacheError = m_CacheStorage->setUserContacts(inUserUUID, Result); // Добавим его в кеш
                if (CacheError) // Ошибки кеша обрабатывам отдельно
                    LOG_WARNING(CacheError.message_qstr());
            }
        }   // Поиск в самом хранилище

        if (outErrorCode) // Если ошибка поиска сообщения
            Result = nullptr; // На всякий случай сбросим результат
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::shared_ptr<std::set<QUuid>> HMCombinedDataStorage::getUserGroups(const QUuid& inUserUUID, errors::error_code& outErrorCode) const
{
    std::shared_ptr<std::set<QUuid>> Result = nullptr;
    outErrorCode = make_error_code(errors::eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        outErrorCode = make_error_code(errors::eDataStorageError::dsNotOpen);
    else
    {
        errors::error_code CacheError = make_error_code(errors::eDataStorageError::dsUserGroupsRelationNotExists); // Отдельный результат для поиска в кеше

        if (m_CacheStorage) // Если доступен кеш
        {
            Result = m_CacheStorage->getUserGroups(inUserUUID, CacheError); // Сначала ищим сообщения в кеше
            if (CacheError && CacheError.value() != static_cast<int32_t>(errors::eDataStorageError::dsUserGroupsRelationNotExists)) // Если ошибка отличается от "объект не найден"
                 LOG_WARNING(CacheError.message_qstr()); // Пишим её в лог
        }

        if (CacheError) // Если в кеше не удалось найти связь
        {   // Ищим в физическом хранилище
            Result = m_HardStorage->getUserGroups(inUserUUID, outErrorCode);

            // !ОСОБЫЙ СЛУЧАЙ! Если в кеше не найдено значение и взято в физическом хранилище то не требуется писать его в кеш

        }   // Поиск в самом хранилище

        if (outErrorCode) // Если ошибка поиска сообщения
            Result = nullptr; // На всякий случай сбросим результат
    }

    return Result;
}
//-----------------------------------------------------------------------------
errors::error_code HMCombinedDataStorage::addGroup(const std::shared_ptr<hmcommon::HMGroupInfo> inGroup)
{
    errors::error_code Error = make_error_code(errors::eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(errors::eDataStorageError::dsNotOpen);
    else
    {
        if (!inGroup) // Работаем только с валидным указателем
            Error = make_error_code(errors::eSystemErrorEx::seInvalidPtr);
        else
        {
            Error = m_HardStorage->addGroup(inGroup); // Пытаемся добавить группу в физическое хранилище

            if (!Error && m_CacheStorage) // Если группа успешно добавлена в физическое хранилище и доступен кеш
            {
                errors::error_code CacheError = m_CacheStorage->addGroup(inGroup); // Добавляем группу в кеш
                if (CacheError) // Ошибки кеша обрабатывам отдельно
                    LOG_WARNING(CacheError.message_qstr());
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
errors::error_code HMCombinedDataStorage::updateGroup(const std::shared_ptr<hmcommon::HMGroupInfo> inGroup)
{
    errors::error_code Error = make_error_code(errors::eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(errors::eDataStorageError::dsNotOpen);
    else
    {
        if (!inGroup) // Работаем только с валидным указателем
            Error = make_error_code(errors::eSystemErrorEx::seInvalidPtr);
        else
        {
            Error = m_HardStorage->updateGroup(inGroup); // Обнавляем группу в физическом хранилище

            if (!Error && m_CacheStorage) // Если группа успешно обновлёна в физическое хранилище и доступен кеш
            {
                errors::error_code CacheError = m_CacheStorage->updateGroup(inGroup); // Обновляем данные о группе в кеше
                if (CacheError) // Если не удалось обновить объект
                {
                    CacheError = m_CacheStorage->addGroup(inGroup); // То добавляем в кеш уже обновлённый объект
                    if (CacheError) // Если и добавление не прошло
                        LOG_WARNING(CacheError.message_qstr()); // Обрабатываем ошибку
                }
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMGroupInfo> HMCombinedDataStorage::findGroupByUUID(const QUuid &inGroupUUID, errors::error_code &outErrorCode) const
{
    std::shared_ptr<hmcommon::HMGroupInfo> Result = nullptr;
    outErrorCode = make_error_code(errors::eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open())
        outErrorCode = make_error_code(errors::eDataStorageError::dsNotOpen);
    else
    {
        errors::error_code CacheError = make_error_code(errors::eDataStorageError::dsGroupNotExists); // Отдельный результат для поиска в кеше

        if (m_CacheStorage) // Если доступен кеш
        {
            Result = m_CacheStorage->findGroupByUUID(inGroupUUID, CacheError); // Сначала ищим пользователя в кеше
            if (CacheError && CacheError.value() != static_cast<int32_t>(errors::eDataStorageError::dsGroupNotExists)) // Если ошибка отличается от "объект не найден"
                 LOG_WARNING(CacheError.message_qstr()); // Пишим её в лог
        }

        if (CacheError) // Если в кеше не удалось найти группу
        {   // Ищим в физическом хранилище
            Result = m_HardStorage->findGroupByUUID(inGroupUUID, outErrorCode);

            if (!outErrorCode && m_CacheStorage) // Если группа успешно найдена в физическое хранилище и доступен кеш
            {   // Добавим её в кеш
                CacheError = m_CacheStorage->addGroup(Result);
                if (CacheError) // Ошибки кеша обрабатывам отдельно
                    LOG_WARNING(CacheError.message_qstr());
            }
        }   // Поиск в самом хранилище

        if (outErrorCode) // Если ошибка поиска группы
            Result = nullptr; // На всякий случай сбросим результат
    }

    return Result;
}
//-----------------------------------------------------------------------------
errors::error_code HMCombinedDataStorage::removeGroup(const QUuid& inGroupUUID)
{
    errors::error_code Error = make_error_code(errors::eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(errors::eDataStorageError::dsNotOpen);
    else
    {
        Error = m_HardStorage->removeGroup(inGroupUUID); // Удаляем группу в физическом хранилище

        if (!Error && m_CacheStorage) // Если группа успешно удалёна в физическое хранилище и доступен кеш
        {
            errors::error_code CacheError = m_CacheStorage->removeGroup(inGroupUUID); // Удаляем группу из кеша
            if (CacheError) // Ошибки кеша обрабатывам отдельно
                LOG_WARNING(CacheError.message_qstr());
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
errors::error_code HMCombinedDataStorage::setGroupUsers(const QUuid& inGroupUUID, const std::shared_ptr<std::set<QUuid>> inUsers)
{
    errors::error_code Error = make_error_code(errors::eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(errors::eDataStorageError::dsNotOpen);
    else
    {
        Error = m_HardStorage->setGroupUsers(inGroupUUID, inUsers); // Пытаемся добавить связь в физическое хранилище

        if (!Error && m_CacheStorage) // Если связь успешно добавлена в физическое хранилище и доступен кеш
        {
            errors::error_code CacheError = m_CacheStorage->setGroupUsers(inGroupUUID, inUsers); // Добавляем связь в кеш
            if (CacheError) // Ошибки кеша обрабатывам отдельно
                LOG_WARNING(CacheError.message_qstr());
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
errors::error_code HMCombinedDataStorage::addGroupUser(const QUuid& inGroupUUID, const QUuid& inUserUUID)
{
    errors::error_code Error = make_error_code(errors::eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(errors::eDataStorageError::dsNotOpen);
    else
    {
        Error = m_HardStorage->addGroupUser(inGroupUUID, inUserUUID); // Пытаемся добавить пользователя в связь в физического хранилища

        if (!Error && m_CacheStorage) // Если пользователь успешно добавлен в связь физического хранилища и доступен кеш
        {
            errors::error_code CacheError = m_CacheStorage->addGroupUser(inGroupUUID, inUserUUID); // Добавляем пользователя в связь в кеше
            if (CacheError) // Ошибки кеша обрабатывам отдельно
                LOG_WARNING(CacheError.message_qstr());
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
errors::error_code HMCombinedDataStorage::removeGroupUser(const QUuid& inGroupUUID, const QUuid& inUserUUID)
{
    errors::error_code Error = make_error_code(errors::eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(errors::eDataStorageError::dsNotOpen);
    else
    {
        Error = m_HardStorage->removeGroupUser(inGroupUUID, inUserUUID); // Пытаемся удалить контакт из связи в физическом хранилище

        if (!Error && m_CacheStorage) // Если контакт успешно удалён из связи в физическом хранилище и доступен кеш
        {
            errors::error_code CacheError = m_CacheStorage->removeGroupUser(inGroupUUID, inUserUUID); // Удаляем контакт из связи в кеше
            if (CacheError) // Ошибки кеша обрабатывам отдельно
                LOG_WARNING(CacheError.message_qstr());
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
errors::error_code HMCombinedDataStorage::clearGroupUsers(const QUuid& inGroupUUID)
{
    errors::error_code Error = make_error_code(errors::eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(errors::eDataStorageError::dsNotOpen);
    else
    {
        Error = m_HardStorage->clearGroupUsers(inGroupUUID); // Пытаемся удалить связь в физическом хранилище

        if (!Error && m_CacheStorage) // Если связь удалена в физическом хранилище и доступен кеш
        {
            errors::error_code CacheError = m_CacheStorage->clearGroupUsers(inGroupUUID); // Удаляем связь в кеше
            if (CacheError) // Ошибки кеша обрабатывам отдельно
                LOG_WARNING(CacheError.message_qstr());
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::shared_ptr<std::set<QUuid>> HMCombinedDataStorage::getGroupUserList(const QUuid& inGroupUUID, errors::error_code& outErrorCode) const
{
    std::shared_ptr<std::set<QUuid>> Result = nullptr;
    outErrorCode = make_error_code(errors::eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        outErrorCode = make_error_code(errors::eDataStorageError::dsNotOpen);
    else
    {
        errors::error_code CacheError = make_error_code(errors::eDataStorageError::dsGroupUserRelationNotExists); // Отдельный результат для поиска в кеше

        if (m_CacheStorage) // Если доступен кеш
        {
            Result = m_CacheStorage->getGroupUserList(inGroupUUID, CacheError); // Сначала ищим сообщения в кеше
            if (CacheError && CacheError.value() != static_cast<int32_t>(errors::eDataStorageError::dsGroupUserRelationNotExists)) // Если ошибка отличается от "объект не найден"
                 LOG_WARNING(CacheError.message_qstr()); // Пишим её в лог
        }

        if (CacheError) // Если в кеше не удалось найти связь
        {   // Ищим в физическом хранилище
            Result = m_HardStorage->getGroupUserList(inGroupUUID, outErrorCode);

            if (!outErrorCode && m_CacheStorage) // Если список окнтактов успешно найден в физическом хранилище и доступен кеш
            {
                CacheError = m_CacheStorage->setGroupUsers(inGroupUUID, Result); // Добавим его в кеш
                if (CacheError) // Ошибки кеша обрабатывам отдельно
                    LOG_WARNING(CacheError.message_qstr());
            }
        }   // Поиск в самом хранилище

        if (outErrorCode) // Если ошибка поиска сообщения
            Result = nullptr; // На всякий случай сбросим результат
    }

    return Result;
}
//-----------------------------------------------------------------------------
errors::error_code HMCombinedDataStorage::addMessage(const std::shared_ptr<hmcommon::HMGroupInfoMessage> inMessage)
{
    errors::error_code Error = make_error_code(errors::eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(errors::eDataStorageError::dsNotOpen);
    else
    {
        if (!inMessage) // Работаем только с валидным указателем
            Error = make_error_code(errors::eSystemErrorEx::seInvalidPtr);
        else
        {
            Error = m_HardStorage->addMessage(inMessage); // Пытаемся добавить сообщение в физическое хранилище

            if (!Error && m_CacheStorage) // Если сообщение успешно добавлено в физическое хранилище и доступен кеш
            {
                errors::error_code CacheError = m_CacheStorage->addMessage(inMessage); // Добавляем сообщение в кеш
                if (CacheError) // Ошибки кеша обрабатывам отдельно
                    LOG_WARNING(CacheError.message_qstr());
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
errors::error_code HMCombinedDataStorage::updateMessage(const std::shared_ptr<hmcommon::HMGroupInfoMessage> inMessage)
{
    errors::error_code Error = make_error_code(errors::eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(errors::eDataStorageError::dsNotOpen);
    else
    {
        if (!inMessage) // Работаем только с валидным указателем
            Error = make_error_code(errors::eSystemErrorEx::seInvalidPtr);
        else
        {
            Error = m_HardStorage->updateMessage(inMessage); // Обнавляем сообщение в физическом хранилище

            if (!Error && m_CacheStorage) // Если сообщение успешно обновлено в физическом хранилище и доступен кеш
            {
                errors::error_code CacheError = m_CacheStorage->updateMessage(inMessage); // Обновляем данные о сообщение в кеше
                if (CacheError) // Ошибки кеша обрабатывам отдельно
                    LOG_WARNING(CacheError.message_qstr());
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMGroupInfoMessage> HMCombinedDataStorage::findMessage(const QUuid& inMessageUUID, errors::error_code& outErrorCode) const
{
    std::shared_ptr<hmcommon::HMGroupInfoMessage> Result = nullptr;
    outErrorCode = make_error_code(errors::eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open())
        outErrorCode = make_error_code(errors::eDataStorageError::dsNotOpen);
    else
    {
        errors::error_code CacheError = make_error_code(errors::eDataStorageError::dsMessageNotExists); // Отдельный результат для поиска в кеше

        if (m_CacheStorage) // Если доступен кеш
        {
            Result = m_CacheStorage->findMessage(inMessageUUID, CacheError); // Сначала ищим сообщение в кеше
            if (CacheError && CacheError.value() != static_cast<int32_t>(errors::eDataStorageError::dsMessageNotExists)) // Если ошибка отличается от "объект не найден"
                 LOG_WARNING(CacheError.message_qstr()); // Пишим её в лог
        }

        if (CacheError) // Если в кеше не удалось найти сообщение
        {   // Ищим в физическом хранилище
            Result = m_HardStorage->findMessage(inMessageUUID, outErrorCode);

            if (!outErrorCode && m_CacheStorage) // Если сообщение успешно найдено в физическом хранилище и доступен кеш
            {   // Добавим его в кеш
                CacheError = m_CacheStorage->addMessage(Result);
                if (CacheError) // Ошибки кеша обрабатывам отдельно
                    LOG_WARNING(CacheError.message_qstr());
            }
        }   // Поиск в самом хранилище

        if (outErrorCode) // Если ошибка поиска сообщения
            Result = nullptr; // На всякий случай сбросим результат
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::vector<std::shared_ptr<hmcommon::HMGroupInfoMessage>> HMCombinedDataStorage::findMessages(const QUuid& inGroupUUID, const hmcommon::MsgRange& inRange,  errors::error_code& outErrorCode) const
{
    std::vector<std::shared_ptr<hmcommon::HMGroupInfoMessage>> Result;
    outErrorCode = make_error_code(errors::eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open())
        outErrorCode = make_error_code(errors::eDataStorageError::dsNotOpen);
    else
    {
        errors::error_code CacheError = make_error_code(errors::eDataStorageError::dsMessageNotExists); // Отдельный результат для поиска в кеше

        if (m_CacheStorage) // Если доступен кеш
        {
            Result = m_CacheStorage->findMessages(inGroupUUID, inRange, CacheError); // Сначала ищим сообщения в кеше
            if (CacheError && CacheError.value() != static_cast<int32_t>(errors::eDataStorageError::dsMessageNotExists)) // Если ошибка отличается от "объект не найден"
                 LOG_WARNING(CacheError.message_qstr()); // Пишим её в лог
        }

        if (CacheError) // Если в кеше не удалось найти сообщения
        {   // Ищим в физическом хранилище
            Result = m_HardStorage->findMessages(inGroupUUID, inRange, outErrorCode);

            if (!outErrorCode && m_CacheStorage) // Если сообщения успешно найдены в физическом хранилище и доступен кеш
            {   // Добавим их в кеш
                for (const auto& Message : Result)
                {
                    CacheError = m_CacheStorage->addMessage(Message);
                    if (CacheError) // Ошибки кеша обрабатывам отдельно
                        LOG_WARNING(CacheError.message_qstr());
                }
            }
        }   // Поиск в самом хранилище

        if (outErrorCode) // Если ошибка поиска сообщения
            Result.clear(); // На всякий случай сбросим результат
    }

    return Result;
}
//-----------------------------------------------------------------------------
errors::error_code HMCombinedDataStorage::removeMessage(const QUuid& inMessageUUID, const QUuid& inGroupUUID)
{
    errors::error_code Error = make_error_code(errors::eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(errors::eDataStorageError::dsNotOpen);
    else
    {
        Error = m_HardStorage->removeMessage(inMessageUUID, inGroupUUID); // Удаляем сообщение в физическом хранилище

        if (!Error && m_CacheStorage) // Если сообщение успешно удалено в физическом хранилище и доступен кеш
        {
            errors::error_code CacheError = m_CacheStorage->removeMessage(inMessageUUID, inGroupUUID); // Удаляем сообщение из кеша
            if (CacheError) // Ошибки кеша обрабатывам отдельно
                LOG_WARNING(CacheError.message_qstr());
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
