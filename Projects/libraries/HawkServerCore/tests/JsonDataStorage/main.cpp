#include <gtest/gtest.h>

#include <thread>
#include <filesystem>

#include <systemerrorex.h>
#include <datastorage/DataStorage.h>

#include <HawkCommonTestUtils.hpp>

using namespace hmservcommon::datastorage;

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
TEST(JsonDataStorage, Open)
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
TEST(JsonDataStorage, AddUser)
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
    EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserUUIDAlreadyRegistered)); // Должны получить сообщение о том, что этот UUID уже зарегистрирован

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
TEST(JsonDataStorage, updateUser)
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
TEST(JsonDataStorage, findUserByUUID)
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
TEST(JsonDataStorage, findUserByAuthentication)
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
TEST(JsonDataStorage, removeUser)
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
 * @brief TEST - Тест проверит добавление группы
 */
TEST(JsonDataStorage, addGroup)
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
TEST(JsonDataStorage, updateGroup)
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
TEST(JsonDataStorage, findGroupByUUID)
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
TEST(JsonDataStorage, removeGroup)
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
 * @brief TEST - Тест проверит добовление сообщения
 */
TEST(JsonDataStorage, addMessage)
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
TEST(JsonDataStorage, updateMessage)
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
TEST(JsonDataStorage, findMessage)
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
TEST(JsonDataStorage, findMessages)
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
TEST(JsonDataStorage, removeMessage)
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
TEST(JsonDataStorage, setUserContacts)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> Storage = makeStorage(); // Создаём JSON хранилище

    Error = Storage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user(QUuid::createUuid(), "TestUser@login.com");
    std::shared_ptr<hmcommon::HMUser> NewContact1 = testscommon::make_user(QUuid::createUuid(), "TestContact1@login.com");
    std::shared_ptr<hmcommon::HMUser> NewContact2 = testscommon::make_user(QUuid::createUuid(), "TestContact2@login.com");

    std::shared_ptr<hmcommon::HMUserList> NewContactList = std::make_shared<hmcommon::HMUserList>();

    NewContactList->add(NewContact1);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    NewContactList->add(NewContact2);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->setUserContacts(NewUser->m_uuid, NewContactList); // Пытаемся добавить список контактов без добавления пользователя
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // Должны получить сообщение о том, что не существует пользователя, к которому нужно привязать список

    Error = Storage->addUser(NewUser); // Теперь добавим пользователя в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->setUserContacts(NewUser->m_uuid, NewContactList); // И только теперь добавляем список
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsRelationUCAlreadyExists)); // Должны получить сообщение о том, группа уже существует (Была создана пустая дефолтная группа при добавлении пользователя)

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит добавление контакта пользователю
 */
TEST(JsonDataStorage, addUserContact)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> Storage = makeStorage(); // Создаём JSON хранилище

    Error = Storage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user(QUuid::createUuid(), "TestUser@login.com");
    std::shared_ptr<hmcommon::HMUser> NewContact = testscommon::make_user(QUuid::createUuid(), "TestContact@login.com");

    Error = Storage->addUserContact(NewUser->m_uuid, NewContact); // Пытаемся добавить контакт без добавления пользователя
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsRelationUCNotExists)); // Должны получить сообщение о том, что связь не существует, т.к. пользователь "владелец" не существует в хранилище

    Error = Storage->addUser(NewUser); // Теперь добавим пользователя в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->addUserContact(NewUser->m_uuid, NewContact); // И теперь добавляем контакт
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // Должны получить сообщение о том, что пользователь-контакт не существует в хранилище

    Error = Storage->addUser(NewContact); // Теперь добавим пользователя, который будет выступать новым контактом
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->addUserContact(NewUser->m_uuid, NewContact); // И накнец штатно добавляем контакт пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление контакта пользователя
 */
TEST(JsonDataStorage, removeUserContact)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> Storage = makeStorage(); // Создаём JSON хранилище

    Error = Storage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user(QUuid::createUuid(), "TestUser@login.com");
    std::shared_ptr<hmcommon::HMUser> NewContact = testscommon::make_user(QUuid::createUuid(), "TestContact@login.com");

    Error = Storage->removeUserContact(NewUser->m_uuid, NewContact->m_uuid); // Пытаемся удалить контакт без добавления пользователя
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsRelationUCNotExists)); // Должны получить сообщение о том, что связь не существует, т.к. пользователь "владелец" не существует в хранилище

    Error = Storage->addUser(NewUser); // Теперь добавим пользователя в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->removeUserContact(NewUser->m_uuid, NewContact->m_uuid); // И теперь добавляем контакт
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsRelationUCContactNotExists)); // Должны получить сообщение о том, что контакта в связи не существует

    Error = Storage->addUser(NewContact); // Теперь добавим пользователя, который будет выступать новым контактом
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->addUserContact(NewUser->m_uuid, NewContact); // И добавляем контакт пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->removeUserContact(NewUser->m_uuid, NewContact->m_uuid); // И наконец штатно удаляем связь
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление связи пользователь-контакты (НЕ ДОЛЖНО ВЫПОЛНЯТЬСЯ ПОЛЬЗОВАТЕЛЕМ)
 */
TEST(JsonDataStorage, removeUserContacts)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> Storage = makeStorage(); // Создаём JSON хранилище

    Error = Storage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user(QUuid::createUuid(), "TestUser@login.com");
    std::shared_ptr<hmcommon::HMUser> NewContact1 = testscommon::make_user(QUuid::createUuid(), "TestContact1@login.com");
    std::shared_ptr<hmcommon::HMUser> NewContact2 = testscommon::make_user(QUuid::createUuid(), "TestContact2@login.com");

    Error = Storage->removeUserContacts(NewUser->m_uuid); // Пытаемся удалить не сущестующую связь
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsRelationUCNotExists)); // Должны получить сообщение о том, что связь не существует
    // Добавляем пользователей
    Error = Storage->addUser(NewUser);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->addUserContact(NewUser->m_uuid, NewUser); // Пытаемся добавить в список контактов самого себя
    ASSERT_EQ(Error.value(), static_cast<int32_t>(hmcommon::eSystemErrorEx::seIncorretData)); // Должны получить сообщение о том, что данные не корректны

    Error = Storage->addUser(NewContact1);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->addUser(NewContact2);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->addUserContact(NewUser->m_uuid, NewContact1); // И добавляем контакт1 пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->addUserContact(NewUser->m_uuid, NewContact2); // И добавляем контакт2 пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->removeUserContacts(NewUser->m_uuid);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит получение списка контатков пользователя
 */
TEST(JsonDataStorage, getUserContactList)
{
    std::error_code Error;
    std::unique_ptr<HMDataStorage> Storage = makeStorage(); // Создаём JSON хранилище

    Error = Storage->open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = testscommon::make_user(QUuid::createUuid(), "TestUser@login.com");
    std::shared_ptr<hmcommon::HMUser> NewContact1 = testscommon::make_user(QUuid::createUuid(), "TestContact1@login.com");
    std::shared_ptr<hmcommon::HMUser> NewContact2 = testscommon::make_user(QUuid::createUuid(), "TestContact2@login.com");

    std::shared_ptr<hmcommon::HMUserList> FindRes = Storage->getUserContactList(NewUser->m_uuid, Error); // Пытаемся получить список контактов не существующего пользователя

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsRelationUCNotExists)); // Должны получить сообщение о том, что связь не существует
    // Добавляем пользователей
    Error = Storage->addUser(NewUser);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->addUser(NewContact1);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->addUser(NewContact2);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->addUserContact(NewUser->m_uuid, NewContact1); // И добавляем контакт1 пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->addUserContact(NewUser->m_uuid, NewContact2); // И добавляем контакт2 пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно
    // Начинаем проверять списки контактов
    FindRes = Storage->getUserContactList(NewUser->m_uuid, Error); // Теперь пытваемся получить список контактов пользователя

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_TRUE(FindRes->contain(NewContact1)); // Должен содержать первый контакт
    EXPECT_TRUE(FindRes->contain(NewContact2)); // Должен содержать второй контакт

    FindRes = Storage->getUserContactList(NewContact1->m_uuid, Error); // Проверяем список контактов второго пользователя

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_TRUE(FindRes->contain(NewUser)); // Должен содержать первый контакт

    FindRes = Storage->getUserContactList(NewContact2->m_uuid, Error); // Проверяем список контактов первого пользователя

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_TRUE(FindRes->contain(NewUser)); // Должен содержать первый контакт

    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит присвоение списка участников группе
 */
TEST(JsonDataStorage, setGroupUsers)
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
TEST(JsonDataStorage, addGroupUser)
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
TEST(JsonDataStorage, removeGroupUser)
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
TEST(JsonDataStorage, clearGroupUsers)
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
TEST(JsonDataStorage, getGroupUserList)
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
 * @brief TEST - Тест проверит получение списка UUID'ов групп пользователя
 */
TEST(JsonDataStorage, getUserGroups)
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
        Error = Storage->addUserContact(NewUser->m_uuid, Contacts[Index]);
        EXPECT_FALSE(Error);
    }

    std::shared_ptr<hmcommon::HMGroup> NewGroup = testscommon::make_group(); // Формируем новую группу

    Error = NewGroup->addUser(NewUser);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage->addGroup(NewGroup);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    const size_t MESSAGES = 5;
    std::array<std::shared_ptr<hmcommon::HMGroupMessage>, MESSAGES> Messages;

    for (std::size_t Index = 0; Index < MESSAGES; ++Index)
    {
        hmcommon::MsgData Data(hmcommon::eMsgType::mtText, "ТЕКСТ сообщения");
        Messages[Index] = testscommon::make_groupmessage(Data, QUuid::createUuid(), NewGroup->m_uuid);

        Error = Storage->addMessage(Messages[Index]);
        ASSERT_FALSE(Error); // Ошибки быть не должно
    }

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

    // А теперь сравниваем все параметры записанного и считанного
    EXPECT_EQ(NewUser->m_uuid, FindUser->m_uuid);
    EXPECT_EQ(NewUser->m_registrationDate, FindUser->m_registrationDate);
    EXPECT_EQ(NewUser->getName(), FindUser->getName());
    EXPECT_EQ(NewUser->getLogin(), FindUser->getLogin());
    EXPECT_EQ(NewUser->getPasswordHash(), FindUser->getPasswordHash());
    EXPECT_EQ(NewUser->getSex(), FindUser->getSex());
    EXPECT_EQ(NewUser->getBirthday(), FindUser->getBirthday());

    std::shared_ptr<hmcommon::HMUserList> FindUCList = Storage->getUserContactList(FindUser->m_uuid, Error);
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(FindGroup, nullptr); // Должен вернуться валидный указатель
    ASSERT_EQ(FindUCList->count(), MESSAGES); // Количество контактов должно быть прежним

    for (std::size_t Index = 0; Index < MESSAGES; ++Index)
    {
        ASSERT_TRUE(FindUCList->contain(Contacts[Index])); // Проверяем что контакт есть в списке

        std::shared_ptr<hmcommon::HMUser> Contact = FindUCList->get(Contacts[Index]->m_uuid, Error);

        ASSERT_FALSE(Error); // Ошибки быть не должно
        ASSERT_NE(FindGroup, nullptr); // Должен вернуться валидный указатель
        // Проверяем что данные контакта валидны
        EXPECT_EQ(Contacts[Index]->m_uuid, Contact->m_uuid);
        EXPECT_EQ(Contacts[Index]->m_registrationDate, Contact->m_registrationDate);
        EXPECT_EQ(Contacts[Index]->getName(), Contact->getName());
        EXPECT_EQ(Contacts[Index]->getLogin(), Contact->getLogin());
        EXPECT_EQ(Contacts[Index]->getPasswordHash(), Contact->getPasswordHash());
        EXPECT_EQ(Contacts[Index]->getSex(), Contact->getSex());
        EXPECT_EQ(Contacts[Index]->getBirthday(), Contact->getBirthday());
        // Проверяем обратную связь
        std::shared_ptr<hmcommon::HMUserList> ReverseUCList = Storage->getUserContactList(Contacts[Index]->m_uuid, Error);

        ASSERT_FALSE(Error); // Ошибки быть не должно
        ASSERT_NE(ReverseUCList, nullptr); // Должен вернуться валидный указатель
        ASSERT_EQ(ReverseUCList->count(), 1); // Должна быть всего одна обратныя связь
        ASSERT_TRUE(ReverseUCList->contain(FindUser->m_uuid)); // Проверяем что в списке именно этот контакт
    }

    EXPECT_EQ(NewGroup->m_uuid, FindGroup->m_uuid);
    EXPECT_EQ(NewGroup->m_registrationDate, FindGroup->m_registrationDate);
    EXPECT_EQ(NewGroup->getName(), FindGroup->getName());
//    EXPECT_EQ(NewGroup->usersCount(), FindGroup->usersCount());

//    for (std::size_t Index = 0; Index < NewGroup->usersCount(); ++Index)
//    {
//        std::shared_ptr<hmcommon::HMUser> User1 = NewGroup->getUser(Index, Error);
//        ASSERT_NE(User1, nullptr); // Должен вернуться валидный указатель
//        ASSERT_FALSE(Error); // Ошибки быть не должно
//        std::shared_ptr<hmcommon::HMUser> User2 = FindGroup->getUser(Index, Error);
//        ASSERT_NE(User2, nullptr); // Должен вернуться валидный указатель
//        ASSERT_FALSE(Error); // Ошибки быть не должно

//        EXPECT_EQ(User1->m_uuid, User2->m_uuid);
//    }

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
