#include <gtest/gtest.h>

#include <thread>
#include <filesystem>

#include <systemerrorex.h>
#include <datastorage/DataStorage.h>

using namespace hmservcommon::datastorage;

//-----------------------------------------------------------------------------
/**
 * @brief make_user - Метод сформирует пользователя для тестирования
 * @param inUserUuid - UUID пользователя
 * @param inUserLogin - Логин пользователя
 * @param inUserPassword - Пароль пользователя
 * @param inCreateDate - Дата создания пользователя
 * @return Вернёт указатель на нового пользователя
 */
std::shared_ptr<hmcommon::HMUser> make_user(const QUuid inUserUuid = QUuid::createUuid(), const QString inUserLogin = "UserLogin@login.com",
                                            const QString inUserPassword = "P@ssworOfUser123", const QDateTime inCreateDate = QDateTime::currentDateTime())//QDateTime(QDate(), QTime::currentTime()))
{
    // Формируем нового пользователя
    std::shared_ptr<hmcommon::HMUser> NewUser = std::make_shared<hmcommon::HMUser>(inUserUuid, inCreateDate);
    // Задаём основные параметры
    NewUser->setLogin(inUserLogin);
    NewUser->setPassword(inUserPassword);

    return NewUser;
}
//-----------------------------------------------------------------------------
/**
 * @brief make_group - Метод сформирует группу для тестирования
 * @param inGroupUuid - UUID группы
 * @param inGroupName - Имя группы
 * @param inCreateDate - Дата создания группы
 * @return Вернёт указатель на новую группу
 */
std::shared_ptr<hmcommon::HMGroup> make_group(const QUuid inGroupUuid = QUuid::createUuid(), const QString inGroupName = "New group name",
                                              const QDateTime inCreateDate = QDateTime::currentDateTime())//QDateTime(QDate(), QTime::currentTime()))
{
    // Формируем новую группу
    std::shared_ptr<hmcommon::HMGroup> NewGroup = std::make_shared<hmcommon::HMGroup>(inGroupUuid, inCreateDate);
    NewGroup->setName(inGroupName); // Задаём имя группы

    return NewGroup;
}
//-----------------------------------------------------------------------------
/**
 * @brief make_groupmessage - Метод сформирует сообщение для тестирования
 * @param inData - Данные сообщения
 * @param inUuid - UUID сообщения
 * @param inGroupUuid - UUID группы, в которую входит сообщение
 * @param inCreateDate - Дата создания сообщения
 * @return Вернёт указатель на новое сообщение
 */
std::shared_ptr<hmcommon::HMGroupMessage> make_groupmessage(const hmcommon::MsgData& inData, const QUuid inUuid = QUuid::createUuid(), const QUuid inGroupUuid = QUuid::createUuid(),
                                                            const QDateTime inCreateDate = QDateTime::currentDateTime())
{
    std::shared_ptr<hmcommon::HMGroupMessage> NewMessage = std::make_shared<hmcommon::HMGroupMessage>(inUuid, inGroupUuid, inCreateDate);
    NewMessage->setMessage(inData);

    return NewMessage;
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит попытку открытия хранилища
 */
TEST(CachedMemoryDataStorage, Open)
{
    std::error_code Error;
    HMCachedMemoryDataStorage CachedStorage;

    Error = CachedStorage.open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    EXPECT_TRUE(CachedStorage.is_open()); // Хранилище должно считаться открытым

    CachedStorage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит добавление пользователя
 */
TEST(CachedMemoryDataStorage, AddUser)
{
    std::error_code Error;
    HMCachedMemoryDataStorage CachedStorage;

    Error = CachedStorage.open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = make_user();

    Error = CachedStorage.addUser(NewUser); // Пытаемся добавить пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = CachedStorage.addUser(NewUser); // Пытаемся добавить повторно
    EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserAlreadyExists)); // Должны получить сообщение что пользователь уже хеширован

    CachedStorage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит обновление пользователя
 */
TEST(CachedMemoryDataStorage, updateUser)
{
    std::error_code Error;
    HMCachedMemoryDataStorage CachedStorage;

    Error = CachedStorage.open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = make_user();

    Error = CachedStorage.addUser(NewUser); // Пытаемся добавить пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    NewUser->setName("New User Name");

    Error = CachedStorage.updateUser(NewUser); // Пытаемся обновить пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    CachedStorage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск пользователья по UUID
 */
TEST(CachedMemoryDataStorage, findUserByUUID)
{
    std::error_code Error;
    HMCachedMemoryDataStorage CachedStorage;

    Error = CachedStorage.open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = make_user();

    std::shared_ptr<hmcommon::HMUser> FindRes = CachedStorage.findUserByUUID(NewUser->m_uuid, Error); // Попытка получить не существующего пользователя

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // И метку, что пользователь не хеширован

    Error = CachedStorage.addUser(NewUser); // Пытаемся добавить пользователя в кеш
    ASSERT_FALSE(Error); // Ошибки быть не должно

    const size_t SilCount = 5; // Добавим некоторое количество левых пользователей
    for (size_t Index = 0; Index < SilCount; ++Index)
    {
        QString TrashUserLogin = "TrashUser" + QString::number(Index); // Логины мусорных пользователей должны быть уникальными
        Error = CachedStorage.addUser(make_user(QUuid::createUuid(), TrashUserLogin)); // Пытаемся добавить пользователя
        ASSERT_FALSE(Error); // Ошибки быть не должно
    }

    FindRes = CachedStorage.findUserByUUID(NewUser->m_uuid, Error);

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_EQ(FindRes, NewUser); // Проводим сравнение указателей (КОНЦЕПЦИЯ ХЕША: Объект всегда хранится под одним указателем)

    CachedStorage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск пользователья по данным аутентификации
 */
TEST(CachedMemoryDataStorage, findUserByAuthentication)
{
    std::error_code Error;
    HMCachedMemoryDataStorage CachedStorage;

    Error = CachedStorage.open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = make_user();

    std::shared_ptr<hmcommon::HMUser> FindRes = CachedStorage.findUserByAuthentication(NewUser->getLogin(), NewUser->getPasswordHash(), Error); // Попытка получить не существующего пользователя

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // И метку, что пользователь не хеширован

    Error = CachedStorage.addUser(NewUser); // Пытаемся добавить пользователя в кеш
    ASSERT_FALSE(Error); // Ошибки быть не должно

    const size_t SilCount = 5; // Добавим некоторое количество левых пользователей
    for (size_t Index = 0; Index < SilCount; ++Index)
    {
        QString TrashUserLogin = "TrashUser" + QString::number(Index); // Логины мусорных пользователей должны быть уникальными
        Error = CachedStorage.addUser(make_user(QUuid::createUuid(), TrashUserLogin)); // Пытаемся добавить пользователя
        ASSERT_FALSE(Error); // Ошибки быть не должно
    }

    FindRes = CachedStorage.findUserByAuthentication(NewUser->getLogin(), NewUser->getPasswordHash(), Error); // Попытка получить не существующего пользователя

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_EQ(FindRes, NewUser); // Проводим сравнение указателей (КОНЦЕПЦИЯ ХЕША: Объект всегда хранится под одним указателем)

    CachedStorage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление пользователя из кеша
 */
TEST(CachedMemoryDataStorage, removeUser)
{
    std::error_code Error;
    HMCachedMemoryDataStorage CachedStorage;

    Error = CachedStorage.open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = make_user();

    Error = CachedStorage.addUser(NewUser); // Пытаемся добавить пользователя в кеш
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = CachedStorage.removeUser(NewUser->m_uuid); // Пытаемся удалить добавленного пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<hmcommon::HMUser> FindRes = CachedStorage.findUserByUUID(NewUser->m_uuid, Error); // Попытка получить удалённого пользователя

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // И метку, что пользователь не хеширован

    CachedStorage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит добавление группы
 */
TEST(CachedMemoryDataStorage, addGroup)
{
    std::error_code Error;
    HMCachedMemoryDataStorage CachedStorage;

    Error = CachedStorage.open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = make_group();

    Error = CachedStorage.addGroup(NewGroup);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = CachedStorage.addGroup(NewGroup);
    EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupAlreadyExists)); // Должны получить сообщение что группа уже хеширована

    CachedStorage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит обновление группы
 */
TEST(CachedMemoryDataStorage, updateGroup)
{
    std::error_code Error;
    HMCachedMemoryDataStorage CachedStorage;

    Error = CachedStorage.open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = make_group();

    Error = CachedStorage.addGroup(NewGroup);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    NewGroup->setName("New Group Name");

    Error = CachedStorage.updateGroup(NewGroup);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    CachedStorage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск группы по UUID
 */
TEST(CachedMemoryDataStorage, findGroupByUUID)
{
    std::error_code Error;
    HMCachedMemoryDataStorage CachedStorage;

    Error = CachedStorage.open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = make_group();

    std::shared_ptr<hmcommon::HMGroup> FindRes = CachedStorage.findGroupByUUID(NewGroup->m_uuid, Error); // Попытка получить не существующую группу

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupNotExists)); // И метку, что группа не хеширована

    Error = CachedStorage.addGroup(NewGroup); // Пытаемся добавить группу в кеш
    ASSERT_FALSE(Error); // Ошибки быть не должно

    const size_t SilCount = 5; // Добавим некоторое количество левых групп
    for (size_t Index = 0; Index < SilCount; ++Index)
    {
        QString TrashGroupName = "TrashGroup" + QString::number(Index); // Логины мусорных пользователей должны быть уникальными
        Error = CachedStorage.addGroup(make_group(QUuid::createUuid(), TrashGroupName)); // Пытаемся добавить группу
        ASSERT_FALSE(Error); // Ошибки быть не должно
    }

    FindRes = CachedStorage.findGroupByUUID(NewGroup->m_uuid, Error);

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_EQ(FindRes, NewGroup); // Проводим сравнение указателей (КОНЦЕПЦИЯ ХЕША: Объект всегда хранится под одним указателем)

    CachedStorage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление группы
 */
TEST(CachedMemoryDataStorage, removeGroup)
{
    std::error_code Error;
    HMCachedMemoryDataStorage CachedStorage;

    Error = CachedStorage.open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = make_group();

    Error = CachedStorage.addGroup(NewGroup); // Пытаемся добавить группу в кеш
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = CachedStorage.removeGroup(NewGroup->m_uuid); // Пытаемся удалить группу пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<hmcommon::HMGroup> FindRes = CachedStorage.findGroupByUUID(NewGroup->m_uuid, Error); // Попытка получить удалённую группу

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupNotExists)); // И метку, что группа не хеширована

    CachedStorage.close();
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

    HMCachedMemoryDataStorage CachedStorage;

    Error = CachedStorage.open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = make_user(QUuid::createUuid(), "TestUser@login.com");
    std::shared_ptr<hmcommon::HMUser> NewContact1 = make_user(QUuid::createUuid(), "TestContact1@login.com");
    std::shared_ptr<hmcommon::HMUser> NewContact2 = make_user(QUuid::createUuid(), "TestContact2@login.com");

    std::shared_ptr<hmcommon::HMContactList> NewContactList = std::make_shared<hmcommon::HMContactList>();

    NewContactList->addContact(NewContact1);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    NewContactList->addContact(NewContact2);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = CachedStorage.setUserContacts(NewUser->m_uuid, NewContactList); // Пытаемся добавить список контактов без добавления пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно (Наличие пользователя в кеше не обязательно)

    Error = CachedStorage.setUserContacts(NewUser->m_uuid, NewContactList); // Повторно добавляем список
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsRelationUCAlreadyExists)); // Должны получить сообщение о том, что связь уже существует

    CachedStorage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит добавление контакта пользователю
 */
TEST(CachedMemoryDataStorage, addUserContact)
{
    std::error_code Error;

    HMCachedMemoryDataStorage CachedStorage;

    Error = CachedStorage.open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = make_user(QUuid::createUuid(), "TestUser@login.com");
    std::shared_ptr<hmcommon::HMUser> NewContact = make_user(QUuid::createUuid(), "TestContact@login.com");

    Error = CachedStorage.addUserContact(NewUser->m_uuid, NewContact);
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsRelationUCNotExists)); // Должны получить сообщение о том, что связь не существует

    std::shared_ptr<hmcommon::HMContactList> NewContactList = std::make_shared<hmcommon::HMContactList>();

    Error = CachedStorage.setUserContacts(NewUser->m_uuid, NewContactList); // Добавляем пустой список контактов
    ASSERT_FALSE(Error); // Ошибки быть не должно (Наличие пользователя в кеше не обязательно)

    Error = CachedStorage.addUserContact(NewUser->m_uuid, NewContact); // Добавляем контакт
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = CachedStorage.addUserContact(NewUser->m_uuid, NewContact); // Повторно добавляем контакт
    ASSERT_EQ(Error.value(), static_cast<int32_t>(hmcommon::eSystemErrorEx::seAlredyInContainer)); // Должны получить сообщение о том, что контакт уже в кеше

    CachedStorage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление контакта пользователя
 */
TEST(CachedMemoryDataStorage, removeUserContact)
{
    std::error_code Error;

    HMCachedMemoryDataStorage CachedStorage;

    Error = CachedStorage.open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = make_user(QUuid::createUuid(), "TestUser@login.com");
    std::shared_ptr<hmcommon::HMUser> NewContact = make_user(QUuid::createUuid(), "TestContact@login.com");

    Error = CachedStorage.removeUserContact(NewUser->m_uuid, NewContact->m_uuid); // Пытаемся удалсть не существующий контакт
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsRelationUCNotExists)); // Должны получить сообщение о том, что связи не существует

    std::shared_ptr<hmcommon::HMContactList> NewContactList = std::make_shared<hmcommon::HMContactList>();

    Error = CachedStorage.setUserContacts(NewUser->m_uuid, NewContactList); // Добавляем пустой список контактов
    ASSERT_FALSE(Error); // Ошибки быть не должно (Наличие пользователя в кеше не обязательно)

    Error = CachedStorage.addUserContact(NewUser->m_uuid, NewContact); // И добавляем контакт пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = CachedStorage.removeUserContact(NewUser->m_uuid, NewContact->m_uuid); // И наконец штатно удаляем связь
    ASSERT_FALSE(Error); // Ошибки быть не должно

    CachedStorage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление связи пользователь-контакты (НЕ ДОЛЖНО ВЫПОЛНЯТЬСЯ ПОЛЬЗОВАТЕЛЕМ)
 */
TEST(CachedMemoryDataStorage, removeUserContacts)
{
    std::error_code Error;

    HMCachedMemoryDataStorage CachedStorage;

    Error = CachedStorage.open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = make_user(QUuid::createUuid(), "TestUser@login.com");
    std::shared_ptr<hmcommon::HMUser> NewContact1 = make_user(QUuid::createUuid(), "TestContact1@login.com");
    std::shared_ptr<hmcommon::HMUser> NewContact2 = make_user(QUuid::createUuid(), "TestContact2@login.com");

    Error = CachedStorage.removeUserContacts(NewUser->m_uuid); // Пытаемся удалить не сущестующую связь
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsSuccess)); // Вне зависимости от наличия в кеше, удаление валидно

    std::shared_ptr<hmcommon::HMContactList> NewContactList = std::make_shared<hmcommon::HMContactList>();

    Error = CachedStorage.setUserContacts(NewUser->m_uuid, NewContactList); // Добавляем пустой список контактов
    ASSERT_FALSE(Error); // Ошибки быть не должно (Наличие пользователя в кеше не обязательно)

    Error = CachedStorage.addUserContact(NewUser->m_uuid, NewUser); // Пытаемся добавить в список контактов самого себя
    ASSERT_EQ(Error.value(), static_cast<int32_t>(hmcommon::eSystemErrorEx::seIncorretData)); // Должны получить сообщение о том, что данные не корректны

    Error = CachedStorage.addUserContact(NewUser->m_uuid, NewContact1); // И добавляем контакт1 пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = CachedStorage.addUserContact(NewUser->m_uuid, NewContact2); // И добавляем контакт2 пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = CachedStorage.removeUserContacts(NewUser->m_uuid); // Удаляем связь
    ASSERT_FALSE(Error); // Ошибки быть не должно

    CachedStorage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит получение списка контатков пользователя
 */
TEST(CachedMemoryDataStorage, getUserContactList)
{
    std::error_code Error;

    HMCachedMemoryDataStorage CachedStorage;

    Error = CachedStorage.open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(CachedStorage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = make_user(QUuid::createUuid(), "TestUser@login.com");
    std::shared_ptr<hmcommon::HMUser> NewContact1 = make_user(QUuid::createUuid(), "TestContact1@login.com");
    std::shared_ptr<hmcommon::HMUser> NewContact2 = make_user(QUuid::createUuid(), "TestContact2@login.com");

    std::shared_ptr<hmcommon::HMContactList> FindRes = CachedStorage.getUserContactList(NewUser->m_uuid, Error); // Пытаемся получить список контактов не существующего пользователя

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsRelationUCNotExists)); // Должны получить сообщение о том, что связь не существует

    std::shared_ptr<hmcommon::HMContactList> NewContactList = std::make_shared<hmcommon::HMContactList>();

    Error = CachedStorage.setUserContacts(NewUser->m_uuid, NewContactList); // Добавляем пустой список контактов
    ASSERT_FALSE(Error); // Ошибки быть не должно (Наличие пользователя в кеше не обязательно)

    Error = CachedStorage.addUserContact(NewUser->m_uuid, NewContact1); // И добавляем контакт1 пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = CachedStorage.addUserContact(NewUser->m_uuid, NewContact2); // И добавляем контакт2 пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно

    FindRes = CachedStorage.getUserContactList(NewUser->m_uuid, Error); // Теперь пытваемся получить список контактов пользователя

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_TRUE(FindRes->contain(NewContact1)); // Должен содержать первый контакт
    EXPECT_TRUE(FindRes->contain(NewContact2)); // Должен содержать второй контакт

    CachedStorage.close();
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
