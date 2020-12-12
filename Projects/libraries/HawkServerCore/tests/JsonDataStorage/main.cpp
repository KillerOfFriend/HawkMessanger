#include <gtest/gtest.h>

#include <thread>
#include <filesystem>

#include <systemerrorex.h>
#include <datastorage/DataStorage.h>

using namespace hmservcommon::datastorage;

//-----------------------------------------------------------------------------
const std::filesystem::path C_JSON_PATH = std::filesystem::current_path() / "DataStorage.json";
//-----------------------------------------------------------------------------
/**
 * @brief clearStorage - Функция очистит хранилище
 */
void clearStorage()
{
    std::error_code Error; // Метка ошибки

    if (std::filesystem::exists(C_JSON_PATH, Error))
        std::filesystem::remove(C_JSON_PATH, Error);
}
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
TEST(JsonDataStorage, Open)
{
    std::error_code Error;
    clearStorage();

    {   // Попытка открыть хранилище (ИСТОЧНИК ДИРЕКТОРИЯ)
        std::filesystem::path DirPath = std::filesystem::temp_directory_path(Error) / "JDS_OFP";
        ASSERT_FALSE(Error);

        std::filesystem::create_directory(DirPath, Error); // Создаём временную папку
        ASSERT_FALSE(Error);

        HMJsonDataStorage Storage(DirPath); // Создаём JSON хранилище с путём к директории
        Error = Storage.open(); // Пытаемся открыть по невалидному пути

        // Результат должен вернуть ошибку не соответствия пути файлу
        ASSERT_TRUE(Error.value() == static_cast<int32_t>(hmcommon::eSystemErrorEx::seObjectNotFile));
        ASSERT_FALSE(Storage.is_open()); // Хранилище не должно считаться открытым

        std::filesystem::remove_all(DirPath); // Удаляем временную папку
    }

    {   // Попытка открыть хранилище (Источник не существующий файл)
        HMJsonDataStorage Storage(C_JSON_PATH); // Создаём JSON хранилище с путём к директории

        Error = Storage.open();

        ASSERT_FALSE(Error); // Ошибки быть не должно
        EXPECT_TRUE(Storage.is_open()); // Хранилище должно считаться открытым

        Storage.close();
    }
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит добавление пользователя
 */
TEST(JsonDataStorage, AddUser)
{
    std::error_code Error;
    clearStorage();

    HMJsonDataStorage Storage(C_JSON_PATH); // Создаём JSON хранилище с путём к директории

    Error = Storage.open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = make_user();

    Error = Storage.addUser(NewUser); // Пытаемся добавить пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage.addUser(NewUser); // Пытаемся добавить повторно
    EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserUUIDAlreadyRegistered)); // Должны получить сообщение о том, что этот UUID уже зарегистрирован

    NewUser = make_user(QUuid::createUuid()); // Формируем такого же пользователя но с другим UUID

    Error = Storage.addUser(NewUser); // Пытаемся добавить с новым UUID
    EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserLoginAlreadyRegistered)); // Должны получить сообщение о том, что этот логин уже занят

    NewUser = make_user(QUuid::createUuid(), "OtherUser@login.com"); // Формируем с другим UUID и логином

    Error = Storage.addUser(NewUser); // Пытаемся добавить с другим UUID и логином
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Storage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит обновление пользователя
 */
TEST(JsonDataStorage, updateUser)
{
    std::error_code Error;
    clearStorage();

    HMJsonDataStorage Storage(C_JSON_PATH); // Создаём JSON хранилище с путём к директории

    Error = Storage.open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = make_user();

    Error = Storage.addUser(NewUser); // Пытаемся добавить пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    NewUser->setName("New User Name");

    Error = Storage.updateUser(NewUser); // Пытаемся обновить пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    NewUser = make_user(); // Формируем нового пользователя

    Error = Storage.updateUser(NewUser); // Пытаемся обновить пользователя, не добавленного в хранилище
    EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // Должны получить сообщение о том, что нет такого пользователя

    Storage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск пользователья по UUID
 */
TEST(JsonDataStorage, findUserByUUID)
{
    std::error_code Error;
    clearStorage();

    HMJsonDataStorage Storage(C_JSON_PATH); // Создаём JSON хранилище с путём к директории

    Error = Storage.open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = make_user();

    std::shared_ptr<hmcommon::HMUser> FindRes = Storage.findUserByUUID(NewUser->m_uuid, Error); // Попытка получить не существующего пользователя

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // И метку, что пользователь не найден в хранилище

    Error = Storage.addUser(NewUser); // Пытаемся добавить пользователя в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    const size_t SilCount = 5; // Добавим некоторое количество левых пользователей
    for (size_t Index = 0; Index < SilCount; ++Index)
    {
        QString TrashUserLogin = "TrashUser" + QString::number(Index); // Логины мусорных пользователей должны быть уникальными
        Error = Storage.addUser(make_user(QUuid::createUuid(), TrashUserLogin)); // Пытаемся добавить пользователя
        ASSERT_FALSE(Error); // Ошибки быть не должно
    }

    FindRes = Storage.findUserByUUID(NewUser->m_uuid, Error);

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_EQ(*NewUser, *FindRes); // Полное сравнение объектов должно пройти успешно

    Storage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск пользователья по данным аутентификации
 */
TEST(JsonDataStorage, findUserByAuthentication)
{
    std::error_code Error;
    clearStorage();

    HMJsonDataStorage Storage(C_JSON_PATH); // Создаём JSON хранилище с путём к директории

    Error = Storage.open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = make_user();

    std::shared_ptr<hmcommon::HMUser> FindRes = Storage.findUserByAuthentication(NewUser->getLogin(), NewUser->getPasswordHash(), Error); // Попытка получить не существующего пользователя

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // И метку, что пользователь не добавлен в хранилище

    Error = Storage.addUser(NewUser); // Пытаемся добавить пользователя в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    const size_t SilCount = 5; // Добавим некоторое количество левых пользователей
    for (size_t Index = 0; Index < SilCount; ++Index)
    {
        QString TrashUserLogin = "TrashUser" + QString::number(Index); // Логины мусорных пользователей должны быть уникальными
        Error = Storage.addUser(make_user(QUuid::createUuid(), TrashUserLogin)); // Пытаемся добавить пользователя
        ASSERT_FALSE(Error); // Ошибки быть не должно
    }

    FindRes = Storage.findUserByAuthentication(NewUser->getLogin(), NewUser->getPasswordHash(), Error); // Попытка получить не существующего пользователя

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_EQ(*NewUser, *FindRes); // Полное сравнение объектов должно пройти успешно

    Storage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление пользователя из хранилище
 */
TEST(JsonDataStorage, removeUser)
{
    std::error_code Error;
    clearStorage();

    HMJsonDataStorage Storage(C_JSON_PATH); // Создаём JSON хранилище с путём к директории

    Error = Storage.open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = make_user();

    Error = Storage.addUser(NewUser); // Пытаемся добавить пользователя в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage.removeUser(NewUser->m_uuid); // Пытаемся удалить добавленного пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<hmcommon::HMUser> FindRes = Storage.findUserByUUID(NewUser->m_uuid, Error); // Попытка получить удалённого пользователя

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // И метку, что пользователь не найден в хранилище

    Error = Storage.removeUser(NewUser->m_uuid); // Пытаемся удалить не существующего пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Storage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит добавление группы
 */
TEST(JsonDataStorage, addGroup)
{
    std::error_code Error;
    clearStorage();

    HMJsonDataStorage Storage(C_JSON_PATH); // Создаём JSON хранилище с путём к директории

    Error = Storage.open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = make_group();

    Error = Storage.addGroup(NewGroup);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage.addGroup(NewGroup);
    EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupUUIDAlreadyRegistered)); // Должны получить сообщение о том, что этот UUID уже зарегистрирован

    Storage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит обновление группы
 */
TEST(JsonDataStorage, updateGroup)
{
    std::error_code Error;
    clearStorage();

    HMJsonDataStorage Storage(C_JSON_PATH); // Создаём JSON хранилище с путём к директории

    Error = Storage.open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = make_group();

    Error = Storage.addGroup(NewGroup);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    NewGroup->setName("New Group Name");

    Error = Storage.updateGroup(NewGroup);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    NewGroup = make_group(); // Формируем новую группу

    Error = Storage.updateGroup(NewGroup); // Пытаемся обновить группу, не добавленную в хранилище
    EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupNotExists)); // Должны получить сообщение о том, что нет такой группы

    Storage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск группы по UUID
 */
TEST(JsonDataStorage, findGroupByUUID)
{
    std::error_code Error;
    clearStorage();

    HMJsonDataStorage Storage(C_JSON_PATH); // Создаём JSON хранилище с путём к директории

    Error = Storage.open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = make_group();

    std::shared_ptr<hmcommon::HMGroup> FindRes = Storage.findGroupByUUID(NewGroup->m_uuid, Error); // Попытка получить не существующую группу

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupNotExists)); // И метку, что группа не найдена в хранилище

    Error = Storage.addGroup(NewGroup); // Пытаемся добавить группу в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    const size_t SilCount = 5; // Добавим некоторое количество левых групп
    for (size_t Index = 0; Index < SilCount; ++Index)
    {
        QString TrashGroupName = "TrashGroup" + QString::number(Index); // Логины мусорных пользователей должны быть уникальными
        Error = Storage.addGroup(make_group(QUuid::createUuid(), TrashGroupName)); // Пытаемся добавить группу
        ASSERT_FALSE(Error); // Ошибки быть не должно
    }

    FindRes = Storage.findGroupByUUID(NewGroup->m_uuid, Error);

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_EQ(*NewGroup, *FindRes); // Полное сравнение объектов должно пройти успешно

    Storage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление группы
 */
TEST(JsonDataStorage, removeGroup)
{
    std::error_code Error;
    clearStorage();

    HMJsonDataStorage Storage(C_JSON_PATH); // Создаём JSON хранилище с путём к директории

    Error = Storage.open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = make_group();

    Error = Storage.addGroup(NewGroup); // Пытаемся добавить группу в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage.removeGroup(NewGroup->m_uuid); // Пытаемся удалить группу пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<hmcommon::HMGroup> FindRes = Storage.findGroupByUUID(NewGroup->m_uuid, Error); // Попытка получить удалённую группу

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupNotExists)); // И метку, что группа не найдена в хранилище

    Error = Storage.removeGroup(NewGroup->m_uuid); // Пытаемся удалить не существующую группу
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Storage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит добовление сообщения
 */
TEST(JsonDataStorage, addMessage)
{
    std::error_code Error; // Метка ошибки
    clearStorage(); // Очищаем хранилище

    HMJsonDataStorage Storage(C_JSON_PATH); // Создаём JSON хранилище с путём к директории
    Error = Storage.open(); // Пытаемся открыть хранилище

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage.is_open()); // Хранилище должно считаться открытым

    // Сообщениее может быть добавлено только в группу
    std::shared_ptr<hmcommon::HMGroup> NewGroup = make_group();

    hmcommon::MsgData TextData(hmcommon::eMsgType::mtText, "Текст сообщения"); // Формируем данные сообщения
    std::shared_ptr<hmcommon::HMGroupMessage> NewMessage = make_groupmessage(TextData, QUuid::createUuid(), NewGroup->m_uuid); // Формируем сообщение

    Error = Storage.addMessage(NewMessage); // Пытаемся добавить сообщение без группы
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupNotExists)); // И метку, что группа не найдена в хранилище

    Error = Storage.addGroup(NewGroup); // Теперь добавим группу
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage.addMessage(NewMessage); // Пытаемся добавить сообщение
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage.addMessage(NewMessage); // Пытаемся добавить сообщение повторно
    EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsMessageAlreadyExists)); // Должны получить сообщение о том, что этот UUID уже зарегистрирован

    Storage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит обновление сообщения
 */
TEST(JsonDataStorage, updateMessage)
{
    std::error_code Error; // Метка ошибки
    clearStorage(); // Очищаем хранилище

    HMJsonDataStorage Storage(C_JSON_PATH); // Создаём JSON хранилище с путём к директории
    Error = Storage.open(); // Пытаемся открыть хранилище

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage.is_open()); // Хранилище должно считаться открытым

    // Сообщениее может быть добавлено только в группу
    std::shared_ptr<hmcommon::HMGroup> NewGroup = make_group();

    Error = Storage.addGroup(NewGroup);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    hmcommon::MsgData TextData(hmcommon::eMsgType::mtText, "Текст сообщения"); // Формируем данные сообщения
    std::shared_ptr<hmcommon::HMGroupMessage> NewMessage = make_groupmessage(TextData, QUuid::createUuid(), NewGroup->m_uuid); // Формируем сообщение

    Error = Storage.updateMessage(NewMessage); // Пытаемся обновить сообщение не добавляя
    EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsMessageNotExists)); // Должны получить сообщение о том, что не удалось найти сообщение для обновления

    Error = Storage.addMessage(NewMessage); // Пытаемся добавить сообщение
    ASSERT_FALSE(Error); // Ошибки быть не должно

    TextData.m_data = QString("Новый текст сообщения").toLocal8Bit(); // Меняем текст
    NewMessage->setMessage(TextData);

    Error = Storage.updateMessage(NewMessage); // Пытаемся обновить сообщение
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Storage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск сообщения по UUID
 */
TEST(JsonDataStorage, findMessage)
{
    std::error_code Error; // Метка ошибки
    clearStorage(); // Очищаем хранилище

    HMJsonDataStorage Storage(C_JSON_PATH); // Создаём JSON хранилище с путём к директории
    Error = Storage.open(); // Пытаемся открыть хранилище

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage.is_open()); // Хранилище должно считаться открытым

    // Сообщениее может быть добавлено только в группу
    std::shared_ptr<hmcommon::HMGroup> NewGroup = make_group();

    hmcommon::MsgData TextData(hmcommon::eMsgType::mtText, "Текст сообщения"); // Формируем данные сообщения
    std::shared_ptr<hmcommon::HMGroupMessage> NewMessage = make_groupmessage(TextData, QUuid::createUuid(), NewGroup->m_uuid); // Формируем сообщение

    std::shared_ptr<hmcommon::HMGroupMessage> FindRes = Storage.findMessage(NewMessage->m_uuid, Error); // Пытаемся найти не добавленное сообщение

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsMessageNotExists)); // Должны получить сообщение о том, что сообщение не найдено

    Error = Storage.addGroup(NewGroup); // Добавим группу
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage.addMessage(NewMessage); // Пытаемся добавить сообщение
    ASSERT_FALSE(Error); // Ошибки быть не должно

    FindRes = Storage.findMessage(NewMessage->m_uuid, Error); // Пытаемся найти не добавленное сообщение

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Storage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит поиск перечня сообщений по временному промежутку
 */
TEST(JsonDataStorage, findMessages)
{
    std::error_code Error; // Метка ошибки
    clearStorage(); // Очищаем хранилище

    hmservcommon::datastorage::HMJsonDataStorage Storage(C_JSON_PATH); // Создаём JSON хранилище с путём к директории
    Error = Storage.open(); // Пытаемся открыть хранилище

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = make_group();

    Error = Storage.addGroup(NewGroup); // добавляем группу сообщения
    ASSERT_FALSE(Error); // Ошибки быть не должно

    const size_t MESSAGES = 5;
    std::array<std::shared_ptr<hmcommon::HMGroupMessage>, MESSAGES> Messages;
    std::array<QDateTime, MESSAGES> Times;

    for (std::size_t Index = 0; Index < MESSAGES; ++Index)
    {
        //QDateTime(QDate(), QTime::currentTime()); // Запоминаем время
        Times[Index] = QDateTime::currentDateTime();
        hmcommon::MsgData TextData(hmcommon::eMsgType::mtText, ("Текст сообщения " + QString::number(Index)).toLocal8Bit()); // Формируем данные сообщения
        Messages[Index] = make_groupmessage(TextData, QUuid::createUuid(), NewGroup->m_uuid, Times[Index]); // Формируем сообщение группы

        Error = Storage.addMessage(Messages[Index]);
        ASSERT_FALSE(Error); // Ошибки быть не должно
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    hmcommon::MsgRange TimeRange(Times[0], Times[MESSAGES - 2]); // Выбираем временной интервал со временни первого до времени предпоследнего сообщения
    std::vector<std::shared_ptr<hmcommon::HMGroupMessage>> FindRes = Storage.findMessages(NewGroup->m_uuid, TimeRange, Error); // Получаем резульата

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

    Storage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест проверит удаление сообщения
 */
TEST(JsonDataStorage, removeMessage)
{
    std::error_code Error;
    clearStorage();

    HMJsonDataStorage Storage(C_JSON_PATH); // Создаём JSON хранилище с путём к директории

    Error = Storage.open();

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage.is_open()); // Хранилище должно считаться открытым

    // Сообщениее может быть добавлено только в группу
    std::shared_ptr<hmcommon::HMGroup> NewGroup = make_group();

    hmcommon::MsgData TextData(hmcommon::eMsgType::mtText, "Текст сообщения"); // Формируем данные сообщения
    std::shared_ptr<hmcommon::HMGroupMessage> NewMessage = make_groupmessage(TextData, QUuid::createUuid(), NewGroup->m_uuid); // Формируем сообщение

    Error = Storage.addGroup(NewGroup); // Добавим группу
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage.removeGroup(NewGroup->m_uuid); // Пытаемся удалить добавленное сообщение
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<hmcommon::HMGroupMessage> FindRes = Storage.findMessage(NewGroup->m_uuid, Error); // Попытка получить удалённое сообщение

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsMessageNotExists)); // И метку, что сообщение не найдено в хранилище

    Error = Storage.removeGroup(NewGroup->m_uuid); // Пытаемся удалить не существующее сообщение
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Storage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест сохранения хранилища в JSON
 */
TEST(JsonDataStorage, CheckJsonSave)
{
    std::error_code Error; // Метка ошибки
    clearStorage(); // Очищаем хранилище

    HMJsonDataStorage Storage(C_JSON_PATH); // Создаём JSON хранилище с путём к директории
    Error = Storage.open(); // Пытаемся открыть хранилище

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = make_user(); // Формируем пользователя

    // Формируем для пользователя список контактов
    const std::size_t ContactsCount = 5;
    std::array<std::shared_ptr<hmcommon::HMUser>, ContactsCount> Contacts;

    for (std::size_t Index = 0; Index < ContactsCount; ++Index)
    {
//        QString ContactLogin = "Contact@login." + QString::number(Index); // У каждого пользователья должен быть уникальный UUID и логин
//        Contacts[Index] = make_user(QUuid::createUuid(), ContactLogin);
//        Contacts[Index]->setName("User contact " + QString::number(Index));
//        // Добавляем контакт в хранилище (Потому что контакт должен существовать)
//        Error = Storage.addUser(Contacts[Index]);
//        EXPECT_FALSE(Error);
//        // Добавляем контакт пользователю
//        Error = NewUser->m_contactList.addContact(Contacts[Index]);
//        EXPECT_FALSE(Error);
    }

    std::shared_ptr<hmcommon::HMGroup> NewGroup = make_group(); // Формируем новую группу

    Error = NewGroup->addUser(NewUser->m_uuid);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage.addUser(NewUser);
    ASSERT_FALSE(Error); // Ошибки быть не должно
    Error = Storage.addGroup(NewGroup);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    const size_t MESSAGES = 5;
    std::array<std::shared_ptr<hmcommon::HMGroupMessage>, MESSAGES> Messages;

    for (std::size_t Index = 0; Index < MESSAGES; ++Index)
    {
        hmcommon::MsgData Data(hmcommon::eMsgType::mtText, "ТЕКСТ сообщения");
        Messages[Index] = make_groupmessage(Data, QUuid::createUuid(), NewGroup->m_uuid);

        Error = Storage.addMessage(Messages[Index]);
        ASSERT_FALSE(Error); // Ошибки быть не должно
    }

    // Переоткрываем хранилище
    Storage.close();
    Error = Storage.open(); // Пытаемся открыть хранилище

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> FindUser = Storage.findUserByUUID(NewUser->m_uuid, Error);
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(FindUser, nullptr); // Должен вернуться валидный указатель

    std::shared_ptr<hmcommon::HMGroup> FindGroup = Storage.findGroupByUUID(NewGroup->m_uuid, Error);
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

    for (std::size_t Index = 0; Index < MESSAGES; ++Index)
    {
//        std::shared_ptr<hmcommon::HMUser> Contact = FindUser->m_contactList.getContact(Contacts[Index]->m_uuid, Error); // Запрашиваем контакт пользователя по UUID
//        ASSERT_FALSE(Error); // Ошибки быть не должно
//        ASSERT_NE(FindGroup, nullptr); // Должен вернуться валидный указатель

//        EXPECT_EQ(Contacts[Index]->m_uuid, Contact->m_uuid);
//        EXPECT_EQ(Contacts[Index]->m_registrationDate, Contact->m_registrationDate);
//        EXPECT_EQ(Contacts[Index]->getName(), Contact->getName());
//        EXPECT_EQ(Contacts[Index]->getLogin(), Contact->getLogin());
//        EXPECT_EQ(Contacts[Index]->getPasswordHash(), Contact->getPasswordHash());
//        EXPECT_EQ(Contacts[Index]->getSex(), Contact->getSex());
//        EXPECT_EQ(Contacts[Index]->getBirthday(), Contact->getBirthday());
    }

    EXPECT_EQ(NewGroup->m_uuid, FindGroup->m_uuid);
    EXPECT_EQ(NewGroup->m_registrationDate, FindGroup->m_registrationDate);
    EXPECT_EQ(NewGroup->getName(), FindGroup->getName());
    EXPECT_EQ(NewGroup->usersCount(), FindGroup->usersCount());

    for (std::size_t Index = 0; Index < NewGroup->usersCount(); ++Index)
    {
        QUuid Uuid1 = NewGroup->getUser(Index, Error);
        EXPECT_FALSE(Error); // Ошибки быть не должно
        QUuid Uuid2 = FindGroup->getUser(Index, Error);
        EXPECT_FALSE(Error); // Ошибки быть не должно

        EXPECT_EQ(Uuid1, Uuid2);
    }

    for (std::size_t Index = 0; Index < MESSAGES; ++Index)
    {
        std::shared_ptr<hmcommon::HMGroupMessage> FindRes = Storage.findMessage(Messages[Index]->m_uuid, Error);
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

    Storage.close();
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
