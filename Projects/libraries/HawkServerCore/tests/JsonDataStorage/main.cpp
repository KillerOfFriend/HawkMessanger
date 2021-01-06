#include <memory>
#include <filesystem>

#include <gtest/gtest.h>

#include <HawkServerCoreHardDataStorageTest.hpp>
#include <datastorage/jsondatastorage/jsondatastorage.h>

//-----------------------------------------------------------------------------
const std::filesystem::path C_JSON_PATH = std::filesystem::current_path() / "DataStorage.json";
//-----------------------------------------------------------------------------
/**
 * @brief makeStorage - Метод создаст экземпляр хранилища
 * @param inStoragePath - Путь к хранилищу
 * @param inRemoveOld - Флаг, требующий удаления старого хранилища, если оно существует
 * @return Вернёт экземпляр хранилища
 */
std::unique_ptr<HMDataStorage> makeStorage(const std::filesystem::path& inStoragePath = C_JSON_PATH, const bool inRemoveOld = true)
{
    std::error_code Error; // Метка ошибки

    if (inRemoveOld && std::filesystem::exists(inStoragePath, Error)) // При необходимости
        std::filesystem::remove(inStoragePath, Error); // Удаляем хранилище по указанному пути

    return std::make_unique<HMJsonDataStorage>(inStoragePath); // Создаём экземпляр хранилища HMJsonDataStorage
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит попытку открытия хранилища
 */
TEST(JsonDataStorage, open)
{
    std::error_code Error;

    {   // Попытка открыть хранилище (ИСТОЧНИК ДИРЕКТОРИЯ)
        std::filesystem::path DirPath = std::filesystem::temp_directory_path(Error) / "JDS_OFP";
        ASSERT_FALSE(Error);

        std::filesystem::create_directory(DirPath, Error); // Создаём временную папку
        ASSERT_FALSE(Error);

        std::unique_ptr<HMDataStorage> Storage = makeStorage(DirPath, false); // Создаём JSON хранилище с путём к директории
        Error = Storage->open(); // Пытаемся открыть по невалидному пути

        // Результат должен вернуть ошибку не соответствия пути файлу
        ASSERT_TRUE(Error.value() == static_cast<int32_t>(hmcommon::eSystemErrorEx::seObjectNotFile));
        ASSERT_FALSE(Storage->is_open()); // Хранилище не должно считаться открытым

        std::filesystem::remove_all(DirPath); // Удаляем временную папку
    }

    {   // Попытка открыть хранилище (Источник не существующий файл)
        std::unique_ptr<HMDataStorage> Storage = makeStorage(C_JSON_PATH); // Создаём JSON хранилище с путём к файлу

        Error = Storage->open();

        ASSERT_FALSE(Error); // Ошибки быть не должно
        EXPECT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

        Storage->close();
    }
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит добавление пользователя
 */
TEST(JsonDataStorage, addUser)
{
    HardDataStorage_AddUserTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит обновление пользователя
 */
TEST(JsonDataStorage, updateUser)
{
    HardDataStorage_UpdateUserTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск пользователья по UUID
 */
TEST(JsonDataStorage, findUserByUUID)
{
    HardDataStorage_FindUserByUUIDTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск пользователья по данным аутентификации
 */
TEST(JsonDataStorage, findUserByAuthentication)
{
    HardDataStorage_FindUserByAuthenticationTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление пользователя из хранилище
 */
TEST(JsonDataStorage, removeUser)
{
    HardDataStorage_RemoveUserTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит создание связи пользователь-контакты (НЕ ДОЛЖНО ВЫПОЛНЯТЬСЯ ПОЛЬЗОВАТЕЛЕМ)
 */
TEST(JsonDataStorage, setUserContacts)
{
    HardDataStorage_SetUserContactsTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит добавление контакта пользователю
 */
TEST(JsonDataStorage, addUserContact)
{
    HardDataStorage_AddUserContactTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление контакта пользователя
 */
TEST(JsonDataStorage, removeUserContact)
{
    HardDataStorage_RemoveUserContactTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление связи пользователь-контакты (НЕ ДОЛЖНО ВЫПОЛНЯТЬСЯ ПОЛЬЗОВАТЕЛЕМ)
 */
TEST(JsonDataStorage, clearUserContacts)
{
    HardDataStorage_ClearUserContactsTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит получение списка контатков пользователя
 */
TEST(JsonDataStorage, getUserContactList)
{
    HardDataStorage_GetUserContactListTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит получение списка UUID'ов групп пользователя
 */
TEST(JsonDataStorage, getUserGroups)
{
    HardDataStorage_GetUserGroupsTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит добавление группы
 */
TEST(JsonDataStorage, addGroup)
{
    HardDataStorage_AddGroupTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит обновление группы
 */
TEST(JsonDataStorage, updateGroup)
{
    HardDataStorage_UpdateGroupTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск группы по UUID
 */
TEST(JsonDataStorage, findGroupByUUID)
{
    HardDataStorage_FindGroupByUUIDTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление группы
 */
TEST(JsonDataStorage, removeGroup)
{
    HardDataStorage_RemoveGroupTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит присвоение списка участников группе
 */
TEST(JsonDataStorage, setGroupUsers)
{
    HardDataStorage_SetGroupUsersTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит добавление пользователя в группу
 */
TEST(JsonDataStorage, addGroupUser)
{
    HardDataStorage_AddGroupUserTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление пользователя из группы
 */
TEST(JsonDataStorage, removeGroupUser)
{
    HardDataStorage_RemoveGroupUserTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит очистку списка участников группы
 */
TEST(JsonDataStorage, clearGroupUsers)
{
    HardDataStorage_ClearGroupUsersTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит получение списка UUID'ов участников группы
 */
TEST(JsonDataStorage, getGroupUserList)
{
    HardDataStorage_GetGroupUserListTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит добовление сообщения
 */
TEST(JsonDataStorage, addMessage)
{
    HardDataStorage_AddMessageTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит обновление сообщения
 */
TEST(JsonDataStorage, updateMessage)
{
    HardDataStorage_UpdateMessageTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск сообщения по UUID
 */
TEST(JsonDataStorage, findMessage)
{
    HardDataStorage_FindMessageTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск перечня сообщений по временному промежутку
 */
TEST(JsonDataStorage, findMessages)
{
    HardDataStorage_FindMessagesTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление сообщения
 */
TEST(JsonDataStorage, removeMessage)
{
    HardDataStorage_RemoveMessageTest(makeStorage()); // Создаём хранилище и выполняем тест
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест сохранения хранилища в JSON
 */
TEST(JsonDataStorage, CheckJsonSave)
{
    std::error_code Error; // Метка ошибки
    std::unique_ptr<HMDataStorage> Storage = makeStorage(); // Создаём JSON хранилище

    Error = Storage->open(); // Пытаемся открыть хранилище

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user(); // Формируем пользователя

    Error = Storage->addUser(NewUser);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    // Формируем для пользователя список контактов
    const std::size_t ContactsCount = 5;
    std::array<std::shared_ptr<hmcommon::HMUser>, ContactsCount> Contacts;

    for (std::size_t Index = 0; Index < ContactsCount; ++Index)
    {
        QString ContactLogin = "Contact@login." + QString::number(Index); // У каждого пользователья должен быть уникальный UUID и логин
        Contacts[Index] = testscommon::make_user(QUuid::createUuid(), ContactLogin);
        Contacts[Index]->setName("User contact " + QString::number(Index));
        // Добавляем контакт в хранилище (Потому что контакт должен существовать)
        Error = Storage->addUser(Contacts[Index]);
        EXPECT_FALSE(Error);
        // Добавляем контакт пользователю
        Error = Storage->addUserContact(NewUser->m_uuid, Contacts[Index]->m_uuid);
        EXPECT_FALSE(Error);
    }

    std::shared_ptr<hmcommon::HMGroup> NewGroup = testscommon::make_group(); // Формируем новую группу

    Error = Storage->addGroup(NewGroup);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    // Добавляем сообщения в группу

    const size_t MESSAGES = 5;
    std::array<std::shared_ptr<hmcommon::HMGroupMessage>, MESSAGES> Messages;

    for (std::size_t Index = 0; Index < MESSAGES; ++Index)
    {
        hmcommon::MsgData Data(hmcommon::eMsgType::mtText, "ТЕКСТ сообщения");
        Messages[Index] = testscommon::make_groupmessage(Data, QUuid::createUuid(), NewGroup->m_uuid);

        Error = Storage->addMessage(Messages[Index]);
        ASSERT_FALSE(Error); // Ошибки быть не должно
    }

    // Добавляем участников в группу (из ранее созданных пользователей)
    for (std::size_t Index = 0; Index < ContactsCount; ++Index)
    {
        Error = Storage->addGroupUser(NewGroup->m_uuid, Contacts[Index]->m_uuid);
        ASSERT_FALSE(Error); // Ошибки быть не должно
    }

    std::shared_ptr<std::set<QUuid>> GroupUsers = Storage->getGroupUserList(NewGroup->m_uuid, Error);
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(GroupUsers, nullptr); // Должен вернуться валидный указатель
    EXPECT_EQ(Contacts.size(), GroupUsers->size()); // Количество контактов должно совпасть

    // Переоткрываем хранилище
    Storage->close();
    Error = Storage->open(); // Пытаемся открыть хранилище

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> FindUser = Storage->findUserByUUID(NewUser->m_uuid, Error);
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(FindUser, nullptr); // Должен вернуться валидный указатель

    std::shared_ptr<hmcommon::HMGroup> FindGroup = Storage->findGroupByUUID(NewGroup->m_uuid, Error);
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(FindGroup, nullptr); // Должен вернуться валидный указатель

    EXPECT_EQ(NewGroup->m_uuid, FindGroup->m_uuid);
    EXPECT_EQ(NewGroup->m_registrationDate, FindGroup->m_registrationDate);
    EXPECT_EQ(NewGroup->getName(), FindGroup->getName());

    // А теперь сравниваем все параметры записанного и считанного
    EXPECT_EQ(NewUser->m_uuid, FindUser->m_uuid);
    EXPECT_EQ(NewUser->m_registrationDate, FindUser->m_registrationDate);
    EXPECT_EQ(NewUser->getName(), FindUser->getName());
    EXPECT_EQ(NewUser->getLogin(), FindUser->getLogin());
    EXPECT_EQ(NewUser->getPasswordHash(), FindUser->getPasswordHash());
    EXPECT_EQ(NewUser->getSex(), FindUser->getSex());
    EXPECT_EQ(NewUser->getBirthday(), FindUser->getBirthday());

    std::shared_ptr<std::set<QUuid>> FindUCList = Storage->getUserContactList(FindUser->m_uuid, Error);
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(FindUCList, nullptr); // Должен вернуться валидный указатель
    ASSERT_EQ(FindUCList->size(), MESSAGES); // Количество контактов должно быть прежним

    for (std::size_t Index = 0; Index < MESSAGES; ++Index)
        ASSERT_NE(FindUCList->find(Contacts[Index]->m_uuid), FindUCList->end()); // Проверяем что контакт есть в списке

    std::shared_ptr<std::set<QUuid>> FindGroupUsers = Storage->getGroupUserList(NewGroup->m_uuid, Error);
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(FindGroupUsers, nullptr); // Должен вернуться валидный указатель
    EXPECT_EQ(*GroupUsers, *FindGroupUsers); // Сравниваем UUID'ы учасников группы

    for (std::size_t Index = 0; Index < MESSAGES; ++Index)
    {
        std::shared_ptr<hmcommon::HMGroupMessage> FindRes = Storage->findMessage(Messages[Index]->m_uuid, Error);
        ASSERT_FALSE(Error); // Ошибки быть не должно
        ASSERT_NE(FindGroup, nullptr); // Должен вернуться валидный указатель

        EXPECT_EQ(Messages[Index]->m_uuid, FindRes->m_uuid);
        EXPECT_EQ(Messages[Index]->m_group, FindRes->m_group);
        EXPECT_EQ(Messages[Index]->m_createTime, FindRes->m_createTime);

        auto Data1 = Messages[Index]->getMesssage();
        auto Data2 = FindRes->getMesssage();

        EXPECT_EQ(Data1.m_type, Data2.m_type);
        EXPECT_EQ(Data1.m_data, Data2.m_data);
    }

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief main - Входная точка тестировани функционала HMJsonDataStorage
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
