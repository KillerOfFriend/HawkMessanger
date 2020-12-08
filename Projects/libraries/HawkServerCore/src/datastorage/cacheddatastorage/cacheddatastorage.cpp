#include "cacheddatastorage.h"

#include <cassert>

#include <HawkLog.h>

#include "systemerrorex.h"
#include "datastorage/datastorageerrorcategory.h"

using namespace hmservcommon::datastorage;

//-----------------------------------------------------------------------------
HMCachedDataStorage::HMCachedDataStorage() :
    HMAbstractDataStorageFunctional(false) // Инициализируем предка и запрещаем его создавать хеш (ВО ИЗБЕЩАНИИ ФАТАЛЬНОЙ РЕКУРСИИ ПРИ ИНИЦИАЛИЗАЦИИ)
{
    std::error_code Error = makeDefault();

    if (Error)
        LOG_ERROR(QString::fromStdString(Error.message()));
}
//-----------------------------------------------------------------------------
HMCachedDataStorage::~HMCachedDataStorage()
{
    close();
}
//-----------------------------------------------------------------------------
std::error_code HMCachedDataStorage::open()
{
    close();
    return make_error_code(eDataStorageError::dsSuccess); // Кеширующее хранилище всегда открывается успешно
}
//-----------------------------------------------------------------------------
bool HMCachedDataStorage::is_open() const
{
    return true; // Кеширующее хранилище всегда считается открытым
}
//-----------------------------------------------------------------------------
void HMCachedDataStorage::close()
{    
    clearCached(); // При закрытии чистим хеш
}
//-----------------------------------------------------------------------------
std::error_code HMCachedDataStorage::addUser(const std::shared_ptr<hmcommon::HMUser> inUser)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально помечаем как успех

    if (!inUser) // Проверяем указатель на валидность
        Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
    else
    {
       if (!m_cachedUsers.emplace(HMCachedUser(inUser)).second) // Если пользователь не удалось закинуть в хеш
           Error = make_error_code(eDataStorageError::dsUserAlreadyExists);
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMCachedDataStorage::updateUser(const std::shared_ptr<hmcommon::HMUser> inUser)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально помечаем как успех

    if (!inUser) // Проверяем указатель на валидность
        Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
    else
    {
        // ЕСЛИ КОНЦЕПЦИЯ ОДНОГО ОБЪЕКТА РАБОТАЕТ ТО И ОБНОВЛЕНИЕ НА УРОВНЕ ХЕША УЖЕ ДОЛЖНО ПРОИЗОЙТИ
        // Достаточно проверить что этот объект уже в кеше и указатели равны
        std::shared_ptr<hmcommon::HMUser> FindRes = findUserByUUID(inUser->m_uuid, Error, true);

        if (FindRes != inUser)
            Error = make_error_code(hmcommon::eSystemErrorEx::seIncorretData);
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMUser> HMCachedDataStorage::findUserByUUID(const QUuid& inUserUUID, std::error_code& outErrorCode, const bool inWithContacts) const
{
    Q_UNUSED(inWithContacts);

    std::shared_ptr<hmcommon::HMUser> Result = nullptr;
    outErrorCode = make_error_code(eDataStorageError::dsSuccess); // Изначально помечаем как успех

    auto FindRes = m_cachedUsers.find(HMCachedUser(std::make_shared<hmcommon::HMUser>(inUserUUID))); // Ищим пользователя в хеше

    if (FindRes == m_cachedUsers.end()) // Нет пользователя в хеше
        outErrorCode = make_error_code(eDataStorageError::dsUserNotExists);
    else // Пользователь хеширован
    {
        Result = FindRes->m_user; // Вернём указатель на хешированного пользователя
        FindRes->m_lastRequest = QTime::currentTime(); // Помечаем время последнего запроса
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMUser> HMCachedDataStorage::findUserByAuthentication(const QString& inLogin, const QByteArray& inPasswordHash, std::error_code& outErrorCode, const bool inWithContacts) const
{
     Q_UNUSED(inWithContacts);

    std::shared_ptr<hmcommon::HMUser> Result = nullptr;
    outErrorCode = make_error_code(eDataStorageError::dsSuccess); // Изначально помечаем как успех

    // Ничего не поделаешь, перебираем
    auto FindRes = std::find_if(m_cachedUsers.cbegin(), m_cachedUsers.cend(), [&inLogin, &inPasswordHash](const HMCachedUser& CachedUser)
    {
        if (!CachedUser.m_user)
            return false;
        else
            return (CachedUser.m_user->getLogin() == inLogin) && (CachedUser.m_user->getPasswordHash() == inPasswordHash);
    });

    if (FindRes == m_cachedUsers.end()) // Нет пользователя в хеше
        outErrorCode = make_error_code(eDataStorageError::dsUserNotExists);
    else // Пользователь хеширован
    {
        Result = FindRes->m_user; // Вернём указатель на хешированного пользователя
        FindRes->m_lastRequest = QTime::currentTime(); // Помечаем время последнего запроса
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::error_code HMCachedDataStorage::removeUser(const QUuid& inUserUUID)
{
    auto FindRes = m_cachedUsers.find(HMCachedUser(std::make_shared<hmcommon::HMUser>(inUserUUID))); // Ищим пользователя в хеше

    if (FindRes != m_cachedUsers.end()) // Если пользователь найден
        m_cachedUsers.erase(FindRes); // Удаляем его из хеша

    return make_error_code(eDataStorageError::dsSuccess); // Наплевать, был пользователь в хеше или нет
}
//-----------------------------------------------------------------------------
std::error_code HMCachedDataStorage::addGroup(const std::shared_ptr<hmcommon::HMGroup> inGroup)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально помечаем как успех

    if (!inGroup) // Проверяем указатель на валидность
        Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
    else
    {
        if (!m_cachedGroups.emplace(HMCachedGroup(inGroup)).second)
            Error = make_error_code(eDataStorageError::dsGroupAlreadyExists);
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMCachedDataStorage::updateGroup(const std::shared_ptr<hmcommon::HMGroup> inGroup)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально помечаем как успех

    if (!inGroup) // Проверяем указатель на валидность
        Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
    else
    {
        // ЕСЛИ КОНЦЕПЦИЯ ОДНОГО ОБЪЕКТА РАБОТАЕТ ТО И ОБНОВЛЕНИЕ НА УРОВНЕ ХЕША УЖЕ ДОЛЖНО ПРОИЗОЙТИ
        // Достаточно проверить что этот объект уже в кеше и указатели равны
        std::shared_ptr<hmcommon::HMGroup> FindRes = findGroupByUUID(inGroup->m_uuid, Error);

        if (FindRes != inGroup)
            Error = make_error_code(hmcommon::eSystemErrorEx::seIncorretData);
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMGroup> HMCachedDataStorage::findGroupByUUID(const QUuid& inGroupUUID, std::error_code& outErrorCode) const
{
    std::shared_ptr<hmcommon::HMGroup> Result = nullptr;
    outErrorCode = make_error_code(eDataStorageError::dsSuccess); // Изначально помечаем как успех

    auto FindRes = m_cachedGroups.find(HMCachedGroup(std::make_shared<hmcommon::HMGroup>(inGroupUUID))); // Ищим группу в хеше

    if (FindRes == m_cachedGroups.end()) // Нет группы в хеше
        outErrorCode = make_error_code(eDataStorageError::dsGroupNotExists);
    else // Группа хеширована
    {
        Result = FindRes->m_group; // Вернём указатель на хешированную группу
        FindRes->m_lastRequest = QTime::currentTime(); // Помечаем время последнего запроса
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::error_code HMCachedDataStorage::removeGroup(const QUuid& inGroupUUID)
{
    auto FindRes = m_cachedGroups.find(HMCachedGroup(std::make_shared<hmcommon::HMGroup>(inGroupUUID))); // Ищим пользователя в хеше

    if (FindRes != m_cachedGroups.end()) // Если группа найдена
        m_cachedGroups.erase(FindRes); // Удаляем её из хеша

    return make_error_code(eDataStorageError::dsSuccess); // Наплевать, была группа в хеше или нет
}
//-----------------------------------------------------------------------------
std::error_code HMCachedDataStorage::addMessage(const std::shared_ptr<hmcommon::HMGroupMessage> inMessage)
{
    Q_UNUSED(inMessage);
    return make_error_code(hmcommon::eSystemErrorEx::seSuccess); // Много чести хешировать сообщения
}
//-----------------------------------------------------------------------------
std::error_code HMCachedDataStorage::updateMessage(const std::shared_ptr<hmcommon::HMGroupMessage> inMessage)
{
    Q_UNUSED(inMessage);
    return make_error_code(hmcommon::eSystemErrorEx::seSuccess); // Много чести хешировать сообщения
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMGroupMessage> HMCachedDataStorage::findMessage(const QUuid inMessageUUID, std::error_code& outErrorCode) const
{
    Q_UNUSED(inMessageUUID);
    outErrorCode = make_error_code(hmcommon::eSystemErrorEx::seNotInContainer); // Чесно говорим, что сообщение не кешировано
    return nullptr;
}
//-----------------------------------------------------------------------------
std::vector<std::shared_ptr<hmcommon::HMGroupMessage>> HMCachedDataStorage::findMessages(const QUuid inGroupUUID, const hmcommon::MsgRange& inRange,  std::error_code& outErrorCode) const
{
    Q_UNUSED(inGroupUUID);
    Q_UNUSED(inRange);
    outErrorCode = make_error_code(hmcommon::eSystemErrorEx::seNotInContainer); // Чесно говорим, что сообщения не кешированы
    return std::vector<std::shared_ptr<hmcommon::HMGroupMessage>>();
}
//-----------------------------------------------------------------------------
std::error_code HMCachedDataStorage::removeMessage(const QUuid inMessageUUID, const QUuid inGroupUUID)
{
    Q_UNUSED(inMessageUUID);
    Q_UNUSED(inGroupUUID);
    return make_error_code(eDataStorageError::dsSuccess); // Наплевать, было сообщение в хеше или нет
}
//-----------------------------------------------------------------------------
std::error_code HMCachedDataStorage::makeDefault()
{
    clearCached();
    return make_error_code(eDataStorageError::dsSuccess); // Кеширующее хранилище не требует создания структуры
}
//-----------------------------------------------------------------------------
void HMCachedDataStorage::clearCached()
{
    m_cachedGroups.clear();
    m_cachedUsers.clear();
}
//-----------------------------------------------------------------------------
