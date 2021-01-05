#include <gtest/gtest.h>

#include <thread>
#include <filesystem>

#include <systemerrorex.h>
#include <datastorage/DataStorage.h>

#include <HawkCommonTestUtils.hpp>

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
    std::error_code Error; // Метка ошибки

    std::shared_ptr<HMAbstractHardDataStorage> HardStorage = nullptr;
    std::shared_ptr<HMAbstractCahceDataStorage> inCacheStorage = nullptr;

    //---
    {   // Формируем физическое хранилище
        if (std::filesystem::exists(C_JSON_PATH)) // Если физическое хранилище существует
            std::filesystem::remove(C_JSON_PATH, Error); // Удаляем хранилище по указанному пути

         HardStorage = std::make_shared<HMJsonDataStorage>(C_JSON_PATH); // Формируем хранилище JsonDataStorage
    }
    //---
    {   // ФОрмируем кеширующее хранилище
        inCacheStorage = std::make_shared<HMCachedMemoryDataStorage>(inCacheLifeTime, inSleep); // Формируем хранилище HMCachedMemoryDataStorage
    }
    //---

    return std::make_unique<HMCombinedDataStorage>(HardStorage, inCacheStorage); // Создаём экземпляр комбинированного хранилища HMCombinedDataStorage
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит попытку открытия хранилища
 */
TEST(CombinedDataStorage, Open)
{
    std::error_code Error;
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
TEST(CombinedDataStorage, AddUser)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> Storage = makeStorage(); // Создаём JSON хранилище

    Error = Storage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user();

    Error = Storage->addUser(NewUser); // Пытаемся добавить пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->addUser(NewUser); // Пытаемся добавить повторно
    EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserAlreadyExists)); // Должны получить сообщение о том, что пользователь с таким UUID уже зарегистрирован

    NewUser = testscommon::make_user(QUuid::createUuid()); // Формируем такого же пользователя но с другим UUID

    Error = Storage->addUser(NewUser); // Пытаемся добавить с новым UUID
    EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserLoginAlreadyRegistered)); // Должны получить сообщение о том, что этот логин уже занят

    NewUser = testscommon::make_user(QUuid::createUuid(), "OtherUser@login.com"); // Формируем с другим UUID и логином

    Error = Storage->addUser(NewUser); // Пытаемся добавить с другим UUID и логином
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит обновление пользователя
 */
TEST(CombinedDataStorage, updateUser)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> Storage = makeStorage(); // Создаём JSON хранилище

    Error = Storage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user();

    Error = Storage->addUser(NewUser); // Пытаемся добавить пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    NewUser->setName("New User Name");

    Error = Storage->updateUser(NewUser); // Пытаемся обновить пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<hmcommon::HMUser> FindRes = Storage->findUserByUUID(NewUser->m_uuid, Error);
    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно
    EXPECT_EQ(NewUser->getName(), FindRes->getName()); // Имя должно измениться

    NewUser = testscommon::make_user(); // Формируем нового пользователя

    Error = Storage->updateUser(NewUser); // Пытаемся обновить пользователя, не добавленного в хранилище
    EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // Должны получить сообщение о том, что нет такого пользователя

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск пользователья по UUID
 */
TEST(CombinedDataStorage, findUserByUUID)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> Storage = makeStorage(); // Создаём JSON хранилище

    Error = Storage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user();

    std::shared_ptr<hmcommon::HMUser> FindRes = Storage->findUserByUUID(NewUser->m_uuid, Error); // Попытка получить не существующего пользователя

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // И метку, что пользователь не найден в хранилище

    Error = Storage->addUser(NewUser); // Пытаемся добавить пользователя в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    const size_t SilCount = 5; // Добавим некоторое количество левых пользователей
    for (size_t Index = 0; Index < SilCount; ++Index)
    {
        QString TrashUserLogin = "TrashUser" + QString::number(Index); // Логины мусорных пользователей должны быть уникальными
        Error = Storage->addUser(testscommon::make_user(QUuid::createUuid(), TrashUserLogin)); // Пытаемся добавить пользователя
        ASSERT_FALSE(Error); // Ошибки быть не должно
    }

    FindRes = Storage->findUserByUUID(NewUser->m_uuid, Error);

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_EQ(*NewUser, *FindRes); // Полное сравнение объектов должно пройти успешно

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск пользователья по данным аутентификации
 */
TEST(CombinedDataStorage, findUserByAuthentication)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> Storage = makeStorage(); // Создаём JSON хранилище

    Error = Storage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user();

    std::shared_ptr<hmcommon::HMUser> FindRes = Storage->findUserByAuthentication(NewUser->getLogin(), NewUser->getPasswordHash(), Error); // Попытка получить не существующего пользователя

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // И метку, что пользователь не добавлен в хранилище

    Error = Storage->addUser(NewUser); // Пытаемся добавить пользователя в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    const size_t SilCount = 5; // Добавим некоторое количество левых пользователей
    for (size_t Index = 0; Index < SilCount; ++Index)
    {
        QString TrashUserLogin = "TrashUser" + QString::number(Index); // Логины мусорных пользователей должны быть уникальными
        Error = Storage->addUser(testscommon::make_user(QUuid::createUuid(), TrashUserLogin)); // Пытаемся добавить пользователя
        ASSERT_FALSE(Error); // Ошибки быть не должно
    }

    FindRes = Storage->findUserByAuthentication(NewUser->getLogin(), NewUser->getPasswordHash(), Error); // Попытка получить не существующего пользователя

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_EQ(*NewUser, *FindRes); // Полное сравнение объектов должно пройти успешно

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление пользователя из хранилище
 */
TEST(CombinedDataStorage, removeUser)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> Storage = makeStorage(); // Создаём JSON хранилище

    Error = Storage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user();

    Error = Storage->addUser(NewUser); // Пытаемся добавить пользователя в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->removeUser(NewUser->m_uuid); // Пытаемся удалить добавленного пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<hmcommon::HMUser> FindRes = Storage->findUserByUUID(NewUser->m_uuid, Error); // Попытка получить удалённого пользователя

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // И метку, что пользователь не найден в хранилище

    Error = Storage->removeUser(NewUser->m_uuid); // Пытаемся удалить не существующего пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит получение списка UUID'ов групп пользователя
 */
TEST(CombinedDataStorage, getUserGroups)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> Storage = makeStorage(); // Создаём JSON хранилище

    Error = Storage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = testscommon::make_group(); // Создаём группу
    Error = Storage->addGroup(NewGroup); // Добавляем группу в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    const std::size_t UsersCount = 5; // Контейнер, хранащий пользователей
    std::shared_ptr<std::set<QUuid>> UserUUIDs = std::make_shared<std::set<QUuid>>(); // Контейнер UUID'ов пользователей
    std::shared_ptr<std::set<QUuid>> UserGroups = std::make_shared<std::set<QUuid>>(); // Перечень групп пользователя

    for (size_t Index = 0; Index < UsersCount; ++Index)
    {
        QString TestUserLogin = "TestUser" + QString::number(Index); // Логины тестовых пользователей должны быть уникальными
        std::shared_ptr<hmcommon::HMUser> User = testscommon::make_user(QUuid::createUuid(), TestUserLogin);

        UserGroups = Storage->getUserGroups(User->m_uuid, Error); // Запрашиваем перечень групп не существующего в хранилище пользователя
        ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // Получаем метку, что пользователь не найден в хранилище

        Error = Storage->addUser(User); // Пытаемся добавить пользователя
        ASSERT_FALSE(Error); // Ошибки быть не должно

        UserGroups = Storage->getUserGroups(User->m_uuid, Error); // Запрашиваем перечень групп добавленного в хранилище пользователя
        ASSERT_FALSE(Error); // Ошибки быть не должно
        ASSERT_NE(UserGroups, nullptr); // Должен вернуться валидный указаетль
        EXPECT_TRUE(UserGroups->empty()); // Перечень групп должен быть пустым

        UserUUIDs->insert(User->m_uuid); // Запоминаем UUID добавленого в хранилище пользователя
    }

    Error = Storage->setGroupUsers(NewGroup->m_uuid, UserUUIDs); // Пытаемся добавить пользователей в группу
    ASSERT_FALSE(Error); // Ошибки быть не должно

    for (const QUuid& UserUUID : *UserUUIDs) // Перебираем всех добавленых в хранилище пользователей
    {
        UserGroups = Storage->getUserGroups(UserUUID, Error); // Запрашиваем перечень групп пользователей
        ASSERT_FALSE(Error); // Ошибки быть не должно
        ASSERT_NE(UserGroups, nullptr); // Должен вернуться валидный указаетль
        EXPECT_NE(UserGroups->find(NewGroup->m_uuid), UserGroups->cend()); // Группа должна быть успешно найдена в списке групп пользователей
    }

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит добавление группы
 */
TEST(CombinedDataStorage, addGroup)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> Storage = makeStorage(); // Создаём JSON хранилище

    Error = Storage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = testscommon::make_group();

    Error = Storage->addGroup(NewGroup);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->addGroup(NewGroup);
    EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupUUIDAlreadyRegistered)); // Должны получить сообщение о том, что этот UUID уже зарегистрирован

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит обновление группы
 */
TEST(CombinedDataStorage, updateGroup)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> Storage = makeStorage(); // Создаём JSON хранилище

    Error = Storage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = testscommon::make_group();

    Error = Storage->addGroup(NewGroup);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    NewGroup->setName("New Group Name");

    Error = Storage->updateGroup(NewGroup);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<hmcommon::HMGroup> FindRes = Storage->findGroupByUUID(NewGroup->m_uuid, Error);
    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно
    EXPECT_EQ(NewGroup->getName(), FindRes->getName()); // Имя должно измениться

    NewGroup = testscommon::make_group(); // Формируем новую группу

    Error = Storage->updateGroup(NewGroup); // Пытаемся обновить группу, не добавленную в хранилище
    EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupNotExists)); // Должны получить сообщение о том, что нет такой группы

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск группы по UUID
 */
TEST(CombinedDataStorage, findGroupByUUID)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> Storage = makeStorage(); // Создаём JSON хранилище

    Error = Storage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = testscommon::make_group();

    std::shared_ptr<hmcommon::HMGroup> FindRes = Storage->findGroupByUUID(NewGroup->m_uuid, Error); // Попытка получить не существующую группу

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupNotExists)); // И метку, что группа не найдена в хранилище

    Error = Storage->addGroup(NewGroup); // Пытаемся добавить группу в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    const size_t SilCount = 5; // Добавим некоторое количество левых групп
    for (size_t Index = 0; Index < SilCount; ++Index)
    {
        QString TrashGroupName = "TrashGroup" + QString::number(Index); // Логины мусорных пользователей должны быть уникальными
        Error = Storage->addGroup(testscommon::make_group(QUuid::createUuid(), TrashGroupName)); // Пытаемся добавить группу
        ASSERT_FALSE(Error); // Ошибки быть не должно
    }

    FindRes = Storage->findGroupByUUID(NewGroup->m_uuid, Error);

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_EQ(*NewGroup, *FindRes); // Полное сравнение объектов должно пройти успешно

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление группы
 */
TEST(CombinedDataStorage, removeGroup)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> Storage = makeStorage(); // Создаём JSON хранилище

    Error = Storage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = testscommon::make_group();

    Error = Storage->addGroup(NewGroup); // Пытаемся добавить группу в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->removeGroup(NewGroup->m_uuid); // Пытаемся удалить группу пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<hmcommon::HMGroup> FindRes = Storage->findGroupByUUID(NewGroup->m_uuid, Error); // Попытка получить удалённую группу

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupNotExists)); // И метку, что группа не найдена в хранилище

    Error = Storage->removeGroup(NewGroup->m_uuid); // Пытаемся удалить не существующую группу
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит присвоение списка участников группе
 */
TEST(CombinedDataStorage, setGroupUsers)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> Storage = makeStorage(); // Создаём JSON хранилище

    Error = Storage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    const size_t UsersCount = 5;
    std::shared_ptr<std::set<QUuid>> UserUUIDs = std::make_shared<std::set<QUuid>>(); // Инициализируем результат; // Контейнер UUID'ов пользователей

    for (size_t Index = 0; Index < UsersCount; ++Index)
    {
        QString TestUserLogin = "TestUser" + QString::number(Index); // Логины тестовых пользователей должны быть уникальными
        std::shared_ptr<hmcommon::HMUser> User = testscommon::make_user(QUuid::createUuid(), TestUserLogin);

        Error = Storage->addUser(User); // Пытаемся добавить пользователя
        ASSERT_FALSE(Error); // Ошибки быть не должно
        UserUUIDs->insert(User->m_uuid); // Запоминаем UUID добавленого в хранилище пользователя
    }

    std::shared_ptr<hmcommon::HMGroup> NewGroup = testscommon::make_group(); // Создаём группу

    Error = Storage->setGroupUsers(NewGroup->m_uuid, UserUUIDs); // Пытаемся добавить пользователей в не существующую в хранилище группу
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupNotExists)); // Получаем метку, что группа не найдена в хранилище

    Error = Storage->addGroup(NewGroup); // Добавляем группу в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->setGroupUsers(NewGroup->m_uuid, UserUUIDs); // Пытаемся добавить пользователей в группу
    ASSERT_FALSE(Error); // Ошибки быть не должно
    // Теперь проверяем что связи созданы успешно
    std::shared_ptr<std::set<QUuid>> GroupUsers = Storage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем список участников группы
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(GroupUsers, nullptr); // Должен вернуться валидный указаетль
    EXPECT_EQ(*UserUUIDs, *GroupUsers); // Перечни UUID'ов должны совпасть

    for (const QUuid& UserUUID : *UserUUIDs) // Перебираем всех добавленых в хранилище пользователей
    {
        std::shared_ptr<std::set<QUuid>> UserGroups = Storage->getUserGroups(UserUUID, Error); // Запрашиваем перечень групп пользователей
        ASSERT_FALSE(Error); // Ошибки быть не должно
        ASSERT_NE(UserGroups, nullptr); // Должен вернуться валидный указаетль
        EXPECT_NE(UserGroups->find(NewGroup->m_uuid), UserGroups->cend()); // Группа должна быть успешно найдена в списке групп пользователей
    }

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит добавление пользователя в группу
 */
TEST(CombinedDataStorage, addGroupUser)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> Storage = makeStorage(); // Создаём JSON хранилище

    Error = Storage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = testscommon::make_group(); // Создаём группу
    std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user(); // Создаём пользовтаеля

    Error = Storage->addGroupUser(NewGroup->m_uuid, NewUser->m_uuid); // Пытаемся добавить пользователя в не существующую в хранилище группу
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupNotExists)); // Получаем метку, что группа не найдена в хранилище

    Error = Storage->addGroup(NewGroup); // Добавляем группу в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->addGroupUser(NewGroup->m_uuid, NewUser->m_uuid); // Пытаемся добавить не существующего в хранилище пользователя в группу
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // Получаем метку, что пользователь не найдена в хранилище

    Error = Storage->addUser(NewUser); // Добавляем пользователя в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->addGroupUser(NewGroup->m_uuid, NewUser->m_uuid); // Пытаемся добавить пользователя в группу
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<std::set<QUuid>> GroupUsers = Storage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем список групп пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(GroupUsers, nullptr); // Должен вернуться валидный указаетль
    EXPECT_NE(GroupUsers->find(NewUser->m_uuid), GroupUsers->cend()); // Пользователь должен быть успешно найден среди участников группы

    std::shared_ptr<std::set<QUuid>> UserGroups = Storage->getUserGroups(NewUser->m_uuid, Error); // Запрашиваем список пользователей группы
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(UserGroups, nullptr); // Должен вернуться валидный указаетль
    EXPECT_NE(UserGroups->find(NewGroup->m_uuid), UserGroups->cend()); // Группа должна быть успешно найдена в списке групп пользователей

    Error = Storage->addGroupUser(NewGroup->m_uuid, NewUser->m_uuid); // Пытаемся повторно добавить пользователя в группу
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupUserRelationAlredyExists)); // Получаем метку, что группа и пользователь уже связаны

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление пользователя из группы
 */
TEST(CombinedDataStorage, removeGroupUser)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> Storage = makeStorage(); // Создаём JSON хранилище

    Error = Storage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = testscommon::make_group(); // Создаём группу
    Error = Storage->addGroup(NewGroup); // Добавляем группу в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user(); // Создаём пользовтаеля
    Error = Storage->addUser(NewUser); // Добавляем пользователя в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->removeGroupUser(NewGroup->m_uuid, NewUser->m_uuid); // Пытаемся удалить из группы, не существующего в ней пользователя
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupUserRelationNotExists)); // Получаем метку, что группа и пользователь не связаны

    Error = Storage->addGroupUser(NewGroup->m_uuid, NewUser->m_uuid); // Пытаемся добавить пользователя в группу
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<std::set<QUuid>> GroupUsers = Storage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем список групп пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(GroupUsers, nullptr); // Должен вернуться валидный указаетль
    EXPECT_NE(GroupUsers->find(NewUser->m_uuid), GroupUsers->cend()); // Пользователь должен быть успешно найден среди участников группы

    std::shared_ptr<std::set<QUuid>> UserGroups = Storage->getUserGroups(NewUser->m_uuid, Error); // Запрашиваем список пользователей группы
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(UserGroups, nullptr); // Должен вернуться валидный указаетль
    EXPECT_NE(UserGroups->find(NewGroup->m_uuid), UserGroups->cend()); // Группа должна быть успешно найдена в списке групп пользователей

    Error = Storage->removeGroupUser(NewGroup->m_uuid, NewUser->m_uuid); // Пытаемся удалить пользователя из группы
    ASSERT_FALSE(Error); // Ошибки быть не должно

    GroupUsers = Storage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем список групп пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(GroupUsers, nullptr); // Должен вернуться валидный указаетль
    EXPECT_EQ(GroupUsers->find(NewUser->m_uuid), GroupUsers->cend()); // Пользователь не должен быть найден среди участников группы

    UserGroups = Storage->getUserGroups(NewUser->m_uuid, Error); // Запрашиваем список пользователей группы
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(UserGroups, nullptr); // Должен вернуться валидный указаетль
    EXPECT_EQ(UserGroups->find(NewGroup->m_uuid), UserGroups->cend()); // Группа не должна быть найдена в списке групп пользователей

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит очистку списка участников группы
 */
TEST(CombinedDataStorage, clearGroupUsers)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> Storage = makeStorage(); // Создаём JSON хранилище

    Error = Storage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = testscommon::make_group(); // Создаём группу

    Error = Storage->clearGroupUsers(NewGroup->m_uuid); // Пытаемся очистить перечень участников не существующей группы
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupNotExists)); // Получаем метку, что группа не существует

    Error = Storage->addGroup(NewGroup); // Добавляем группу в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    const std::size_t UsersCount = 5; // Контейнер, хранащий пользователей
    std::shared_ptr<std::set<QUuid>> UserUUIDs = std::make_shared<std::set<QUuid>>(); // Контейнер UUID'ов пользователей

    for (size_t Index = 0; Index < UsersCount; ++Index)
    {
        QString TestUserLogin = "TestUser" + QString::number(Index); // Логины тестовых пользователей должны быть уникальными
        std::shared_ptr<hmcommon::HMUser> User = testscommon::make_user(QUuid::createUuid(), TestUserLogin);

        Error = Storage->addUser(User); // Пытаемся добавить пользователя
        ASSERT_FALSE(Error); // Ошибки быть не должно
        UserUUIDs->insert(User->m_uuid); // Запоминаем UUID добавленого в хранилище пользователя
    }

    Error = Storage->setGroupUsers(NewGroup->m_uuid, UserUUIDs); // Пытаемся добавить пользователей в группу
    ASSERT_FALSE(Error); // Ошибки быть не должно
    // Теперь проверяем что связи созданы успешно
    std::shared_ptr<std::set<QUuid>> GroupUsers = Storage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем список участников группы
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(UserUUIDs, nullptr); // Должен вернуться валидный указаетль
    EXPECT_EQ(*UserUUIDs, *GroupUsers); // Перечни UUID'ов должны совпасть

    for (const QUuid& UserUUID : *UserUUIDs) // Перебираем всех добавленых в хранилище пользователей
    {
        std::shared_ptr<std::set<QUuid>> UserGroups = Storage->getUserGroups(UserUUID, Error); // Запрашиваем перечень групп пользователей
        ASSERT_FALSE(Error); // Ошибки быть не должно
        ASSERT_NE(UserGroups, nullptr); // Должен вернуться валидный указаетль
        EXPECT_NE(UserGroups->find(NewGroup->m_uuid), UserGroups->cend()); // Группа должна быть успешно найдена в списке групп пользователей
    }

    Error = Storage->clearGroupUsers(NewGroup->m_uuid); // Очищаем перечень участников группы
    ASSERT_FALSE(Error); // Ошибки быть не должно
    // Теперь проверяем что связи разорваны
    GroupUsers = Storage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем список участников группы
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(GroupUsers, nullptr); // Должен вернуться валидный указаетль
    EXPECT_TRUE(GroupUsers->empty()); // Перечень участников группы должен быть пуст

    for (const QUuid& UserUUID : *UserUUIDs) // Перебираем всех добавленых в хранилище пользователей
    {
        std::shared_ptr<std::set<QUuid>> UserGroups = Storage->getUserGroups(UserUUID, Error); // Запрашиваем перечень групп пользователей
        ASSERT_FALSE(Error); // Ошибки быть не должно
        ASSERT_NE(UserGroups, nullptr); // Должен вернуться валидный указаетль
        EXPECT_EQ(UserGroups->find(NewGroup->m_uuid), UserGroups->cend()); // Группа не должна быть найдена в списке групп пользователей
    }

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит получение списка UUID'ов участников группы
 */
TEST(CombinedDataStorage, getGroupUserList)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> Storage = makeStorage(); // Создаём JSON хранилище

    Error = Storage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = testscommon::make_group(); // Создаём группу

    std::shared_ptr<std::set<QUuid>> GroupUsers = Storage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем список участников не существующей в хранилище группы
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupNotExists)); // Получаем метку, что группы не существует

    Error = Storage->addGroup(NewGroup); // Добавляем группу в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    GroupUsers = Storage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем список участников только что созданной группы
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(GroupUsers, nullptr); // Должен вернуться валидный указаетль
    EXPECT_TRUE(GroupUsers->empty()); // Перечень участников должен быть пуст

    const std::size_t UsersCount = 5; // Контейнер, хранащий пользователей
    std::shared_ptr<std::set<QUuid>> UserUUIDs = std::make_shared<std::set<QUuid>>(); // Контейнер UUID'ов пользователей

    for (size_t Index = 0; Index < UsersCount; ++Index)
    {
        QString TestUserLogin = "TestUser" + QString::number(Index); // Логины тестовых пользователей должны быть уникальными
        std::shared_ptr<hmcommon::HMUser> User = testscommon::make_user(QUuid::createUuid(), TestUserLogin);

        Error = Storage->addUser(User); // Пытаемся добавить пользователя
        ASSERT_FALSE(Error); // Ошибки быть не должно
        UserUUIDs->insert(User->m_uuid); // Запоминаем UUID добавленого в хранилище пользователя
    }

    Error = Storage->setGroupUsers(NewGroup->m_uuid, UserUUIDs); // Пытаемся добавить пользователей в группу
    ASSERT_FALSE(Error); // Ошибки быть не должно

    GroupUsers = Storage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем список участников группы
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(UserUUIDs, nullptr); // Должен вернуться валидный указаетль
    EXPECT_EQ(*UserUUIDs, *GroupUsers); // Перечни UUID'ов должны совпасть

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит добовление сообщения
 */
TEST(CombinedDataStorage, addMessage)
{
    std::error_code Error; // Метка ошибки
    std::unique_ptr<HMDataStorage> Storage = makeStorage(); // Создаём JSON хранилище

    Error = Storage->open(); // Пытаемся открыть хранилище

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    // Сообщениее может быть добавлено только в группу
    std::shared_ptr<hmcommon::HMGroup> NewGroup = testscommon::make_group();

    hmcommon::MsgData TextData(hmcommon::eMsgType::mtText, "Текст сообщения"); // Формируем данные сообщения
    std::shared_ptr<hmcommon::HMGroupMessage> NewMessage = testscommon::make_groupmessage(TextData, QUuid::createUuid(), NewGroup->m_uuid); // Формируем сообщение

    Error = Storage->addMessage(NewMessage); // Пытаемся добавить сообщение без группы
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupNotExists)); // И метку, что группа не найдена в хранилище

    Error = Storage->addGroup(NewGroup); // Теперь добавим группу
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->addMessage(NewMessage); // Пытаемся добавить сообщение
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->addMessage(NewMessage); // Пытаемся добавить сообщение повторно
    EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsMessageAlreadyExists)); // Должны получить сообщение о том, что этот UUID уже зарегистрирован

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит обновление сообщения
 */
TEST(CombinedDataStorage, updateMessage)
{
    std::error_code Error; // Метка ошибки
    std::unique_ptr<HMDataStorage> Storage = makeStorage(); // Создаём JSON хранилище

    Error = Storage->open(); // Пытаемся открыть хранилище

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    // Сообщениее может быть добавлено только в группу
    std::shared_ptr<hmcommon::HMGroup> NewGroup = testscommon::make_group();

    Error = Storage->addGroup(NewGroup);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    hmcommon::MsgData TextData(hmcommon::eMsgType::mtText, "Текст сообщения"); // Формируем данные сообщения
    std::shared_ptr<hmcommon::HMGroupMessage> NewMessage = testscommon::make_groupmessage(TextData, QUuid::createUuid(), NewGroup->m_uuid); // Формируем сообщение

    Error = Storage->updateMessage(NewMessage); // Пытаемся обновить сообщение не добавляя
    EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsMessageNotExists)); // Должны получить сообщение о том, что не удалось найти сообщение для обновления

    Error = Storage->addMessage(NewMessage); // Пытаемся добавить сообщение
    ASSERT_FALSE(Error); // Ошибки быть не должно

    TextData.m_data = QString("Новый текст сообщения").toLocal8Bit(); // Меняем текст
    NewMessage->setMessage(TextData);

    Error = Storage->updateMessage(NewMessage); // Пытаемся обновить сообщение
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск сообщения по UUID
 */
TEST(CombinedDataStorage, findMessage)
{
    std::error_code Error; // Метка ошибки
    std::unique_ptr<HMDataStorage> Storage = makeStorage(); // Создаём JSON хранилище

    Error = Storage->open(); // Пытаемся открыть хранилище

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    // Сообщениее может быть добавлено только в группу
    std::shared_ptr<hmcommon::HMGroup> NewGroup = testscommon::make_group();

    hmcommon::MsgData TextData(hmcommon::eMsgType::mtText, "Текст сообщения"); // Формируем данные сообщения
    std::shared_ptr<hmcommon::HMGroupMessage> NewMessage = testscommon::make_groupmessage(TextData, QUuid::createUuid(), NewGroup->m_uuid); // Формируем сообщение

    std::shared_ptr<hmcommon::HMGroupMessage> FindRes = Storage->findMessage(NewMessage->m_uuid, Error); // Пытаемся найти не добавленное сообщение

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsMessageNotExists)); // Должны получить сообщение о том, что сообщение не найдено

    Error = Storage->addGroup(NewGroup); // Добавим группу
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->addMessage(NewMessage); // Пытаемся добавить сообщение
    ASSERT_FALSE(Error); // Ошибки быть не должно

    FindRes = Storage->findMessage(NewMessage->m_uuid, Error); // Пытаемся найти не добавленное сообщение

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск перечня сообщений по временному промежутку
 */
TEST(CombinedDataStorage, findMessages)
{
    std::error_code Error; // Метка ошибки
    std::unique_ptr<HMDataStorage> Storage = makeStorage(); // Создаём JSON хранилище

    Error = Storage->open(); // Пытаемся открыть хранилище

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = testscommon::make_group();

    Error = Storage->addGroup(NewGroup); // добавляем группу сообщения
    ASSERT_FALSE(Error); // Ошибки быть не должно

    const size_t MESSAGES = 5;
    std::array<std::shared_ptr<hmcommon::HMGroupMessage>, MESSAGES> Messages;
    std::array<QDateTime, MESSAGES> Times;

    for (std::size_t Index = 0; Index < MESSAGES; ++Index)
    {
        //QDateTime(QDate(), QTime::currentTime()); // Запоминаем время
        Times[Index] = QDateTime::currentDateTime();
        hmcommon::MsgData TextData(hmcommon::eMsgType::mtText, ("Текст сообщения " + QString::number(Index)).toLocal8Bit()); // Формируем данные сообщения
        Messages[Index] = testscommon::make_groupmessage(TextData, QUuid::createUuid(), NewGroup->m_uuid, Times[Index]); // Формируем сообщение группы

        Error = Storage->addMessage(Messages[Index]);
        ASSERT_FALSE(Error); // Ошибки быть не должно
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    hmcommon::MsgRange TimeRange(Times[0], Times[MESSAGES - 2]); // Выбираем временной интервал со временни первого до времени предпоследнего сообщения
    std::vector<std::shared_ptr<hmcommon::HMGroupMessage>> FindRes = Storage->findMessages(NewGroup->m_uuid, TimeRange, Error); // Получаем резульата

    ASSERT_FALSE(Error); // Ошибки быть не должно
    EXPECT_EQ(FindRes.size(), MESSAGES - 1); // Результатов должно быть на 1 меньше чем созданых сообщений
    // Сверяем результаты с исхдными сообщениями (Последовательность должна совпасть!)
    for (std::size_t Index = 0; Index < std::min(FindRes.size(), MESSAGES); ++Index)
    {
        EXPECT_EQ(Messages[Index]->m_uuid, FindRes[Index]->m_uuid);
        EXPECT_EQ(Messages[Index]->m_group, FindRes[Index]->m_group);
        EXPECT_EQ(Messages[Index]->m_createTime, FindRes[Index]->m_createTime);

        auto Data1 = Messages[Index]->getMesssage();
        auto Data2 = FindRes[Index]->getMesssage();

        EXPECT_EQ(Data1.m_type, Data2.m_type);
        EXPECT_EQ(Data1.m_data, Data2.m_data);
    }

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление сообщения
 */
TEST(CombinedDataStorage, removeMessage)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> Storage = makeStorage(); // Создаём JSON хранилище

    Error = Storage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    // Сообщениее может быть добавлено только в группу
    std::shared_ptr<hmcommon::HMGroup> NewGroup = testscommon::make_group();

    hmcommon::MsgData TextData(hmcommon::eMsgType::mtText, "Текст сообщения"); // Формируем данные сообщения
    std::shared_ptr<hmcommon::HMGroupMessage> NewMessage = testscommon::make_groupmessage(TextData, QUuid::createUuid(), NewGroup->m_uuid); // Формируем сообщение

    Error = Storage->addGroup(NewGroup); // Добавим группу
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->removeGroup(NewGroup->m_uuid); // Пытаемся удалить добавленное сообщение
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<hmcommon::HMGroupMessage> FindRes = Storage->findMessage(NewGroup->m_uuid, Error); // Попытка получить удалённое сообщение

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsMessageNotExists)); // И метку, что сообщение не найдено в хранилище

    Error = Storage->removeGroup(NewGroup->m_uuid); // Пытаемся удалить не существующее сообщение
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит создание связи пользователь-контакты (НЕ ДОЛЖНО ВЫПОЛНЯТЬСЯ ПОЛЬЗОВАТЕЛЕМ)
 */
TEST(CombinedDataStorage, setUserContacts)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> Storage = makeStorage(); // Создаём JSON хранилище

    Error = Storage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user(QUuid::createUuid(), "TestUser@login.com");
    std::shared_ptr<hmcommon::HMUser> NewContact1 = testscommon::make_user(QUuid::createUuid(), "TestContact1@login.com");
    std::shared_ptr<hmcommon::HMUser> NewContact2 = testscommon::make_user(QUuid::createUuid(), "TestContact2@login.com");

    std::shared_ptr<std::set<QUuid>> NewContactList = std::make_shared<std::set<QUuid>>();

    NewContactList->insert(NewContact1->m_uuid);
    NewContactList->insert(NewContact2->m_uuid);

    Error = Storage->setUserContacts(NewUser->m_uuid, NewContactList); // Пытаемся добавить список контактов без добавления пользователя
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // Должны получить сообщение о том, что не существует пользователя, к которому нужно привязать список

    Error = Storage->addUser(NewUser); // Теперь добавим пользователя в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит добавление контакта пользователю
 */
TEST(CombinedDataStorage, addUserContact)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> Storage = makeStorage(); // Создаём JSON хранилище

    Error = Storage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user(QUuid::createUuid(), "TestUser@login.com");
    std::shared_ptr<hmcommon::HMUser> NewContact = testscommon::make_user(QUuid::createUuid(), "TestContact@login.com");

    Error = Storage->addUserContact(NewUser->m_uuid, NewContact->m_uuid); // Пытаемся добавить контакт без добавления пользователя
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // Должны получить сообщение о том, что пользователь (вледелец) не существует в хранилище

    Error = Storage->addUser(NewUser); // Теперь добавим пользователя в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->addUserContact(NewUser->m_uuid, NewContact->m_uuid); // И теперь добавляем контакт
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // Должны получить сообщение о том, что пользователь (контакт) не существует в хранилище

    Error = Storage->addUser(NewContact); // Теперь добавим пользователя, который будет выступать новым контактом
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->addUserContact(NewUser->m_uuid, NewContact->m_uuid); // И накнец штатно добавляем контакт пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление контакта пользователя
 */
TEST(CombinedDataStorage, removeUserContact)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> Storage = makeStorage(); // Создаём JSON хранилище

    Error = Storage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user(QUuid::createUuid(), "TestUser@login.com");
    std::shared_ptr<hmcommon::HMUser> NewContact = testscommon::make_user(QUuid::createUuid(), "TestContact@login.com");

    Error = Storage->removeUserContact(NewUser->m_uuid, NewContact->m_uuid); // Пытаемся удалить контакт без добавления пользователя
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // Должны получить сообщение о том, что пользователь (вледелец) не существует в хранилище

    Error = Storage->addUser(NewUser); // Теперь добавим пользователя в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->removeUserContact(NewUser->m_uuid, NewContact->m_uuid); // И теперь добавляем контакт
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserContactNotExists)); // Должны получить сообщение о том, что у пользователя нет такого контакта

    Error = Storage->addUser(NewContact); // Теперь добавим пользователя, который будет выступать новым контактом
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->addUserContact(NewUser->m_uuid, NewContact->m_uuid); // И добавляем контакт пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->removeUserContact(NewUser->m_uuid, NewContact->m_uuid); // И наконец штатно удаляем связь
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление связи пользователь-контакты (НЕ ДОЛЖНО ВЫПОЛНЯТЬСЯ ПОЛЬЗОВАТЕЛЕМ)
 */
TEST(CombinedDataStorage, clearUserContacts)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> Storage = makeStorage(); // Создаём JSON хранилище

    Error = Storage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user(QUuid::createUuid(), "TestUser@login.com");
    std::shared_ptr<hmcommon::HMUser> NewContact1 = testscommon::make_user(QUuid::createUuid(), "TestContact1@login.com");
    std::shared_ptr<hmcommon::HMUser> NewContact2 = testscommon::make_user(QUuid::createUuid(), "TestContact2@login.com");

    Error = Storage->clearUserContacts(NewUser->m_uuid); // Пытаемся удалить не сущестующую связь
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // Должны получить сообщение о том, что пользователь в хранилище не найден

    Error = Storage->addUser(NewUser);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->addUserContact(NewUser->m_uuid, NewUser->m_uuid); // Пытаемся добавить в список контактов самого себя
    ASSERT_EQ(Error.value(), static_cast<int32_t>(hmcommon::eSystemErrorEx::seIncorretData)); // Должны получить сообщение о том, что данные не корректны

    Error = Storage->addUser(NewContact1);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->addUser(NewContact2);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->addUserContact(NewUser->m_uuid, NewContact1->m_uuid); // И добавляем контакт1 пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->addUserContact(NewUser->m_uuid, NewContact2->m_uuid); // И добавляем контакт2 пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->clearUserContacts(NewUser->m_uuid);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит получение списка контатков пользователя
 */
TEST(CombinedDataStorage, getUserContactList)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> Storage = makeStorage(); // Создаём JSON хранилище

    Error = Storage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user(QUuid::createUuid(), "TestUser@login.com");
    std::shared_ptr<hmcommon::HMUser> NewContact1 = testscommon::make_user(QUuid::createUuid(), "TestContact1@login.com");
    std::shared_ptr<hmcommon::HMUser> NewContact2 = testscommon::make_user(QUuid::createUuid(), "TestContact2@login.com");

    std::shared_ptr<std::set<QUuid>> FindRes = Storage->getUserContactList(NewUser->m_uuid, Error); // Пытаемся получить список контактов не существующего пользователя
    ASSERT_EQ(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // Должны получить сообщение о том, что пользователь в хранилище не найден
    // Добавляем пользователей
    Error = Storage->addUser(NewUser);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->addUser(NewContact1);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->addUser(NewContact2);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->addUserContact(NewUser->m_uuid, NewContact1->m_uuid); // И добавляем контакт1 пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->addUserContact(NewUser->m_uuid, NewContact2->m_uuid); // И добавляем контакт2 пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно
    // Начинаем проверять списки контактов
    FindRes = Storage->getUserContactList(NewUser->m_uuid, Error); // Теперь пытваемся получить список контактов пользователя

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_NE(FindRes->find(NewContact1->m_uuid), FindRes->end()); // Должен содержать первый контакт
    EXPECT_NE(FindRes->find(NewContact2->m_uuid), FindRes->end()); // Должен содержать второй контакт

    FindRes = Storage->getUserContactList(NewContact1->m_uuid, Error); // Проверяем список контактов второго пользователя

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_NE(FindRes->find(NewUser->m_uuid), FindRes->end()); // Должен содержать первый контакт

    FindRes = Storage->getUserContactList(NewContact2->m_uuid, Error); // Проверяем список контактов первого пользователя

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_NE(FindRes->find(NewUser->m_uuid), FindRes->end()); // Должен содержать первый контакт

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит кеширование объектов
 */
TEST(CombinedDataStorage, CacheTest)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> Storage = makeStorage(C_CACHE_LIFE_TIME_FAST, C_CACHE_SLEEP_FAST); // Создаём кеширующее хранилище (С короткой жизнью объектов)

    Error = Storage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым


    //-----
    // Проверим кеширование пользователя
    //-----
    {
        QUuid UserUUID = QUuid::createUuid();
        std::shared_ptr<hmcommon::HMUser> User = testscommon::make_user(UserUUID, "CachedUser@login.com"); // Формируем нового пользователя

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
        std::shared_ptr<hmcommon::HMGroup> Group = testscommon::make_group(GroupUUID, "CachedGroup");

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
        std::shared_ptr<hmcommon::HMUser> User = testscommon::make_user(UserUUID, "UserWithContacts@login.com"); // Формируем нового пользователя

        Error = Storage->addUser(User); // Добавляем пользователя в хранилище
        ASSERT_FALSE(Error); // Ошибки быть не должно

        const std::size_t ContactsCount = 5;
        std::shared_ptr<std::set<QUuid>> Contacts = std::make_shared<std::set<QUuid>>(); // Формируем список контактов

        for (std::size_t Index = 0; Index < ContactsCount; ++Index)
        {
            QString UserLogin = "Contact@login.com" + QString::number(Index); // Логин должен быть униакальным
            std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user(QUuid::createUuid(), UserLogin); // Формируем нового пользователя

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
        std::shared_ptr<hmcommon::HMGroup> Group = testscommon::make_group(GroupUUID, "GroupWithUsers");

        Error = Storage->addGroup(Group); // Добавляем группу в хранилище
        ASSERT_FALSE(Error); // Ошибки быть не должно

        const std::size_t UserssCount = 5;
        std::shared_ptr<std::set<QUuid>> Users = std::make_shared<std::set<QUuid>>(); // Формируем список контактов

        for (std::size_t Index = 0; Index < UserssCount; ++Index)
        {
            QString UserLogin = "GroupUser@login.com" + QString::number(Index); // Логин должен быть униакальным
            std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user(QUuid::createUuid(), UserLogin); // Формируем нового пользователя

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
