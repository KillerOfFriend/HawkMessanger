#include <memory>
#include <chrono>
#include <filesystem>

#include <gtest/gtest.h>

#include <HawkServerCoreCachedDataStorageTest.hpp>
#include <datastorage/cachedmemorydatastorage/cachedmemorydatastorage.h>

//-----------------------------------------------------------------------------
const std::chrono::milliseconds C_CACHE_LIFE_TIME_FAST = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::milliseconds(100)); ///< Сокращённое время существования объекта в кеше
const std::chrono::milliseconds C_CACHE_SLEEP_FAST = std::chrono::milliseconds(50);                                                             ///< Сокращённое время ожидания потока контроля кеша
const std::chrono::milliseconds C_CACHE_LIFE_END_WAIT = C_CACHE_LIFE_TIME_FAST + C_CACHE_SLEEP_FAST * 2;                                        ///< Время, гарантирующее уничтожение объекта в кеше
//-----------------------------------------------------------------------------
/**
 * @brief makeStorage - Метод создаст экземпляр хранилища
 * @param inCacheLifeTime - Время жизни объектов кеша (в милисекундах)
 * @param inSleep - Время ожидания потока контроля кеша в (в милисекундах)
 * @return Вернёт экземпляр хранилища
 */
std::unique_ptr<HMDataStorage> makeStorage(const std::chrono::milliseconds inCacheLifeTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::minutes(15)),
                                           const std::chrono::milliseconds inSleep = std::chrono::milliseconds(1000))
{
    return std::make_unique<HMCachedMemoryDataStorage>(inCacheLifeTime, inSleep); // Создаём экземпляр кеширующего хранилища HMCachedMemoryDataStorage
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит попытку открытия хранилища
 */
TEST(CachedMemoryDataStorage, open)
{
    hmcommon::error_code Error;
    std::unique_ptr<HMDataStorage> CachedStorage = makeStorage(); // Создаём кеширующее хранилище

    Error = CachedStorage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    EXPECT_TRUE(CachedStorage->is_open()); // Хранилище должно считаться открытым

    CachedStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит добавление пользователя
 */
TEST(CachedMemoryDataStorage, addUser)
{
    CachedDataStorage_AddUserTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит обновление пользователя
 */
TEST(CachedMemoryDataStorage, updateUser)
{
    CachedDataStorage_UpdateUserTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск пользователья по UUID
 */
TEST(CachedMemoryDataStorage, findUserByUUID)
{
    CachedDataStorage_FindUserByUUIDTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск пользователья по данным аутентификации
 */
TEST(CachedMemoryDataStorage, findUserByAuthentication)
{
    CachedDataStorage_FindUserByAuthenticationTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление пользователя из кеша
 */
TEST(CachedMemoryDataStorage, removeUser)
{
    CachedDataStorage_RemoveUserTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------

/**
 * @brief TEST - Тест проверит создание связи пользователь-контакты (НЕ ДОЛЖНО ВЫПОЛНЯТЬСЯ ПОЛЬЗОВАТЕЛЕМ)
 */
TEST(CachedMemoryDataStorage, setUserContacts)
{
    CachedDataStorage_SetUserContactsTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит добавление контакта пользователю
 */
TEST(CachedMemoryDataStorage, addUserContact)
{
    CachedDataStorage_AddUserContactTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление контакта пользователя
 */
TEST(CachedMemoryDataStorage, removeUserContact)
{
    CachedDataStorage_RemoveUserContactTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление связи пользователь-контакты (НЕ ДОЛЖНО ВЫПОЛНЯТЬСЯ ПОЛЬЗОВАТЕЛЕМ)
 */
TEST(CachedMemoryDataStorage, clearUserContacts)
{
    CachedDataStorage_ClearUserContactsTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит получение списка контатков пользователя
 */
TEST(CachedMemoryDataStorage, getUserContactList)
{
    CachedDataStorage_GetUserContactListTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит получение списка UUID'ов групп пользователя
 */
TEST(CachedMemoryDataStorage, getUserGroups)
{
    CachedDataStorage_GetUserGroupsTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит добавление группы
 */
TEST(CachedMemoryDataStorage, addGroup)
{
    CachedDataStorage_AddGroupTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит обновление группы
 */
TEST(CachedMemoryDataStorage, updateGroup)
{
    CachedDataStorage_UpdateGroupTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск группы по UUID
 */
TEST(CachedMemoryDataStorage, findGroupByUUID)
{
    CachedDataStorage_FindGroupByUUIDTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление группы
 */
TEST(CachedMemoryDataStorage, removeGroup)
{
    CachedDataStorage_RemoveGroupTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит присвоение списка участников группе
 */
TEST(CachedMemoryDataStorage, setGroupUsers)
{
    CachedDataStorage_SetGroupUsersTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит добавление пользователя в группу
 */
TEST(CachedMemoryDataStorage, addGroupUser)
{
    CachedDataStorage_AddGroupUserTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление пользователя из группы
 */
TEST(CachedMemoryDataStorage, removeGroupUser)
{
    CachedDataStorage_RemoveGroupUserTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит очистку списка участников группы
 */
TEST(CachedMemoryDataStorage, clearGroupUsers)
{
    CachedDataStorage_ClearGroupUsersTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит получение списка UUID'ов участников группы
 */
TEST(CachedMemoryDataStorage, getGroupUserList)
{
    CachedDataStorage_GetGroupUserListTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит добовление сообщения
 */
TEST(CachedMemoryDataStorage, addMessage)
{
    CachedDataStorage_AddMessageTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит обновление сообщения
 */
TEST(CachedMemoryDataStorage, updateMessage)
{
    CachedDataStorage_UpdateMessageTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск сообщения по UUID
 */
TEST(CachedMemoryDataStorage, findMessage)
{
    CachedDataStorage_FindMessageTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск перечня сообщений по временному промежутку
 */
TEST(CachedMemoryDataStorage, findMessages)
{
    CachedDataStorage_FindMessagesTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление сообщения
 */
TEST(CachedMemoryDataStorage, removeMessage)
{
    CachedDataStorage_RemoveMessageTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит кеширование объектов
 */
TEST(CombinedDataStorage, CacheTest)
{
    hmcommon::error_code Error;
    std::unique_ptr<HMDataStorage> Storage = makeStorage(C_CACHE_LIFE_TIME_FAST, C_CACHE_SLEEP_FAST); // Создаём кеширующее хранилище (С короткой жизнью объектов)

    Error = Storage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым


    //-----
    // Проверим кеширование пользователя
    //-----
    {
        QUuid UserUUID = QUuid::createUuid();
        std::shared_ptr<hmcommon::HMUserInfo> User = testscommon::make_user_info(UserUUID, "CachedUser@login.com"); // Формируем нового пользователя

        Error = Storage->addUser(User); // Добавляем пользователя в хранилище
        ASSERT_FALSE(Error); // Ошибки быть не должно

        std::uintptr_t Address1 = reinterpret_cast<std::uintptr_t>(User.get()); // Запоминаем адрес объекта
        User = nullptr; // Обязательно сбрасываем указатель на добавленного пользователя

        User = Storage->findUserByUUID(UserUUID, Error); // Изщим пользователя по UUID
        ASSERT_FALSE(Error); // Ошибки быть не должно
        ASSERT_NE(User, nullptr); // Должен вернуться валидный указатель

        std::uintptr_t Address2 = reinterpret_cast<std::uintptr_t>(User.get()); // Запоминаем адрес объекта
        User = nullptr; // Обязательно сбрасываем указатель на найденного пользователя

        EXPECT_EQ(Address1, Address2); // Если объект возвращён из кеша, то адреса должны совапасть
        std::this_thread::sleep_for(C_CACHE_LIFE_END_WAIT); // Ожидаем время, гарантирующее уничтожение объекта в кеше

        User = Storage->findUserByUUID(UserUUID, Error); // Ищим пользователя по UUID
        EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // Должны получить сообщение, что пользователь в кеше не существует
    }

    //-----
    // Проверим кеширование групп
    //-----
    {
        QUuid GroupUUID = QUuid::createUuid();
        std::shared_ptr<hmcommon::HMGroupInfo> Group = testscommon::make_group_info(GroupUUID, "CachedGroup");

        Error = Storage->addGroup(Group); // Добавляем группу в хранилище
        ASSERT_FALSE(Error); // Ошибки быть не должно

        std::uintptr_t Address1 = reinterpret_cast<std::uintptr_t>(Group.get()); // Запоминаем адрес объекта
        Group = nullptr; // Обязательно сбрасываем указатель на добавленную группу

        Group = Storage->findGroupByUUID(GroupUUID, Error);
        ASSERT_FALSE(Error); // Ошибки быть не должно
        ASSERT_NE(Group, nullptr); // Должен вернуться валидный указатель

        std::uintptr_t Address2 = reinterpret_cast<std::uintptr_t>(Group.get()); // Запоминаем адрес объекта
        Group = nullptr; // Обязательно сбрасываем указатель на найденную группу

        EXPECT_EQ(Address1, Address2); // Если объект возвращён из кеша, то адреса должны совапасть
        std::this_thread::sleep_for(C_CACHE_LIFE_END_WAIT); // Ожидаем время, гарантирующее уничтожение объекта в кеше

        Group = Storage->findGroupByUUID(GroupUUID, Error); // Ищим пользователя по UUID
        EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupNotExists)); // Должны получить сообщение, что группы в кеше не существует
    }

    //-----
    // Проверим кеширование сообщений
    //-----
    {
        // Сообщения не кешируются
    }

    //-----
    // Проверим кеширование контактов пользователей
    //-----
    {
        QUuid UserUUID = QUuid::createUuid();
        std::shared_ptr<hmcommon::HMUserInfo> User = testscommon::make_user_info(UserUUID, "UserWithContacts@login.com"); // Формируем нового пользователя

        Error = Storage->addUser(User); // Добавляем пользователя в хранилище
        ASSERT_FALSE(Error); // Ошибки быть не должно

        const std::size_t ContactsCount = 5;
        std::shared_ptr<std::set<QUuid>> Contacts = std::make_shared<std::set<QUuid>>(); // Формируем список контактов

        for (std::size_t Index = 0; Index < ContactsCount; ++Index)
        {
            QString UserLogin = "Contact@login.com" + QString::number(Index); // Логин должен быть униакальным
            std::shared_ptr<hmcommon::HMUserInfo> NewUser = testscommon::make_user_info(QUuid::createUuid(), UserLogin); // Формируем нового пользователя

            Error = Storage->addUser(NewUser);
            ASSERT_FALSE(Error); // Ошибки быть не должно
            Contacts->insert(NewUser->m_uuid); // Запоминаем UUID добавленного пользователя
        }

        Error = Storage->setUserContacts(UserUUID, Contacts); // Задаём пользователю список контактов
        ASSERT_FALSE(Error); // Ошибки быть не должно

        std::uintptr_t Address1 = reinterpret_cast<std::uintptr_t>(Contacts.get()); // Запоминаем адрес объекта
        Contacts = nullptr; // Обязательно сбрасываем указатель на добавленный список пользователей

        Contacts = Storage->getUserContactList(UserUUID, Error); // Запрашиваем список контактов пользователя
        ASSERT_FALSE(Error); // Ошибки быть не должно
        ASSERT_NE(Contacts, nullptr); // Должен вернуться валидный указатель

        std::uintptr_t Address2 = reinterpret_cast<std::uintptr_t>(Contacts.get()); // Запоминаем адрес объекта
        Contacts = nullptr; // Обязательно сбрасываем указатель на полученный список пользователей

        EXPECT_EQ(Address1, Address2); // Если объект возвращён из кеша, то адреса должны совапасть
        std::this_thread::sleep_for(C_CACHE_LIFE_END_WAIT); // Ожидаем время, гарантирующее уничтожение объекта в кеше

        Contacts = Storage->getUserContactList(UserUUID, Error); // Запрашиваем список контактов пользователя
        EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserContactRelationNotExists)); // Должны получить сообщение, что список контактов не кеширован
    }

    //-----
    // Проверим кеширование участников групп
    //-----
    {
        QUuid GroupUUID = QUuid::createUuid();
        std::shared_ptr<hmcommon::HMGroupInfo> Group = testscommon::make_group_info(GroupUUID, "GroupWithUsers");

        Error = Storage->addGroup(Group); // Добавляем группу в хранилище
        ASSERT_FALSE(Error); // Ошибки быть не должно

        const std::size_t UserssCount = 5;
        std::shared_ptr<std::set<QUuid>> Users = std::make_shared<std::set<QUuid>>(); // Формируем список контактов

        for (std::size_t Index = 0; Index < UserssCount; ++Index)
        {
            QString UserLogin = "GroupUser@login.com" + QString::number(Index); // Логин должен быть униакальным
            std::shared_ptr<hmcommon::HMUserInfo> NewUser = testscommon::make_user_info(QUuid::createUuid(), UserLogin); // Формируем нового пользователя

            Error = Storage->addUser(NewUser);
            ASSERT_FALSE(Error); // Ошибки быть не должно
            Users->insert(NewUser->m_uuid); // Запоминаем UUID добавленного пользователя
        }

        Error = Storage->setGroupUsers(GroupUUID, Users); // Задаём группе список пользователей
        ASSERT_FALSE(Error); // Ошибки быть не должно

        std::uintptr_t Address1 = reinterpret_cast<std::uintptr_t>(Users.get()); // Запоминаем адрес объекта
        Users = nullptr; // Обязательно сбрасываем указатель на добавленный список пользователей

        Users = Storage->getGroupUserList(GroupUUID, Error); // Запрашиваем список участников группы
        ASSERT_FALSE(Error); // Ошибки быть не должно
        ASSERT_NE(Users, nullptr); // Должен вернуться валидный указатель

        std::uintptr_t Address2 = reinterpret_cast<std::uintptr_t>(Users.get()); // Запоминаем адрес объекта
        Users = nullptr; // Обязательно сбрасываем указатель на полученный список пользователей

        EXPECT_EQ(Address1, Address2); // Если объект возвращён из кеша, то адреса должны совапасть
        std::this_thread::sleep_for(C_CACHE_LIFE_END_WAIT); // Ожидаем время, гарантирующее уничтожение объекта в кеше

        Users = Storage->getGroupUserList(GroupUUID, Error); // Запрашиваем список участников группы
        EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupUserRelationNotExists)); // Должны получить сообщение, что список участников группы не кеширован
    }

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief main - Входная точка тестировани функционала HMCachedMemoryDataStorage
 * @param argc - Количество аргументов
 * @param argv - Перечень аргументов
 * @return Вернёт признак успешности тестирования
 */
int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
//-----------------------------------------------------------------------------
