#include <gtest/gtest.h>

#include <thread>
#include <filesystem>

#include <systemerrorex.h>
#include <datastorage/DataStorage.h>

#include <HawkCommonTestUtils.hpp>

using namespace hmservcommon::datastorage;

//-----------------------------------------------------------------------------
/**
 * @brief makeStorage - Метод создаст экземпляр хранилища
 * @return Вернёт экземпляр хранилища
 */
std::unique_ptr<HMDataStorage> makeStorage()
{
    std::chrono::milliseconds CacheLifeTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::minutes(10));
    std::chrono::milliseconds Sleep = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::minutes(1));

    return std::make_unique<HMCachedMemoryDataStorage>(CacheLifeTime, Sleep); // Создаём экземпляр кеширующего хранилища HMCachedMemoryDataStorage
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит попытку открытия хранилища
 */
TEST(CachedMemoryDataStorage, Open)
{
    std::error_code Error;
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
TEST(CachedMemoryDataStorage, AddUser)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> CachedStorage = makeStorage(); // Создаём кеширующее хранилище

    Error = CachedStorage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user();

    Error = CachedStorage->addUser(NewUser); // Пытаемся добавить пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = CachedStorage->addUser(NewUser); // Пытаемся добавить повторно
    EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserAlreadyExists)); // Должны получить сообщение что пользователь уже кеширован

    CachedStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит обновление пользователя
 */
TEST(CachedMemoryDataStorage, updateUser)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> CachedStorage = makeStorage(); // Создаём кеширующее хранилище

    Error = CachedStorage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user();

    Error = CachedStorage->addUser(NewUser); // Пытаемся добавить пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    NewUser->setName("New User Name");

    Error = CachedStorage->updateUser(NewUser); // Пытаемся обновить пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    CachedStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск пользователья по UUID
 */
TEST(CachedMemoryDataStorage, findUserByUUID)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> CachedStorage = makeStorage(); // Создаём кеширующее хранилище

    Error = CachedStorage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user();

    std::shared_ptr<hmcommon::HMUser> FindRes = CachedStorage->findUserByUUID(NewUser->m_uuid, Error); // Попытка получить не существующего пользователя

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // И метку, что пользователь не кеширован

    Error = CachedStorage->addUser(NewUser); // Пытаемся добавить пользователя в кеш
    ASSERT_FALSE(Error); // Ошибки быть не должно

    const size_t SilCount = 5; // Добавим некоторое количество левых пользователей
    for (size_t Index = 0; Index < SilCount; ++Index)
    {
        QString TrashUserLogin = "TrashUser" + QString::number(Index); // Логины мусорных пользователей должны быть уникальными
        Error = CachedStorage->addUser(testscommon::make_user(QUuid::createUuid(), TrashUserLogin)); // Пытаемся добавить пользователя
        ASSERT_FALSE(Error); // Ошибки быть не должно
    }

    FindRes = CachedStorage->findUserByUUID(NewUser->m_uuid, Error);

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_EQ(FindRes, NewUser); // Проводим сравнение указателей (КОНЦЕПЦИЯ кешА: Объект всегда хранится под одним указателем)

    CachedStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск пользователья по данным аутентификации
 */
TEST(CachedMemoryDataStorage, findUserByAuthentication)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> CachedStorage = makeStorage(); // Создаём кеширующее хранилище

    Error = CachedStorage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user();

    std::shared_ptr<hmcommon::HMUser> FindRes = CachedStorage->findUserByAuthentication(NewUser->getLogin(), NewUser->getPasswordHash(), Error); // Попытка получить не существующего пользователя

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // И метку, что пользователь не кеширован

    Error = CachedStorage->addUser(NewUser); // Пытаемся добавить пользователя в кеш
    ASSERT_FALSE(Error); // Ошибки быть не должно

    const size_t SilCount = 5; // Добавим некоторое количество левых пользователей
    for (size_t Index = 0; Index < SilCount; ++Index)
    {
        QString TrashUserLogin = "TrashUser" + QString::number(Index); // Логины мусорных пользователей должны быть уникальными
        Error = CachedStorage->addUser(testscommon::make_user(QUuid::createUuid(), TrashUserLogin)); // Пытаемся добавить пользователя
        ASSERT_FALSE(Error); // Ошибки быть не должно
    }

    FindRes = CachedStorage->findUserByAuthentication(NewUser->getLogin(), NewUser->getPasswordHash(), Error); // Попытка получить не существующего пользователя

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_EQ(FindRes, NewUser); // Проводим сравнение указателей (КОНЦЕПЦИЯ кешА: Объект всегда хранится под одним указателем)

    CachedStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление пользователя из кеша
 */
TEST(CachedMemoryDataStorage, removeUser)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> CachedStorage = makeStorage(); // Создаём кеширующее хранилище

    Error = CachedStorage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user();

    Error = CachedStorage->addUser(NewUser); // Пытаемся добавить пользователя в кеш
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = CachedStorage->removeUser(NewUser->m_uuid); // Пытаемся удалить добавленного пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<hmcommon::HMUser> FindRes = CachedStorage->findUserByUUID(NewUser->m_uuid, Error); // Попытка получить удалённого пользователя

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // И метку, что пользователь не кеширован

    CachedStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит получение списка UUID'ов групп пользователя
 */
TEST(CachedMemoryDataStorage, getUserGroups)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> CachedStorage = makeStorage(); // Создаём кеширующее хранилище

    Error = CachedStorage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = testscommon::make_group(); // Создаём группу (В кеш добавлять не обязательно)
    std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user(); // Создаём пользователя(В кеш добавлять не обязательно)

    // Кешируем полный список
    std::shared_ptr<std::set<QUuid>> UserUuids = std::make_shared<std::set<QUuid>>(); // Формируем пустой перечень участников группы
    UserUuids->insert(NewUser->m_uuid); // Добавляем пользователя в перечень

    Error = CachedStorage->setGroupUsers(NewGroup->m_uuid, UserUuids); // Задаём перечень участников группы
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<std::set<QUuid>> UserGroups = CachedStorage->getUserGroups(NewGroup->m_uuid, Error); // Запрашиваем список групп пользователя
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserGroupsRelationNotExists)); // СВЯЗИ ДЛЯ ПОЛЬЗОВАТЕЛЯ НЕ КЕШИРУЮТСЯ
    ASSERT_EQ(UserGroups, nullptr); // Должен вернуться nullptr

    CachedStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит добавление группы
 */
TEST(CachedMemoryDataStorage, addGroup)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> CachedStorage = makeStorage(); // Создаём кеширующее хранилище

    Error = CachedStorage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = testscommon::make_group();

    Error = CachedStorage->addGroup(NewGroup);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = CachedStorage->addGroup(NewGroup);
    EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupAlreadyExists)); // Должны получить сообщение что группа уже кеширована

    CachedStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит обновление группы
 */
TEST(CachedMemoryDataStorage, updateGroup)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> CachedStorage = makeStorage(); // Создаём кеширующее хранилище

    Error = CachedStorage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = testscommon::make_group();

    Error = CachedStorage->addGroup(NewGroup);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    NewGroup->setName("New Group Name");

    Error = CachedStorage->updateGroup(NewGroup);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    CachedStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск группы по UUID
 */
TEST(CachedMemoryDataStorage, findGroupByUUID)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> CachedStorage = makeStorage(); // Создаём кеширующее хранилище

    Error = CachedStorage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = testscommon::make_group();

    std::shared_ptr<hmcommon::HMGroup> FindRes = CachedStorage->findGroupByUUID(NewGroup->m_uuid, Error); // Попытка получить не существующую группу

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupNotExists)); // И метку, что группа не кеширована

    Error = CachedStorage->addGroup(NewGroup); // Пытаемся добавить группу в кеш
    ASSERT_FALSE(Error); // Ошибки быть не должно

    const size_t SilCount = 5; // Добавим некоторое количество левых групп
    for (size_t Index = 0; Index < SilCount; ++Index)
    {
        QString TrashGroupName = "TrashGroup" + QString::number(Index); // Логины мусорных пользователей должны быть уникальными
        Error = CachedStorage->addGroup(testscommon::make_group(QUuid::createUuid(), TrashGroupName)); // Пытаемся добавить группу
        ASSERT_FALSE(Error); // Ошибки быть не должно
    }

    FindRes = CachedStorage->findGroupByUUID(NewGroup->m_uuid, Error);

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_EQ(FindRes, NewGroup); // Проводим сравнение указателей (КОНЦЕПЦИЯ кешА: Объект всегда хранится под одним указателем)

    CachedStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление группы
 */
TEST(CachedMemoryDataStorage, removeGroup)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> CachedStorage = makeStorage(); // Создаём кеширующее хранилище

    Error = CachedStorage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = testscommon::make_group();

    Error = CachedStorage->addGroup(NewGroup); // Пытаемся добавить группу в кеш
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = CachedStorage->removeGroup(NewGroup->m_uuid); // Пытаемся удалить группу пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<hmcommon::HMGroup> FindRes = CachedStorage->findGroupByUUID(NewGroup->m_uuid, Error); // Попытка получить удалённую группу

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupNotExists)); // И метку, что группа не кеширована

    CachedStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит присвоение списка участников группе
 */
TEST(CachedMemoryDataStorage, setGroupUsers)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> CachedStorage = makeStorage(); // Создаём кеширующее хранилище

    Error = CachedStorage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage->is_open()); // Хранилище должно считаться открытым

    const size_t UsersCount = 5;
    std::shared_ptr<std::set<QUuid>> UserUUIDs = std::make_shared<std::set<QUuid>>(); // Инициализируем результат; // Контейнер UUID'ов пользователей

    for (size_t Index = 0; Index < UsersCount; ++Index)
    {
        QString TestUserLogin = "TestUser" + QString::number(Index); // Логины тестовых пользователей должны быть уникальными
        std::shared_ptr<hmcommon::HMUser> User = testscommon::make_user(QUuid::createUuid(), TestUserLogin); // Создаём пользователя (в кеш добавлять не обязательно)

        UserUUIDs->insert(User->m_uuid); // Запоминаем UUID добавленого в хранилище пользователя
    }

    std::shared_ptr<hmcommon::HMGroup> NewGroup = testscommon::make_group(); // Создаём группу (в кеш добавлять не обязательно)

    Error = CachedStorage->setGroupUsers(NewGroup->m_uuid, UserUUIDs); // Пытаемся добавить пользователей
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<std::set<QUuid>> GroupUsers = CachedStorage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем участников группы
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(GroupUsers, nullptr); // Должен вернуться валидный указатель
    EXPECT_EQ(*UserUUIDs, *GroupUsers); //

    CachedStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит добавление пользователя в группу
 */
TEST(CachedMemoryDataStorage, addGroupUser)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> CachedStorage = makeStorage(); // Создаём кеширующее хранилище

    Error = CachedStorage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = testscommon::make_group(); // Создаём группу (в кеш добавлять не обязательно)
    std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user(); // Создаём пользователя (в кеш добавлять не обязательно)

    Error = CachedStorage->addGroupUser(NewGroup->m_uuid, NewUser->m_uuid); // Добавляем участника в группу
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupUserRelationNotExists)); // Дложны получить сообщение, что нет связь не кеширована
    // Кешируем полный список
    std::shared_ptr<std::set<QUuid>> UserUuids = std::make_shared<std::set<QUuid>>(); // Формируем пустой перечень участников группы

    Error = CachedStorage->setGroupUsers(NewGroup->m_uuid, UserUuids);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = CachedStorage->addGroupUser(NewGroup->m_uuid, NewUser->m_uuid); // Добавляем участника в группу с уже кешированными данными
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<std::set<QUuid>> GroupUsers = CachedStorage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем перечень пользователей группы
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(GroupUsers, nullptr); // Должен вернуться валидный указаетль
    EXPECT_NE(GroupUsers->find(NewUser->m_uuid), GroupUsers->cend()); // Пользователь должен быть успешно найден в перечне участников

    CachedStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление пользователя из группы
 */
TEST(CachedMemoryDataStorage, removeGroupUser)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> CachedStorage = makeStorage(); // Создаём кеширующее хранилище

    Error = CachedStorage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = testscommon::make_group(); // Создаём группу (в кеш добавлять не обязательно)
    std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user(); // Создаём пользовтаеля (в кеш добавлять не обязательно)

    // Кешируем полный список
    std::shared_ptr<std::set<QUuid>> UserUuids = std::make_shared<std::set<QUuid>>(); // Формируем пустой перечень участников группы

    Error = CachedStorage->setGroupUsers(NewGroup->m_uuid, UserUuids); // Задаём перечень участников группы
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = CachedStorage->addGroupUser(NewGroup->m_uuid, NewUser->m_uuid); // Пытаемся добавить пользователя в группу
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<std::set<QUuid>> GroupUsers = CachedStorage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем перечень пользователей группы
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(GroupUsers, nullptr); // Должен вернуться валидный указаетль
    EXPECT_NE(GroupUsers->find(NewUser->m_uuid), GroupUsers->cend()); // Пользователь должен быть успешно найден в перечне участников

    Error = CachedStorage->removeGroupUser(NewGroup->m_uuid, NewUser->m_uuid); // Пытаемся удалить пользователя из группы
    ASSERT_FALSE(Error); // Ошибки быть не должно

    GroupUsers = CachedStorage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем перечень пользователей группы
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(GroupUsers, nullptr); // Должен вернуться валидный указаетль
    EXPECT_EQ(GroupUsers->find(NewUser->m_uuid), GroupUsers->cend()); // Пользователь не должен быть найден в перечне участников

    CachedStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит очистку списка участников группы
 */
TEST(CachedMemoryDataStorage, clearGroupUsers)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> CachedStorage = makeStorage(); // Создаём кеширующее хранилище

    Error = CachedStorage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = testscommon::make_group(); // Создаём группу (в кеш добавлять не обязательно)

    const std::size_t UsersCount = 5; // Контейнер, хранащий пользователей
    std::shared_ptr<std::set<QUuid>> UserUUIDs = std::make_shared<std::set<QUuid>>(); // Контейнер UUID'ов пользователей

    for (size_t Index = 0; Index < UsersCount; ++Index)
    {
        QString TestUserLogin = "TestUser" + QString::number(Index); // Логины тестовых пользователей должны быть уникальными
        std::shared_ptr<hmcommon::HMUser> User = testscommon::make_user(QUuid::createUuid(), TestUserLogin); // Создаём пользователя (в кеш добавлять не обязательно)

        UserUUIDs->insert(User->m_uuid); // Запоминаем UUID добавленого в хранилище пользователя
    }

    Error = CachedStorage->setGroupUsers(NewGroup->m_uuid, UserUUIDs); // Пытаемся добавить пользователей в группу
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<std::set<QUuid>> GroupUsers = CachedStorage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем список участников группы
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(UserUUIDs, nullptr); // Должен вернуться валидный указаетль
    EXPECT_EQ(*UserUUIDs, *GroupUsers); // Перечни UUID'ов должны совпасть

    Error = CachedStorage->clearGroupUsers(NewGroup->m_uuid); // Очищаем перечень участников группы
    ASSERT_FALSE(Error); // Ошибки быть не должно

    GroupUsers = CachedStorage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем список участников группы
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(GroupUsers, nullptr); // Должен вернуться валидный указаетль
    EXPECT_TRUE(GroupUsers->empty()); // Перечень участников группы должен быть пуст

    CachedStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит получение списка UUID'ов участников группы
 */
TEST(CachedMemoryDataStorage, getGroupUserList)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> CachedStorage = makeStorage(); // Создаём кеширующее хранилище

    Error = CachedStorage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = testscommon::make_group(); // Создаём группу

    std::shared_ptr<std::set<QUuid>> GroupUsers = CachedStorage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем список участников группы
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupUserRelationNotExists)); // Получаем метку, связь не кеширована
    ASSERT_EQ(GroupUsers, nullptr); // Должен вернуться валидный указаетль

    const std::size_t UsersCount = 5; // Контейнер, хранащий пользователей
    std::shared_ptr<std::set<QUuid>> UserUUIDs = std::make_shared<std::set<QUuid>>(); // Контейнер UUID'ов пользователей

    for (size_t Index = 0; Index < UsersCount; ++Index)
    {
        QString TestUserLogin = "TestUser" + QString::number(Index); // Логины тестовых пользователей должны быть уникальными
        std::shared_ptr<hmcommon::HMUser> User = testscommon::make_user(QUuid::createUuid(), TestUserLogin); // Создаём пользователя (в кеш добавлять не обязательно)
        UserUUIDs->insert(User->m_uuid); // Запоминаем UUID добавленого в хранилище пользователя
    }

    Error = CachedStorage->setGroupUsers(NewGroup->m_uuid, UserUUIDs); // Пытаемся добавить пользователей в группу
    ASSERT_FALSE(Error); // Ошибки быть не должно

    GroupUsers = CachedStorage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем список участников группы
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(UserUUIDs, nullptr); // Должен вернуться валидный указаетль
    EXPECT_EQ(*UserUUIDs, *GroupUsers); // Перечни UUID'ов должны совпасть

    CachedStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит добовление сообщения
 */
TEST(CachedMemoryDataStorage, addMessage)
{
    ASSERT_TRUE(true); // Кеширование сообщений не поддерживается
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит обновление сообщения
 */
TEST(CachedMemoryDataStorage, updateMessage)
{
    ASSERT_TRUE(true); // Кеширование сообщений не поддерживается
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск сообщения по UUID
 */
TEST(CachedMemoryDataStorage, findMessage)
{
    ASSERT_TRUE(true); // Кеширование сообщений не поддерживается
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск перечня сообщений по временному промежутку
 */
TEST(CachedMemoryDataStorage, findMessages)
{
    ASSERT_TRUE(true); // Кеширование сообщений не поддерживается
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление сообщения
 */
TEST(CachedMemoryDataStorage, removeMessage)
{
    ASSERT_TRUE(true); // Кеширование сообщений не поддерживается
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит создание связи пользователь-контакты (НЕ ДОЛЖНО ВЫПОЛНЯТЬСЯ ПОЛЬЗОВАТЕЛЕМ)
 */
TEST(CachedMemoryDataStorage, setUserContacts)
{
    std::error_code Error;

    std::unique_ptr<HMDataStorage> CachedStorage = makeStorage(); // Создаём кеширующее хранилище

    Error = CachedStorage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user(QUuid::createUuid(), "TestUser@login.com");
    std::shared_ptr<hmcommon::HMUser> NewContact1 = testscommon::make_user(QUuid::createUuid(), "TestContact1@login.com");
    std::shared_ptr<hmcommon::HMUser> NewContact2 = testscommon::make_user(QUuid::createUuid(), "TestContact2@login.com");

    std::shared_ptr<std::set<QUuid>> NewContactList = std::make_shared<std::set<QUuid>>();

    NewContactList->insert(NewContact1->m_uuid);
    NewContactList->insert(NewContact2->m_uuid);

    Error = CachedStorage->setUserContacts(NewUser->m_uuid, NewContactList); // Пытаемся добавить список контактов без добавления пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно (Наличие пользователя в кеше не обязательно)

    Error = CachedStorage->setUserContacts(NewUser->m_uuid, NewContactList); // Повторно добавляем список
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserContactAlredyExists)); // Должны получить сообщение о том, что контакт уже есть в списке пользователя

    CachedStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит добавление контакта пользователю
 */
TEST(CachedMemoryDataStorage, addUserContact)
{
    std::error_code Error;

    std::unique_ptr<HMDataStorage> CachedStorage = makeStorage(); // Создаём кеширующее хранилище

    Error = CachedStorage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user(QUuid::createUuid(), "TestUser@login.com");
    std::shared_ptr<hmcommon::HMUser> NewContact = testscommon::make_user(QUuid::createUuid(), "TestContact@login.com");

    Error = CachedStorage->addUserContact(NewUser->m_uuid, NewContact->m_uuid);
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserContactRelationNotExists)); // Должны получить сообщение о том, что связь не существует

    std::shared_ptr<std::set<QUuid>> NewContactList = std::make_shared<std::set<QUuid>>();

    Error = CachedStorage->setUserContacts(NewUser->m_uuid, NewContactList); // Добавляем пустой список контактов
    ASSERT_FALSE(Error); // Ошибки быть не должно (Наличие пользователя в кеше не обязательно)
    Error = CachedStorage->addUserContact(NewUser->m_uuid, NewContact->m_uuid); // Добавляем контакт
    ASSERT_FALSE(Error); // Ошибки быть не должно (Наличие пользователя в кеше не обязательно)

    Error = CachedStorage->addUserContact(NewUser->m_uuid, NewContact->m_uuid); // Повторно добавляем контакт
    ASSERT_EQ(Error.value(), static_cast<int32_t>(hmcommon::eSystemErrorEx::seAlredyInContainer)); // Должны получить сообщение о том, что контакт уже в кеше

    CachedStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление контакта пользователя
 */
TEST(CachedMemoryDataStorage, removeUserContact)
{
    std::error_code Error;

    std::unique_ptr<HMDataStorage> CachedStorage = makeStorage(); // Создаём кеширующее хранилище

    Error = CachedStorage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user(QUuid::createUuid(), "TestUser@login.com");
    std::shared_ptr<hmcommon::HMUser> NewContact = testscommon::make_user(QUuid::createUuid(), "TestContact@login.com");

    Error = CachedStorage->removeUserContact(NewUser->m_uuid, NewContact->m_uuid); // Пытаемся удалсть не существующий контакт
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserContactRelationNotExists)); // Должны получить сообщение о том, что связи не существует

    std::shared_ptr<std::set<QUuid>> NewContactList = std::make_shared<std::set<QUuid>>();

    Error = CachedStorage->setUserContacts(NewUser->m_uuid, NewContactList); // Добавляем пустой список контактов
    ASSERT_FALSE(Error); // Ошибки быть не должно (Наличие пользователя в кеше не обязательно)

    Error = CachedStorage->addUserContact(NewUser->m_uuid, NewContact->m_uuid); // И добавляем контакт пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = CachedStorage->removeUserContact(NewUser->m_uuid, NewContact->m_uuid); // И наконец штатно удаляем связь
    ASSERT_FALSE(Error); // Ошибки быть не должно

    CachedStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление связи пользователь-контакты (НЕ ДОЛЖНО ВЫПОЛНЯТЬСЯ ПОЛЬЗОВАТЕЛЕМ)
 */
TEST(CachedMemoryDataStorage, clearUserContacts)
{
    std::error_code Error;

    std::unique_ptr<HMDataStorage> CachedStorage = makeStorage(); // Создаём кеширующее хранилище

    Error = CachedStorage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user(QUuid::createUuid(), "TestUser@login.com");
    std::shared_ptr<hmcommon::HMUser> NewContact1 = testscommon::make_user(QUuid::createUuid(), "TestContact1@login.com");
    std::shared_ptr<hmcommon::HMUser> NewContact2 = testscommon::make_user(QUuid::createUuid(), "TestContact2@login.com");

    Error = CachedStorage->clearUserContacts(NewUser->m_uuid); // Пытаемся удалить не сущестующую связь
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsSuccess)); // Вне зависимости от наличия в кеше, удаление валидно

    std::shared_ptr<std::set<QUuid>> NewContactList = std::make_shared<std::set<QUuid>>();

    Error = CachedStorage->setUserContacts(NewUser->m_uuid, NewContactList); // Добавляем пустой список контактов
    ASSERT_FALSE(Error); // Ошибки быть не должно (Наличие пользователя в кеше не обязательно)

    Error = CachedStorage->addUserContact(NewUser->m_uuid, NewUser->m_uuid); // Пытаемся добавить в список контактов самого себя
    ASSERT_EQ(Error.value(), static_cast<int32_t>(hmcommon::eSystemErrorEx::seIncorretData)); // Должны получить сообщение о том, что данные не корректны

    Error = CachedStorage->addUserContact(NewUser->m_uuid, NewContact1->m_uuid); // И добавляем контакт1 пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = CachedStorage->addUserContact(NewUser->m_uuid, NewContact2->m_uuid); // И добавляем контакт2 пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = CachedStorage->clearUserContacts(NewUser->m_uuid); // Удаляем связь
    ASSERT_FALSE(Error); // Ошибки быть не должно

    CachedStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит получение списка контатков пользователя
 */
TEST(CachedMemoryDataStorage, getUserContactList)
{
    std::error_code Error;

    std::unique_ptr<HMDataStorage> CachedStorage = makeStorage(); // Создаём кеширующее хранилище

    Error = CachedStorage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user(QUuid::createUuid(), "TestUser@login.com");
    std::shared_ptr<hmcommon::HMUser> NewContact1 = testscommon::make_user(QUuid::createUuid(), "TestContact1@login.com");
    std::shared_ptr<hmcommon::HMUser> NewContact2 = testscommon::make_user(QUuid::createUuid(), "TestContact2@login.com");

    std::shared_ptr<std::set<QUuid>> FindRes = CachedStorage->getUserContactList(NewUser->m_uuid, Error); // Пытаемся получить список контактов не существующего пользователя

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserContactRelationNotExists)); // Должны получить сообщение о том, что связь не существует

    std::shared_ptr<std::set<QUuid>> NewContactList = std::make_shared<std::set<QUuid>>();

    Error = CachedStorage->setUserContacts(NewUser->m_uuid, NewContactList); // Добавляем пустой список контактов
    ASSERT_FALSE(Error); // Ошибки быть не должно (Наличие пользователя в кеше не обязательно)

    Error = CachedStorage->addUserContact(NewUser->m_uuid, NewContact1->m_uuid); // И добавляем контакт1 пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = CachedStorage->addUserContact(NewUser->m_uuid, NewContact2->m_uuid); // И добавляем контакт2 пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно

    FindRes = CachedStorage->getUserContactList(NewUser->m_uuid, Error); // Теперь пытваемся получить список контактов пользователя

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_NE(FindRes->find(NewContact1->m_uuid), FindRes->end()); // Должен содержать первый контакт
    EXPECT_NE(FindRes->find(NewContact2->m_uuid), FindRes->end()); // Должен содержать второй контакт

    CachedStorage->close();
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
