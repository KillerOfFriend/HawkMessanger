#include <chrono>

#include <gtest/gtest.h>

#include <datastorage/DataStorage.h>
#include <HawkServerCoreHardDataStorageTest.hpp>

using namespace hmservcommon::datastorage;

//-----------------------------------------------------------------------------
const std::filesystem::path C_JSON_PATH = std::filesystem::current_path() / "DataStorage.json";                                                 ///< Путь к физическому хранилищу данных (JSON)
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
    errors::error_code Error; // Метка ошибки

    std::shared_ptr<HMAbstractHardDataStorage> HardStorage = nullptr;
    std::shared_ptr<HMAbstractCahceDataStorage> CacheStorage = nullptr;

    //---
    {   // Формируем физическое хранилище
        if (std::filesystem::exists(C_JSON_PATH)) // Если физическое хранилище существует
            std::filesystem::remove(C_JSON_PATH, Error); // Удаляем хранилище по указанному пути

         HardStorage = std::make_shared<HMJsonDataStorage>(C_JSON_PATH); // Формируем хранилище CombinedDataStorage
    }
    //---
    {   // Формируем кеширующее хранилище
        CacheStorage = std::make_shared<HMCachedMemoryDataStorage>(inCacheLifeTime, inSleep); // Формируем хранилище HMCachedMemoryDataStorage
    }
    //---

    return std::make_unique<HMCombinedDataStorage>(HardStorage, CacheStorage); // Создаём экземпляр комбинированного хранилища HMCombinedDataStorage
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит попытку открытия хранилища
 */
TEST(CombinedDataStorage, open)
{
    errors::error_code Error;
    std::unique_ptr<HMDataStorage> Storage = makeStorage(); // Создаём JSON хранилище с путём к файлу

    Error = Storage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    EXPECT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит добавление пользователя
 */
TEST(CombinedDataStorage, addUser)
{
    HardDataStorage_AddUserTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит обновление пользователя
 */
TEST(CombinedDataStorage, updateUser)
{
    HardDataStorage_UpdateUserTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск пользователья по UUID
 */
TEST(CombinedDataStorage, findUserByUUID)
{
    HardDataStorage_FindUserByUUIDTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск пользователья по данным аутентификации
 */
TEST(CombinedDataStorage, findUserByAuthentication)
{
    HardDataStorage_FindUserByAuthenticationTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление пользователя из хранилище
 */
TEST(CombinedDataStorage, removeUser)
{
    HardDataStorage_RemoveUserTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит создание связи пользователь-контакты (НЕ ДОЛЖНО ВЫПОЛНЯТЬСЯ ПОЛЬЗОВАТЕЛЕМ)
 */
TEST(CombinedDataStorage, setUserContacts)
{
    HardDataStorage_SetUserContactsTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит добавление контакта пользователю
 */
TEST(CombinedDataStorage, addUserContact)
{
    HardDataStorage_AddUserContactTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление контакта пользователя
 */
TEST(CombinedDataStorage, removeUserContact)
{
    HardDataStorage_RemoveUserContactTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление связи пользователь-контакты (НЕ ДОЛЖНО ВЫПОЛНЯТЬСЯ ПОЛЬЗОВАТЕЛЕМ)
 */
TEST(CombinedDataStorage, clearUserContacts)
{
    HardDataStorage_ClearUserContactsTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит получение списка контатков пользователя
 */
TEST(CombinedDataStorage, getUserContactList)
{
    HardDataStorage_GetUserContactListTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит получение списка UUID'ов групп пользователя
 */
TEST(CombinedDataStorage, getUserGroups)
{
    HardDataStorage_GetUserGroupsTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит добавление группы
 */
TEST(CombinedDataStorage, addGroup)
{
    HardDataStorage_AddGroupTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит обновление группы
 */
TEST(CombinedDataStorage, updateGroup)
{
    HardDataStorage_UpdateGroupTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск группы по UUID
 */
TEST(CombinedDataStorage, findGroupByUUID)
{
    HardDataStorage_FindGroupByUUIDTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление группы
 */
TEST(CombinedDataStorage, removeGroup)
{
    HardDataStorage_RemoveGroupTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит присвоение списка участников группе
 */
TEST(CombinedDataStorage, setGroupUsers)
{
    HardDataStorage_SetGroupUsersTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит добавление пользователя в группу
 */
TEST(CombinedDataStorage, addGroupUser)
{
    HardDataStorage_AddGroupUserTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление пользователя из группы
 */
TEST(CombinedDataStorage, removeGroupUser)
{
    HardDataStorage_RemoveGroupUserTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит очистку списка участников группы
 */
TEST(CombinedDataStorage, clearGroupUsers)
{
    HardDataStorage_ClearGroupUsersTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит получение списка UUID'ов участников группы
 */
TEST(CombinedDataStorage, getGroupUserList)
{
    HardDataStorage_GetGroupUserListTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит добовление сообщения
 */
TEST(CombinedDataStorage, addMessage)
{
    HardDataStorage_AddMessageTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит обновление сообщения
 */
TEST(CombinedDataStorage, updateMessage)
{
    HardDataStorage_UpdateMessageTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск сообщения по UUID
 */
TEST(CombinedDataStorage, findMessage)
{
    HardDataStorage_FindMessageTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск перечня сообщений по временному промежутку
 */
TEST(CombinedDataStorage, findMessages)
{
    HardDataStorage_FindMessagesTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление сообщения
 */
TEST(CombinedDataStorage, removeMessage)
{
    HardDataStorage_RemoveMessageTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит кеширование объектов
 */
TEST(CombinedDataStorage, CacheTest)
{
    errors::error_code Error;
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
        ASSERT_FALSE(Error); // Ошибки быть не должно
        ASSERT_NE(User, nullptr); // Должен вернуться валидный указатель

        Address2 = reinterpret_cast<std::uintptr_t>(User.get()); // Запоминаем адрес объекта
        EXPECT_NE(Address1, Address2); // В этот раз мы должны получить указатель на объект, сформированный из физического хранилища и адреса не должны совпасть
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
        ASSERT_FALSE(Error); // Ошибки быть не должно
        ASSERT_NE(Group, nullptr); // Должен вернуться валидный указатель

        Address2 = reinterpret_cast<std::uintptr_t>(Group.get()); // Запоминаем адрес объекта
        EXPECT_NE(Address1, Address2); // В этот раз мы должны получить указатель на объект, сформированный из физического хранилища и адреса не должны совпасть
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
        ASSERT_FALSE(Error); // Ошибки быть не должно
        ASSERT_NE(Contacts, nullptr); // Должен вернуться валидный указатель

        Address2 = reinterpret_cast<std::uintptr_t>(Contacts.get()); // Запоминаем адрес объекта
        EXPECT_NE(Address1, Address2); // В этот раз мы должны получить указатель на объект, сформированный из физического хранилища и адреса не должны совпасть
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
        ASSERT_FALSE(Error); // Ошибки быть не должно
        ASSERT_NE(Users, nullptr); // Должен вернуться валидный указатель

        Address2 = reinterpret_cast<std::uintptr_t>(Users.get()); // Запоминаем адрес объекта
        EXPECT_NE(Address1, Address2); // В этот раз мы должны получить указатель на объект, сформированный из физического хранилища и адреса не должны совпасть
    }

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief main - Входная точка тестировани функционала HMCombinedDataStorage
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
