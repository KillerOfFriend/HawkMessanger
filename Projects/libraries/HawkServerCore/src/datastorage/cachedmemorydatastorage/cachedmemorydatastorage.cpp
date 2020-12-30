#include "cachedmemorydatastorage.h"

#include <cassert>
#include <algorithm>

#include <HawkLog.h>

#include "systemerrorex.h"
#include "datastorage/datastorageerrorcategory.h"

using namespace hmservcommon::datastorage;

//-----------------------------------------------------------------------------
HMCachedMemoryDataStorage::~HMCachedMemoryDataStorage()
{
    close();
}
//-----------------------------------------------------------------------------
std::error_code HMCachedMemoryDataStorage::open()
{
    close();
    return HMAbstractCahceDataStorage::open(); // Вызываем открытие предка
}
//-----------------------------------------------------------------------------
bool HMCachedMemoryDataStorage::is_open() const
{
    return HMAbstractCahceDataStorage::is_open(); // Возвращаем открытость предка
}
//-----------------------------------------------------------------------------
void HMCachedMemoryDataStorage::close()
{    
    HMAbstractCahceDataStorage::close();
    clearCached(); // При закрытии чистим кеш
}
//-----------------------------------------------------------------------------
std::error_code HMCachedMemoryDataStorage::addUser(const std::shared_ptr<hmcommon::HMUser> inUser)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально помечаем как успех

    if (!inUser) // Проверяем указатель на валидность
        Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
    else
    {
        std::unique_lock ul(m_usersDefender); // Эксклюзивно блокируем доступ к пользователям
        if (!m_cachedUsers.emplace(HMCachedUser(inUser)).second) // Если пользователь не удалось закинуть в кеш
            Error = make_error_code(eDataStorageError::dsUserAlreadyExists);
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMCachedMemoryDataStorage::updateUser(const std::shared_ptr<hmcommon::HMUser> inUser)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально помечаем как успех

    if (!inUser) // Проверяем указатель на валидность
        Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
    else
    {
        // ЕСЛИ КОНЦЕПЦИЯ ОДНОГО ОБЪЕКТА РАБОТАЕТ ТО И ОБНОВЛЕНИЕ НА УРОВНЕ кешА УЖЕ ДОЛЖНО ПРОИЗОЙТИ
        // Достаточно проверить что этот объект уже в кеше и указатели равны
        std::shared_ptr<hmcommon::HMUser> FindRes = findUserByUUID(inUser->m_uuid, Error);

        if (FindRes != inUser)
            Error = make_error_code(hmcommon::eSystemErrorEx::seIncorretData);
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMUser> HMCachedMemoryDataStorage::findUserByUUID(const QUuid& inUserUUID, std::error_code& outErrorCode) const
{
    std::shared_ptr<hmcommon::HMUser> Result = nullptr;
    outErrorCode = make_error_code(eDataStorageError::dsSuccess); // Изначально помечаем как успех

    std::shared_lock sl(m_usersDefender); // Публично блокируем пользователей

    auto FindRes = m_cachedUsers.find(HMCachedUser(std::make_shared<hmcommon::HMUser>(inUserUUID))); // Ищим пользователя в кеше

    if (FindRes == m_cachedUsers.end()) // Нет пользователя в кеше
        outErrorCode = make_error_code(eDataStorageError::dsUserNotExists);
    else // Пользователь кеширован
    {
        Result = FindRes->m_user; // Вернём указатель на кешированного пользователя
        FindRes->m_lastRequest = QTime::currentTime(); // Помечаем время последнего запроса
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMUser> HMCachedMemoryDataStorage::findUserByAuthentication(const QString& inLogin, const QByteArray& inPasswordHash, std::error_code& outErrorCode) const
{
    std::shared_ptr<hmcommon::HMUser> Result = nullptr;
    outErrorCode = make_error_code(eDataStorageError::dsSuccess); // Изначально помечаем как успех

    std::shared_lock sl(m_usersDefender); // Публично блокируем пользователей

    // Ничего не поделаешь, перебираем
    auto FindRes = std::find_if(m_cachedUsers.cbegin(), m_cachedUsers.cend(), [&inLogin, &inPasswordHash](const HMCachedUser& CachedUser)
    {
        if (!CachedUser.m_user)
            return false;
        else
            return (CachedUser.m_user->getLogin() == inLogin) && (CachedUser.m_user->getPasswordHash() == inPasswordHash);
    });

    if (FindRes == m_cachedUsers.end()) // Нет пользователя в кеше
        outErrorCode = make_error_code(eDataStorageError::dsUserNotExists);
    else // Пользователь кеширован
    {
        Result = FindRes->m_user; // Вернём указатель на кешированного пользователя
        FindRes->m_lastRequest = QTime::currentTime(); // Помечаем время последнего запроса
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::error_code HMCachedMemoryDataStorage::removeUser(const QUuid& inUserUUID)
{
    std::unique_lock ul(m_usersDefender); // Эксклюзивно блокируем доступ к пользователям
    auto FindRes = m_cachedUsers.find(HMCachedUser(std::make_shared<hmcommon::HMUser>(inUserUUID))); // Ищим пользователя в кеше

    if (FindRes != m_cachedUsers.end()) // Если пользователь найден
        m_cachedUsers.erase(FindRes); // Удаляем его из кеша

    return make_error_code(eDataStorageError::dsSuccess); // Наплевать, был пользователь в кеше или нет
}
//-----------------------------------------------------------------------------
std::error_code HMCachedMemoryDataStorage::addGroup(const std::shared_ptr<hmcommon::HMGroup> inGroup)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально помечаем как успех

    if (!inGroup) // Проверяем указатель на валидность
        Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
    else
    {
        std::unique_lock ul(m_groupsDefender); // Эксклюзивно блокируем доступ к группам
        if (!m_cachedGroups.emplace(HMCachedGroup(inGroup)).second)
            Error = make_error_code(eDataStorageError::dsGroupAlreadyExists);
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMCachedMemoryDataStorage::updateGroup(const std::shared_ptr<hmcommon::HMGroup> inGroup)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально помечаем как успех

    if (!inGroup) // Проверяем указатель на валидность
        Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
    else
    {
        // ЕСЛИ КОНЦЕПЦИЯ ОДНОГО ОБЪЕКТА РАБОТАЕТ ТО И ОБНОВЛЕНИЕ НА УРОВНЕ кешА УЖЕ ДОЛЖНО ПРОИЗОЙТИ
        // Достаточно проверить что этот объект уже в кеше и указатели равны
        std::shared_ptr<hmcommon::HMGroup> FindRes = findGroupByUUID(inGroup->m_uuid, Error);

        if (FindRes != inGroup)
            Error = make_error_code(hmcommon::eSystemErrorEx::seIncorretData);
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMGroup> HMCachedMemoryDataStorage::findGroupByUUID(const QUuid& inGroupUUID, std::error_code& outErrorCode) const
{
    std::shared_ptr<hmcommon::HMGroup> Result = nullptr;
    outErrorCode = make_error_code(eDataStorageError::dsSuccess); // Изначально помечаем как успех

    std::shared_lock sl(m_groupsDefender); // Публично блокируем группы

    auto FindRes = m_cachedGroups.find(HMCachedGroup(std::make_shared<hmcommon::HMGroup>(inGroupUUID))); // Ищим группу в кеше

    if (FindRes == m_cachedGroups.end()) // Нет группы в кеше
        outErrorCode = make_error_code(eDataStorageError::dsGroupNotExists);
    else // Группа кеширована
    {
        Result = FindRes->m_group; // Вернём указатель на кешированную группу
        FindRes->m_lastRequest = QTime::currentTime(); // Помечаем время последнего запроса
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::error_code HMCachedMemoryDataStorage::removeGroup(const QUuid& inGroupUUID)
{
    std::unique_lock ul(m_groupsDefender); // Эксклюзивно блокируем доступ к группам
    auto FindRes = m_cachedGroups.find(HMCachedGroup(std::make_shared<hmcommon::HMGroup>(inGroupUUID))); // Ищим пользователя в кеше

    if (FindRes != m_cachedGroups.end()) // Если группа найдена
        m_cachedGroups.erase(FindRes); // Удаляем её из кеша

    return make_error_code(eDataStorageError::dsSuccess); // Наплевать, была группа в кеше или нет
}
//-----------------------------------------------------------------------------
std::error_code HMCachedMemoryDataStorage::addMessage(const std::shared_ptr<hmcommon::HMGroupMessage> inMessage)
{
    Q_UNUSED(inMessage);
    return make_error_code(hmcommon::eSystemErrorEx::seSuccess); // Много чести кешировать сообщения
}
//-----------------------------------------------------------------------------
std::error_code HMCachedMemoryDataStorage::updateMessage(const std::shared_ptr<hmcommon::HMGroupMessage> inMessage)
{
    Q_UNUSED(inMessage);
    return make_error_code(hmcommon::eSystemErrorEx::seSuccess); // Много чести кешировать сообщения
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMGroupMessage> HMCachedMemoryDataStorage::findMessage(const QUuid& inMessageUUID, std::error_code& outErrorCode) const
{
    Q_UNUSED(inMessageUUID);
    outErrorCode = make_error_code(eDataStorageError::dsMessageNotExists); // Чесно говорим, что сообщение не кешировано
    return nullptr;
}
//-----------------------------------------------------------------------------
std::vector<std::shared_ptr<hmcommon::HMGroupMessage>> HMCachedMemoryDataStorage::findMessages(const QUuid& inGroupUUID, const hmcommon::MsgRange& inRange,  std::error_code& outErrorCode) const
{
    Q_UNUSED(inGroupUUID);
    Q_UNUSED(inRange);
    outErrorCode = make_error_code(eDataStorageError::dsMessageNotExists); // Чесно говорим, что сообщения не кешированы
    return std::vector<std::shared_ptr<hmcommon::HMGroupMessage>>();
}
//-----------------------------------------------------------------------------
std::error_code HMCachedMemoryDataStorage::removeMessage(const QUuid& inMessageUUID, const QUuid& inGroupUUID)
{
    Q_UNUSED(inMessageUUID);
    Q_UNUSED(inGroupUUID);
    return make_error_code(eDataStorageError::dsSuccess); // Наплевать, было сообщение в кеше или нет
}
//-----------------------------------------------------------------------------
std::error_code HMCachedMemoryDataStorage::setUserContacts(const QUuid& inUserUUID, const std::shared_ptr<hmcommon::HMUserList> inContacts)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!inContacts) // Проверяем указатель на валидность
        Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
    else
    {
        std::unique_lock ul(m_userContactsDefender); // Эксклюзивно блокируем доступ к связам пользователь-контакты
        if (!m_cachedUserContacts.emplace(HMCachedUserContacts(inUserUUID, inContacts)).second)
            Error = make_error_code(eDataStorageError::dsRelationUCAlreadyExists);
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMCachedMemoryDataStorage::addUserContact(const QUuid& inUserUUID, const std::shared_ptr<hmcommon::HMUser> inContact)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!inContact) // Проверяем указатель на валидность
        Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
    else
    {
        if (inUserUUID == inContact->m_uuid) // Попытка добавить связь с самим собой
            Error = make_error_code(hmcommon::eSystemErrorEx::seIncorretData);
        else
        {
            std::shared_lock sl(m_userContactsDefender); // Публично блокируем связи пользователь-контакты
            auto FindRes = m_cachedUserContacts.find(HMCachedUserContacts(inUserUUID, std::make_shared<hmcommon::HMUserList>())); // Ищим связь в кеше

            if (FindRes == m_cachedUserContacts.end()) // Нет связи в кеше
                Error = make_error_code(eDataStorageError::dsRelationUCNotExists);
            else // Связь кеширована
            {
                Error = FindRes->m_contactList->add(inContact); // Добавляем контакт
                FindRes->m_lastRequest = QTime::currentTime(); // Помечаем время последнего запроса
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMCachedMemoryDataStorage::removeUserContact(const QUuid& inUserUUID, const QUuid& inContactUUID)
{
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    std::shared_lock sl(m_userContactsDefender); // Публично блокируем связи пользователь-контакты
    auto FindRes = m_cachedUserContacts.find(HMCachedUserContacts(inUserUUID, std::make_shared<hmcommon::HMUserList>())); // Ищим связь в кеше

    if (FindRes == m_cachedUserContacts.end()) // Нет связи в кеше
        Error = make_error_code(eDataStorageError::dsRelationUCNotExists);
    else // Связь кеширована
    {
        Error = FindRes->m_contactList->remove(inContactUUID); // Удаляем контакт
        FindRes->m_lastRequest = QTime::currentTime(); // Помечаем время последнего запроса
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMCachedMemoryDataStorage::removeUserContacts(const QUuid& inUserUUID)
{
    std::shared_lock sl(m_userContactsDefender); // Публично блокируем связи пользователь-контакты
    auto FindRes = m_cachedUserContacts.find(HMCachedUserContacts(inUserUUID, std::make_shared<hmcommon::HMUserList>())); // Ищим связь в кеше

    if (FindRes != m_cachedUserContacts.end()) // Если связь найдена
        m_cachedUserContacts.erase(FindRes); // Удаляем её из кеша

    return make_error_code(eDataStorageError::dsSuccess); // Наплевать, был пользователь в кеше или нет
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMUserList> HMCachedMemoryDataStorage::getUserContactList(const QUuid& inUserUUID, std::error_code& outErrorCode) const
{
    std::shared_ptr<hmcommon::HMUserList> Result = nullptr;
    outErrorCode = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    std::shared_lock sl(m_userContactsDefender); // Публично блокируем связи пользователь-контакты
    auto FindRes = m_cachedUserContacts.find(HMCachedUserContacts(inUserUUID, std::make_shared<hmcommon::HMUserList>())); // Ищим связь в кеше

    if (FindRes == m_cachedUserContacts.end()) // Нет связи в кеше
        outErrorCode = make_error_code(eDataStorageError::dsRelationUCNotExists);
    else // Связь кеширована
    {
        Result = FindRes->m_contactList; // Вернём указатель на кешированную связь
        FindRes->m_lastRequest = QTime::currentTime(); // Помечаем время последнего запроса
    }

    return Result;
}
//-----------------------------------------------------------------------------
void HMCachedMemoryDataStorage::clearCached()
{
    std::unique_lock ulu(m_usersDefender);
    std::unique_lock ulg(m_groupsDefender);
    std::unique_lock uluc(m_userContactsDefender);

    m_cachedGroups.clear();
    m_cachedUsers.clear();
    m_cachedUserContacts.clear();
}
//-----------------------------------------------------------------------------
void HMCachedMemoryDataStorage::processCacheInThread()
{
    static const std::int32_t cacheLifeTime = 15 * 60000; // Время жизни кешированого объекта 15 минут в милисекундах

    // СПЕРВА ОБРАБОТАТЬ СВЯЗИ

    // Обрабатываем кешированные связи пользорватель-контакты
    if (m_userContactsDefender.try_lock()) // Если прошла эксклюзивная блокировка
    {   // Просматриваем кеш связи пользорватель-контакты
        std::unique_lock ul(m_userContactsDefender, std::adopt_lock); // Передаём контроль в unique_lock

        auto It = m_cachedUserContacts.begin();
        // Пока не c++20 будем удалять по старинке
        while (It != m_cachedUserContacts.end())
        {   // Если объектом владеет только кеш и время жизни объекта вышло
            if (It->m_contactList.use_count() == 1 && It->m_lastRequest.msecsTo(QTime::currentTime()) >= cacheLifeTime)
                It = m_cachedUserContacts.erase(It); // Удаляем пользователя из кеша
            else // Объект не привысил лимит жизни
                It++; // Переходим к следующему объекту
        }
    }

    // ТЕПЕРЬ ОБРАБОТАТЬ СУЩЬНОСТИ

    // Обрабатываем кешированных пользователей
    if (m_usersDefender.try_lock()) // Если прошла эксклюзивная блокировка
    {   // Просматриваем кеш пользователей
        std::unique_lock ul(m_usersDefender, std::adopt_lock); // Передаём контроль в unique_lock

        auto It = m_cachedUsers.begin();
        // Пока не c++20 будем удалять по старинке
        while (It != m_cachedUsers.end())
        {   // Если объектом владеет только кеш и время жизни объекта вышло
            if (It->m_user.use_count() == 1 && It->m_lastRequest.msecsTo(QTime::currentTime()) >= cacheLifeTime)
                It = m_cachedUsers.erase(It); // Удаляем пользователя из кеша
            else // Объект не привысил лимит жизни
                It++; // Переходим к следующему объекту
        }

    }

    // Обрабатываем кешированные группы
    if (m_groupsDefender.try_lock()) // Если прошла эксклюзивная блокировка
    {   // Просматриваем кеш групп
        std::unique_lock ul(m_groupsDefender, std::adopt_lock); // Передаём контроль в unique_lock

        auto It = m_cachedGroups.begin();
        // Пока не c++20 будем удалять по старинке
        while (It != m_cachedGroups.end())
        {   // Если объектом владеет только кеш и время жизни объекта вышло
            if (It->m_group.use_count() == 1 && It->m_lastRequest.msecsTo(QTime::currentTime()) >= cacheLifeTime)
                It = m_cachedGroups.erase(It); // Удаляем группу из кеша
            else // Объект не привысил лимит жизни
                It++; // Переходим к следующему объекту
        }
    }
}
//-----------------------------------------------------------------------------
