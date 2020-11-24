#include <gtest/gtest.h>

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
std::shared_ptr<hmcommon::HMUser> make_user()
{
    const QUuid UserUuid = QUuid::createUuid();       // Формируем новый UUID
    const QString UserLogin = "UserLogin@login.com";  // Формируем логин пользователя
    const QString UserPassword = "P@ssworOfUser123";  // Формируем пароль пользователя
    const QDateTime CreateDate = QDateTime(QDate(), QTime::currentTime()); //QDateTime::fromString(QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss:SSS")); // !!!

    // Формируем пользователя, которого нет в хранилище
    std::shared_ptr<hmcommon::HMUser> NewUser = std::make_shared<hmcommon::HMUser>(UserUuid, CreateDate);
    // Задаём основные параметры
    NewUser->setLogin(UserLogin);
    NewUser->setPassword(UserPassword);

    return NewUser;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMGroup> make_group()
{
    const QUuid GroupUuid = QUuid::createUuid();        // Формируем новый UUID
    const QString GroupName = "New group name";         // Имя группы
    const QDateTime CreateDate = QDateTime(QDate(), QTime::currentTime()); // QDateTime::fromString(QDateTime::currentDateTime().toString("dd.MM.yyyy HH:mm:ss:SSS")); // !!!

    // Формируем новую группу
    std::shared_ptr<hmcommon::HMGroup> NewGroup = std::make_shared<hmcommon::HMGroup>(GroupUuid, CreateDate);
    NewGroup->setName(GroupName); // Задаём имя группы

    return NewGroup;
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
