#include "combineddatastorage.h"

#include <cassert>

#include <HawkLog.h>
#include "systemerrorex.h"
#include "datastorage/datastorageerrorcategory.h"

using namespace hmservcommon::datastorage;

//-----------------------------------------------------------------------------
HMCombinedDataStorage::HMCombinedDataStorage(const std::shared_ptr<HMDataStorage> inHardStorage, const std::shared_ptr<HMDataStorage> inCacheStorage) :
    m_HardStorage(inHardStorage),
    m_CacheStorage(inCacheStorage)
{
    assert(m_HardStorage != nullptr);
}
//-----------------------------------------------------------------------------
std::error_code HMCombinedDataStorage::open()
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

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
std::error_code HMCombinedDataStorage::addUser(const std::shared_ptr<hmcommon::HMUser> inUser)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        if (!inUser) // Работаем только с валидным указателем
            Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
        else
        {
            Error = m_HardStorage->addUser(inUser); // Пытаемся добавить пользователя в физическое хранилище

            if (!Error && m_CacheStorage) // Если пользователь успешно добавлен в физическое хранилище и доступен кеш
            {
                std::error_code CacheError = m_CacheStorage->addUser(inUser); // Добавляем пользователя в кеш
                if (CacheError) // Ошибки кеша обрабатывам отдельно
                    LOG_WARNING_EX(QString::fromStdString(CacheError.message()), this);
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMCombinedDataStorage::updateUser(const std::shared_ptr<hmcommon::HMUser> inUser)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        if (!inUser) // Работаем только с валидным указателем
            Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
        else
        {
            Error = m_HardStorage->updateUser(inUser); // Обнавляем пользователя в физическом хранилище

            if (!Error && m_CacheStorage) // Если пользователь успешно обновлён в физическое хранилище и доступен кеш
            {
                std::error_code CacheError = m_CacheStorage->updateUser(inUser); // Обновляем данные о пользователе в кеше
                if (CacheError) // Ошибки кеша обрабатывам отдельно
                    LOG_WARNING_EX(QString::fromStdString(CacheError.message()), this);
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMUser> HMCombinedDataStorage::findUserByUUID(const QUuid &inUserUUID, std::error_code &outErrorCode) const
{
    std::shared_ptr<hmcommon::HMUser> Result = nullptr;
    outErrorCode = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open())
        outErrorCode = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        std::error_code CacheError = make_error_code(eDataStorageError::dsSuccess); // Отдельный результат для поиска в кеше

        if (!m_CacheStorage) // Если не доступен кеш
            CacheError = make_error_code(eDataStorageError::dsUserNotExists); // Помечаем результат поиска в кеше как ошибку
        else // Если доступен кеш
            Result = m_CacheStorage->findUserByUUID(inUserUUID, CacheError); // Сначала ищим пользователя в кеше

        if (CacheError.value() == static_cast<int32_t>(eDataStorageError::dsUserNotExists)) // Если в кеше не удалось найти пользователя
        {   // Ищим в физическом хранилище
            Result = m_HardStorage->findUserByUUID(inUserUUID, outErrorCode);

            if (!outErrorCode && m_CacheStorage) // Если пользователь успешно найден в физическое хранилище и доступен кеш
            {   // Добавим его в кеш
                CacheError = m_CacheStorage->addUser(Result);
                if (CacheError) // Ошибки кеша обрабатывам отдельно
                    LOG_WARNING_EX(QString::fromStdString(CacheError.message()), this);
            }
        }   // Поиск в самом хранилище
        else // В кеше встретелось что-то внезапное
            outErrorCode = CacheError; // Вернём ошибку из кеширующего хранилища

        if (outErrorCode) // Если ошибка поиска пользователя
            Result = nullptr; // На всякий случай сбросим результат
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMUser> HMCombinedDataStorage::findUserByAuthentication(const QString &inLogin, const QByteArray &inPasswordHash, std::error_code &outErrorCode) const
{
    std::shared_ptr<hmcommon::HMUser> Result = nullptr;
    outErrorCode = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open())
        outErrorCode = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        std::error_code CacheError = make_error_code(eDataStorageError::dsSuccess); // Отдельный результат для поиска в кеше

        if (!m_CacheStorage) // Если не доступен кеш
            CacheError = make_error_code(eDataStorageError::dsUserNotExists); // Помечаем результат поиска в кеше как ошибку
        else // Если доступен кеш
            Result = m_CacheStorage->findUserByAuthentication(inLogin, inPasswordHash, CacheError); // Сначала ищим пользователя в кеше

        if (CacheError.value() == static_cast<int32_t>(eDataStorageError::dsUserNotExists)) // Если в кеше не удалось найти пользователя
        {   // Ищим в физическом хранилище
            Result = m_HardStorage->findUserByAuthentication(inLogin, inPasswordHash, outErrorCode);

            if (!outErrorCode && m_CacheStorage) // Если пользователь успешно найден в физическое хранилище и доступен кеш
            {   // Добавим его в кеш
                CacheError = m_CacheStorage->addUser(Result);
                if (CacheError) // Ошибки кеша обрабатывам отдельно
                    LOG_WARNING_EX(QString::fromStdString(CacheError.message()), this);
            }
        }   // Поиск в самом хранилище
        else // В кеше встретелось что-то внезапное
            outErrorCode = CacheError; // Вернём ошибку из кеширующего хранилища

        if (outErrorCode) // Если ошибка поиска пользователя
            Result = nullptr; // На всякий случай сбросим результат
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::error_code HMCombinedDataStorage::removeUser(const QUuid& inUserUUID)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        Error = m_HardStorage->removeUser(inUserUUID); // Удаляем пользователя в физическом хранилище

        if (!Error && m_CacheStorage) // Если пользователь успешно удалён в физическое хранилище и доступен кеш
        {
            std::error_code CacheError = m_CacheStorage->removeUser(inUserUUID); // Удаляем пользователя из кеша
            if (CacheError) // Ошибки кеша обрабатывам отдельно
                LOG_WARNING_EX(QString::fromStdString(CacheError.message()), this);
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMCombinedDataStorage::addGroup(const std::shared_ptr<hmcommon::HMGroup> inGroup)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        if (!inGroup) // Работаем только с валидным указателем
            Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
        else
        {
            Error = m_HardStorage->addGroup(inGroup); // Пытаемся добавить группу в физическое хранилище

            if (!Error && m_CacheStorage) // Если группа успешно добавлена в физическое хранилище и доступен кеш
            {
                std::error_code CacheError = m_CacheStorage->addGroup(inGroup); // Добавляем группу в кеш
                if (CacheError) // Ошибки кеша обрабатывам отдельно
                    LOG_WARNING_EX(QString::fromStdString(CacheError.message()), this);
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMCombinedDataStorage::updateGroup(const std::shared_ptr<hmcommon::HMGroup> inGroup)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        if (!inGroup) // Работаем только с валидным указателем
            Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
        else
        {
            Error = m_HardStorage->updateGroup(inGroup); // Обнавляем группу в физическом хранилище

            if (!Error && m_CacheStorage) // Если группа успешно обновлёна в физическое хранилище и доступен кеш
            {
                std::error_code CacheError = m_CacheStorage->updateGroup(inGroup); // Обновляем данные о группе в кеше
                if (CacheError) // Ошибки кеша обрабатывам отдельно
                    LOG_WARNING_EX(QString::fromStdString(CacheError.message()), this);
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMGroup> HMCombinedDataStorage::findGroupByUUID(const QUuid &inGroupUUID, std::error_code &outErrorCode) const
{
    std::shared_ptr<hmcommon::HMGroup> Result = nullptr;
    outErrorCode = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open())
        outErrorCode = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        std::error_code CacheError = make_error_code(eDataStorageError::dsSuccess); // Отдельный результат для поиска в кеше

        if (!m_CacheStorage) // Если не доступен кеш
            CacheError = make_error_code(eDataStorageError::dsGroupNotExists); // Помечаем результат поиска в кеше как ошибку
        else // Если доступен кеш
            Result = m_CacheStorage->findGroupByUUID(inGroupUUID, CacheError); // Сначала ищим пользователя в кеше

        if (CacheError.value() == static_cast<int32_t>(eDataStorageError::dsGroupNotExists)) // Если в кеше не удалось найти группу
        {   // Ищим в физическом хранилище
            Result = m_HardStorage->findGroupByUUID(inGroupUUID, outErrorCode);

            if (!outErrorCode && m_CacheStorage) // Если группа успешно найдена в физическое хранилище и доступен кеш
            {   // Добавим её в кеш
                CacheError = m_CacheStorage->addGroup(Result);
                if (CacheError) // Ошибки кеша обрабатывам отдельно
                    LOG_WARNING_EX(QString::fromStdString(CacheError.message()), this);
            }
        }   // Поиск в самом хранилище
        else // В кеше встретелось что-то внезапное
            outErrorCode = CacheError; // Вернём ошибку из кеширующего хранилища

        if (outErrorCode) // Если ошибка поиска группы
            Result = nullptr; // На всякий случай сбросим результат
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::error_code HMCombinedDataStorage::removeGroup(const QUuid& inGroupUUID)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        Error = m_HardStorage->removeGroup(inGroupUUID); // Удаляем группу в физическом хранилище

        if (!Error && m_CacheStorage) // Если группа успешно удалёна в физическое хранилище и доступен кеш
        {
            std::error_code CacheError = m_CacheStorage->removeGroup(inGroupUUID); // Удаляем группу из кеша
            if (CacheError) // Ошибки кеша обрабатывам отдельно
                LOG_WARNING_EX(QString::fromStdString(CacheError.message()), this);
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMCombinedDataStorage::addMessage(const std::shared_ptr<hmcommon::HMGroupMessage> inMessage)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        if (!inMessage) // Работаем только с валидным указателем
            Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
        else
        {
            Error = m_HardStorage->addMessage(inMessage); // Пытаемся добавить сообщение в физическое хранилище

            if (!Error && m_CacheStorage) // Если сообщение успешно добавлено в физическое хранилище и доступен кеш
            {
                std::error_code CacheError = m_CacheStorage->addMessage(inMessage); // Добавляем сообщение в кеш
                if (CacheError) // Ошибки кеша обрабатывам отдельно
                    LOG_WARNING_EX(QString::fromStdString(CacheError.message()), this);
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMCombinedDataStorage::updateMessage(const std::shared_ptr<hmcommon::HMGroupMessage> inMessage)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        if (!inMessage) // Работаем только с валидным указателем
            Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
        else
        {
            Error = m_HardStorage->updateMessage(inMessage); // Обнавляем сообщение в физическом хранилище

            if (!Error && m_CacheStorage) // Если сообщение успешно обновлено в физическом хранилище и доступен кеш
            {
                std::error_code CacheError = m_CacheStorage->updateMessage(inMessage); // Обновляем данные о сообщение в кеше
                if (CacheError) // Ошибки кеша обрабатывам отдельно
                    LOG_WARNING_EX(QString::fromStdString(CacheError.message()), this);
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMGroupMessage> HMCombinedDataStorage::findMessage(const QUuid inMessageUUID, std::error_code& outErrorCode) const
{
    std::shared_ptr<hmcommon::HMGroupMessage> Result = nullptr;
    outErrorCode = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open())
        outErrorCode = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        std::error_code CacheError = make_error_code(eDataStorageError::dsSuccess); // Отдельный результат для поиска в кеше

        if (!m_CacheStorage) // Если не доступен кеш
            CacheError = make_error_code(eDataStorageError::dsMessageNotExists); // Помечаем результат поиска в кеше как ошибку
        else // Если доступен кеш
            Result = m_CacheStorage->findMessage(inMessageUUID, CacheError); // Сначала ищим сообщение в кеше

        if (CacheError.value() == static_cast<int32_t>(eDataStorageError::dsMessageNotExists)) // Если в кеше не удалось найти сообщение
        {   // Ищим в физическом хранилище
            Result = m_HardStorage->findMessage(inMessageUUID, outErrorCode);

            if (!outErrorCode && m_CacheStorage) // Если сообщение успешно найдено в физическом хранилище и доступен кеш
            {   // Добавим его в кеш
                CacheError = m_CacheStorage->addMessage(Result);
                if (CacheError) // Ошибки кеша обрабатывам отдельно
                    LOG_WARNING_EX(QString::fromStdString(CacheError.message()), this);
            }
        }   // Поиск в самом хранилище
        else // В кеше встретелось что-то внезапное
            outErrorCode = CacheError; // Вернём ошибку из кеширующего хранилища

        if (outErrorCode) // Если ошибка поиска сообщения
            Result = nullptr; // На всякий случай сбросим результат
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::vector<std::shared_ptr<hmcommon::HMGroupMessage>> HMCombinedDataStorage::findMessages(const QUuid inGroupUUID, const hmcommon::MsgRange& inRange,  std::error_code& outErrorCode) const
{
    std::vector<std::shared_ptr<hmcommon::HMGroupMessage>> Result;
    outErrorCode = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open())
        outErrorCode = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        std::error_code CacheError = make_error_code(eDataStorageError::dsSuccess); // Отдельный результат для поиска в кеше

        if (!m_CacheStorage) // Если не доступен кеш
            CacheError = make_error_code(eDataStorageError::dsMessageNotExists); // Помечаем результат поиска в кеше как ошибку
        else // Если доступен кеш
            Result = m_CacheStorage->findMessages(inGroupUUID, inRange, CacheError); // Сначала ищим сообщения в кеше

        if (CacheError.value() == static_cast<int32_t>(eDataStorageError::dsMessageNotExists)) // Если в кеше не удалось найти сообщения
        {   // Ищим в физическом хранилище
            Result = m_HardStorage->findMessages(inGroupUUID, inRange, outErrorCode);

            if (!outErrorCode && m_CacheStorage) // Если сообщения успешно найдены в физическом хранилище и доступен кеш
            {   // Добавим их в кеш
                for (const auto& Message : Result)
                {
                    CacheError = m_CacheStorage->addMessage(Message);
                    if (CacheError) // Ошибки кеша обрабатывам отдельно
                        LOG_WARNING_EX(QString::fromStdString(CacheError.message()), this);
                }
            }
        }   // Поиск в самом хранилище
        else // В кеше встретелось что-то внезапное
            outErrorCode = CacheError; // Вернём ошибку из кеширующего хранилища

        if (outErrorCode) // Если ошибка поиска сообщения
            Result.clear(); // На всякий случай сбросим результат
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::error_code HMCombinedDataStorage::removeMessage(const QUuid inMessageUUID, const QUuid inGroupUUID)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        Error = m_HardStorage->removeMessage(inMessageUUID, inGroupUUID); // Удаляем сообщение в физическом хранилище

        if (!Error && m_CacheStorage) // Если сообщение успешно удалено в физическом хранилище и доступен кеш
        {
            std::error_code CacheError = m_CacheStorage->removeMessage(inMessageUUID, inGroupUUID); // Удаляем сообщение из кеша
            if (CacheError) // Ошибки кеша обрабатывам отдельно
                LOG_WARNING_EX(QString::fromStdString(CacheError.message()), this);
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
