#include <gtest/gtest.h>

#include <thread>
#include <filesystem>

#include <jsondatastorage.h>
#include <systemerrorex.h>
#include <datastorageerrorcategory.h>

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
 * @brief TEST - Тест проверит попытку открытия хранилища по невалидному пути
 */
TEST(JsonDataStorage, OpenFailPath)
{
    std::error_code Error; // Метка ошибки

    std::filesystem::path DirPath = std::filesystem::temp_directory_path(Error) / "JDS_OFP";
    ASSERT_FALSE(Error);

    std::filesystem::create_directory(DirPath, Error); // Создаём временную папку
    ASSERT_FALSE(Error);

    hmservcommon::HMJsonDataStorage Storage(DirPath); // Создаём JSON хранилище с путём к директории
    Error = Storage.open(); // Пытаемся открыть по невалидному пути

    // Результат должен вернуть ошибку не соответствия пути файлу
    ASSERT_TRUE(Error.value() == static_cast<int32_t>(hmcommon::eSystemErrorEx::seObjectNotFile));
    ASSERT_FALSE(Storage.is_open()); // Хранилище не должно считаться открытым

    std::filesystem::remove_all(DirPath); // Удаляем временную папку
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест поиска несуществующего пользователя
 */
TEST(JsonDataStorage, FindNotExistUser)
{
    std::error_code Error; // Метка ошибки
    clearStorage(); // Очищаем хранилище

    hmservcommon::HMJsonDataStorage Storage(C_JSON_PATH); // Создаём JSON хранилище с путём к директории
    Error = Storage.open(); // Пытаемся открыть хранилище

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = make_user(); // Формируем пользователя, которого нет в хранилище
    std::shared_ptr<hmcommon::HMUser> FindRes = Storage.findUserByUUID(NewUser->m_uuid, Error); // Ищим несуществующего пользователя по UUID

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_TRUE(Error.value() == static_cast<int32_t>(hmservcommon::eDataStoragError::dsUserNotExists)); // И метку, что пользователь не существует

    FindRes = Storage.findUserByAuthentication(NewUser->getLogin(), NewUser->getPasswordHash(), Error); // Ищим несуществующего пользователя по данным аутентификации

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_TRUE(Error.value() == static_cast<int32_t>(hmservcommon::eDataStoragError::dsUserNotExists)); // И метку, что пользователь не существует

    Storage.close(); // Закрываем хранилище
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест поиска существующего пользователя
 */
TEST(JsonDataStorage, FindExistUser)
{
    std::error_code Error; // Метка ошибки
    clearStorage(); // Очищаем хранилище

    hmservcommon::HMJsonDataStorage Storage(C_JSON_PATH); // Создаём JSON хранилище с путём к директории
    Error = Storage.open(); // Пытаемся открыть хранилище

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = make_user(); // Формируем пользователя

    Error = Storage.addUser(NewUser); // Добавляем пользователья в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<hmcommon::HMUser> FindRes = Storage.findUserByUUID(NewUser->m_uuid, Error); // Ищим существующего пользователя по UUID

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    FindRes = Storage.findUserByAuthentication(NewUser->getLogin(), NewUser->getPasswordHash(), Error); // Ищим существующего пользователя по данным аутентификации

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Storage.close(); // Закрываем хранилище
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест поиска несуществующей группы
 */
TEST(JsonDataStorage, FindNotExistGroup)
{
    std::error_code Error; // Метка ошибки
    clearStorage(); // Очищаем хранилище

    hmservcommon::HMJsonDataStorage Storage(C_JSON_PATH); // Создаём JSON хранилище с путём к директории
    Error = Storage.open(); // Пытаемся открыть хранилище

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = make_group(); // Формируем группу, которой нет в хранилище
    std::shared_ptr<hmcommon::HMGroup> FindRes = Storage.findGroupByUUID(NewGroup->m_uuid, Error); // Ищим несуществующую группу по UUID

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_TRUE(Error.value() == static_cast<int32_t>(hmservcommon::eDataStoragError::dsGroupNotExists)); // И метку, что группа не существует

    Storage.close(); // Закрываем хранилище
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест поиска существующей группы
 */
TEST(JsonDataStorage, FindExistGroup)
{
    std::error_code Error; // Метка ошибки
    clearStorage(); // Очищаем хранилище

    hmservcommon::HMJsonDataStorage Storage(C_JSON_PATH); // Создаём JSON хранилище с путём к директории
    Error = Storage.open(); // Пытаемся открыть хранилище

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = make_group(); // Формируем новую группу

    Error = Storage.addGroup(NewGroup);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<hmcommon::HMGroup> FindRes = Storage.findGroupByUUID(NewGroup->m_uuid, Error); // Ищим существующую группу по UUID

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Storage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест поиска несуществующего сообщения
 */
TEST(JsonDataStorage, FindNotExistMessage)
{
    std::error_code Error; // Метка ошибки
    clearStorage(); // Очищаем хранилище

    hmservcommon::HMJsonDataStorage Storage(C_JSON_PATH); // Создаём JSON хранилище с путём к директории
    Error = Storage.open(); // Пытаемся открыть хранилище

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage.is_open()); // Хранилище должно считаться открытым

    hmcommon::MsgData TextData(hmcommon::eMsgType::mtText, "Текст сообщения"); // Формируем данные сообщения
    std::shared_ptr<hmcommon::HMGroupMessage> NewMessage = make_groupmessage(TextData); // Формируем сообщение

    std::shared_ptr<hmcommon::HMGroupMessage> FindRes = Storage.findMessage(NewMessage->m_uuid, Error);

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_TRUE(Error.value() == static_cast<int32_t>(hmservcommon::eDataStoragError::dsMessageNotExists)); // И метку, что сообщение не существует

    Storage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест поиска существующего сообщения
 */
TEST(JsonDataStorage, FindExistMessage)
{
    std::error_code Error; // Метка ошибки
    clearStorage(); // Очищаем хранилище

    hmservcommon::HMJsonDataStorage Storage(C_JSON_PATH); // Создаём JSON хранилище с путём к директории
    Error = Storage.open(); // Пытаемся открыть хранилище

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = make_group();

    Error = Storage.addGroup(NewGroup); // добавляем группу сообщения
    ASSERT_FALSE(Error); // Ошибки быть не должно

    hmcommon::MsgData TextData(hmcommon::eMsgType::mtText, "Текст сообщения"); // Формируем данные сообщения
    std::shared_ptr<hmcommon::HMGroupMessage> NewMessage = make_groupmessage(TextData, QUuid::createUuid(), NewGroup->m_uuid); // Формируем сообщение

    Error = Storage.addMessage(NewMessage); // Добавляем сообщение
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<hmcommon::HMGroupMessage> FindRes = Storage.findMessage(NewMessage->m_uuid, Error);

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Storage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест поиска не существующих сообщений
 */
TEST(JsonDataStorage, FindNotExistMessages)
{
    std::error_code Error; // Метка ошибки
    clearStorage(); // Очищаем хранилище

    hmservcommon::HMJsonDataStorage Storage(C_JSON_PATH); // Создаём JSON хранилище с путём к директории
    Error = Storage.open(); // Пытаемся открыть хранилище

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage.is_open()); // Хранилище должно считаться открытым

    hmcommon::MsgData TextData(hmcommon::eMsgType::mtText, "Текст сообщения"); // Формируем данные сообщения
    std::shared_ptr<hmcommon::HMGroupMessage> NewMessage = make_groupmessage(TextData); // Формируем сообщение

    hmcommon::MsgRange Range(QDateTime::currentDateTime(), QDateTime::currentDateTime());
    std::vector<std::shared_ptr<hmcommon::HMGroupMessage>> FindRes = Storage.findMessages(NewMessage->m_group, Range, Error);

    ASSERT_EQ(FindRes.empty(), true); // Должен вернуться пустой контейнер
    ASSERT_TRUE(Error.value() == static_cast<int32_t>(hmservcommon::eDataStoragError::dsMessageNotExists)); // И метку, что сообщение не существует

    Storage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест поиска существующих сообщений
 */
TEST(JsonDataStorage, FindExistMessages)
{
    std::error_code Error; // Метка ошибки
    clearStorage(); // Очищаем хранилище

    hmservcommon::HMJsonDataStorage Storage(C_JSON_PATH); // Создаём JSON хранилище с путём к директории
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
 * @brief TEST - Тест удаления пользователя
 */
TEST(JsonDataStorage, RemoveUser)
{
    std::error_code Error; // Метка ошибки
    clearStorage(); // Очищаем хранилище

    hmservcommon::HMJsonDataStorage Storage(C_JSON_PATH); // Создаём JSON хранилище с путём к директории
    Error = Storage.open(); // Пытаемся открыть хранилище

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = make_user(); // Формируем пользователя

    Error = Storage.addUser(NewUser); // Добавляем пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<hmcommon::HMUser> FindRes = Storage.findUserByUUID(NewUser->m_uuid, Error); // Ищим существующего пользователя по UUID

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage.removeUser(NewUser->m_uuid);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    FindRes = Storage.findUserByUUID(NewUser->m_uuid, Error); // Ищим удалённого пользователя по UUID

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_TRUE(Error.value() == static_cast<int32_t>(hmservcommon::eDataStoragError::dsUserNotExists)); // И метку, что пользователь не существует

    Storage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест удаления группы
 */
TEST(JsonDataStorage, RemoveGroup)
{
    std::error_code Error; // Метка ошибки
    clearStorage(); // Очищаем хранилище

    hmservcommon::HMJsonDataStorage Storage(C_JSON_PATH); // Создаём JSON хранилище с путём к директории
    Error = Storage.open(); // Пытаемся открыть хранилище

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = make_group(); // Формируем группу

    Error = Storage.addGroup(NewGroup); // Добавляем группу
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<hmcommon::HMGroup> FindRes = Storage.findGroupByUUID(NewGroup->m_uuid, Error); // Ищим существующую группу по UUID

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage.removeGroup(NewGroup->m_uuid);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    FindRes = Storage.findGroupByUUID(NewGroup->m_uuid, Error); // Ищим удалённую группу по UUID

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_TRUE(Error.value() == static_cast<int32_t>(hmservcommon::eDataStoragError::dsGroupNotExists)); // И метку, что группы не существует

    Storage.close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест удаления сообщения
 */
TEST(JsonDataStorage, RemoveMessage)
{
    std::error_code Error; // Метка ошибки
    clearStorage(); // Очищаем хранилище

    hmservcommon::HMJsonDataStorage Storage(C_JSON_PATH); // Создаём JSON хранилище с путём к директории
    Error = Storage.open(); // Пытаемся открыть хранилище

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroup> NewGroup = make_group();

    Error = Storage.addGroup(NewGroup); // добавляем группу сообщения
    ASSERT_FALSE(Error); // Ошибки быть не должно

    hmcommon::MsgData TextData(hmcommon::eMsgType::mtText, "Текст сообщения"); // Формируем данные сообщения
    std::shared_ptr<hmcommon::HMGroupMessage> NewMessage = make_groupmessage(TextData, QUuid::createUuid(), NewGroup->m_uuid); // Формируем сообщение

    Error = Storage.addMessage(NewMessage); // Добавляем сообщение
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<hmcommon::HMGroupMessage> FindRes = Storage.findMessage(NewMessage->m_uuid, Error);

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = Storage.removeMessage(NewMessage->m_uuid, NewMessage->m_group); // Удаляем сообщение
    ASSERT_FALSE(Error); // Ошибки быть не должно

    FindRes = Storage.findMessage(NewMessage->m_uuid, Error);

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_TRUE(Error.value() == static_cast<int32_t>(hmservcommon::eDataStoragError::dsMessageNotExists)); // И метку, что сообщение не существует

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

    hmservcommon::HMJsonDataStorage Storage(C_JSON_PATH); // Создаём JSON хранилище с путём к директории
    Error = Storage.open(); // Пытаемся открыть хранилище

    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Storage.is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUser> NewUser = make_user(); // Формируем пользователя
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
