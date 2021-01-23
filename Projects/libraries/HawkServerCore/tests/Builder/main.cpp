#include <gtest/gtest.h>

#include <filesystem>

#include <builder.h>
#include <HawkLog.h>
#include <systemerrorex.h>
#include <datastorage/DataStorage.h>

#include <HawkCommonTestUtils.hpp>

using namespace hmservcommon::datastorage;

//-----------------------------------------------------------------------------
const std::filesystem::path C_JSON_PATH = std::filesystem::current_path() / "DataStorage.json";
//-----------------------------------------------------------------------------
std::shared_ptr<HMDataStorage> make_storage()
{
    std::error_code Error; // Метка ошибки

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
        CacheStorage = std::make_shared<HMCachedMemoryDataStorage>(); // Формируем хранилище HMCachedMemoryDataStorage
    }
    //---

    return std::make_shared<HMCombinedDataStorage>(HardStorage, CacheStorage);
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmservcommon::HMBuilder> make_builder(const std::shared_ptr<HMDataStorage> inStorage = make_storage())
{
    return std::make_shared<hmservcommon::HMBuilder>(inStorage);
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест сбора группы
 */
TEST(Builder, BuildGroup)
{
    std::error_code Error; // Метка ошибки
    std::shared_ptr<HMDataStorage> Storage = make_storage(); // Формируем хранилище данных

    Error = Storage->open(); // Пытаемся открыть хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<hmservcommon::HMBuilder> Builder = make_builder(Storage); // Создаём билдер

    QUuid GroupUUID = QUuid::createUuid(); // Создаём UUID группы
    std::shared_ptr<hmcommon::HMGroup> BuildGroup = Builder->buildGroup(GroupUUID, Error); // Пытаемся собрать группу не существующую в хранилище
    ASSERT_TRUE(Error.value() == static_cast<int32_t>(eDataStorageError::dsGroupNotExists)); // Должны получить сообщение, о том, что группа не существует в хранилище
    ASSERT_EQ(BuildGroup, nullptr); // Должен вернуться nullptr

    // Формируем данные в хранилище
    const std::size_t GroupUserCount = 5; // Количество пользователей в группе

    std::shared_ptr<hmcommon::HMGroup> NewGroup = std::make_shared<hmcommon::HMGroup>();
    NewGroup->m_info = testscommon::make_group_info(GroupUUID); // Формируем информацию о пользователе

    Error = Storage->addGroup(NewGroup->m_info); // Добавляем группу в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    for (std::size_t UserIndex = 0; UserIndex < GroupUserCount; ++UserIndex)
    {
        QString UserLogin = "TestUser" + QString::number(UserIndex); // У каждого нового пользователя должен быть уникальный логин
        std::shared_ptr<hmcommon::HMUserInfo> NewGroupUser = testscommon::make_user_info(QUuid::createUuid(), UserLogin);

        Error = NewGroup->m_users.add(NewGroupUser); // Добавляем пользователья в группу
        ASSERT_FALSE(Error); // Ошибки быть не должно

        Error = Storage->addUser(NewGroupUser); // Добавляеяем пользователья в хранилище
        ASSERT_FALSE(Error); // Ошибки быть не должно

        Storage->addGroupUser(NewGroup->m_info->m_uuid, NewGroupUser->m_uuid); // Добавляем пользователя в группу (в хранилище)
    }
    // Данные в хранилище сформированы

    BuildGroup = Builder->buildGroup(GroupUUID, Error); // Пытаемся собрать группу из данных в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(BuildGroup, nullptr); // Должен вернуться валидный указатель
    ASSERT_NE(BuildGroup->m_info, nullptr); // Должен вернуться валидный внутренний указатель

    // Тепреь сравниваем то что было изначально с тем, что собралось из хранилища
    EXPECT_EQ(*NewGroup->m_info, *BuildGroup->m_info); // Информация о группе полученная из хранилища должна совпасть с исходной
    ASSERT_EQ(NewGroup->m_users.count(), BuildGroup->m_users.count()); // Количество пользователей должно совпасть

    for (std::size_t UserIndex = 0; UserIndex < GroupUserCount; ++UserIndex) // Сравниваем всех участников группы
    {
        std::shared_ptr<hmcommon::HMUserInfo> UserGroup1 = NewGroup->m_users.get(UserIndex, Error); // Получаем пользователя первой группы
        ASSERT_FALSE(Error); // Ошибки быть не должно
        ASSERT_NE(UserGroup1, nullptr); // Должен вернуться валидный указатель

        std::shared_ptr<hmcommon::HMUserInfo> UserGroup2 = BuildGroup->m_users.get(UserGroup1->m_uuid, Error); // Получаем пользователя второй группы (ВАЖНО! Получени должно быть по UUID)
        ASSERT_FALSE(Error); // Ошибки быть не должно
        ASSERT_NE(UserGroup2, nullptr); // Должен вернуться валидный указатель

        EXPECT_EQ(*UserGroup1, *UserGroup2); // Сравниваем пользователей
    }

    Builder = nullptr;
    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест сбора пользователя
 */
TEST(Builder, BuildUser)
{
    std::error_code Error; // Метка ошибки
    std::shared_ptr<HMDataStorage> Storage = make_storage(); // Формируем хранилище данных

    Error = Storage->open(); // Пытаемся открыть хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<hmservcommon::HMBuilder> Builder = make_builder(Storage); // Создаём билдер

    QUuid UserUUID = QUuid::createUuid(); // Создаём UUID пользователя
    std::shared_ptr<hmcommon::HMUser> BuildUser = Builder->buildUser(UserUUID, Error); // Пытаемся собрать пользователя не существующего в хранилище
    ASSERT_TRUE(Error.value() == static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // Должны получить сообщение, о том, что пользователь не существует в хранилище
    ASSERT_EQ(BuildUser, nullptr); // Должен вернуться nullptr
    
    std::shared_ptr<hmcommon::HMUser> NewUser = std::make_shared<hmcommon::HMUser>(); // Формируем нового пользователя
    NewUser->m_info = testscommon::make_user_info(UserUUID);

    Error = Storage->addUser(NewUser->m_info); // Добавляем пользователя в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    // Формируем данные в хранилище
    const std::size_t UserContactCount = 10; // Количество контактов пользователя
    const std::size_t UserGrouptCount = 5; // Количество групп пользователя

    for (std::size_t ContactIndex = 0; ContactIndex < UserContactCount; ++ContactIndex)
    {   // Формируем перечень контактов пользователей
        QString ContactLogin = "TestContact" + QString::number(ContactIndex); // У каждого нового пользователя должен быть уникальный логин
        std::shared_ptr<hmcommon::HMUserInfo> NewContact = testscommon::make_user_info(QUuid::createUuid(), ContactLogin);

        Error = NewUser->m_contacts.add(NewContact); // Добавляем контакт пользователю
        ASSERT_FALSE(Error); // Ошибки быть не должно

        Error = Storage->addUser(NewContact); // Добавляеяем пользователья в хранилище
        ASSERT_FALSE(Error); // Ошибки быть не должно

        Error = Storage->addUserContact(NewUser->m_info->m_uuid, NewContact->m_uuid); // Добавляем пользователя в контакты (в хранилище)
        ASSERT_FALSE(Error); // Ошибки быть не должно
    }

    for (std::size_t GroupIndex = 0; GroupIndex < UserGrouptCount; ++GroupIndex)
    {   // Формируем перечень групп, в которые входят пользователи
        QString GroupName = "TestFroup" + QString::number(GroupIndex);
        std::shared_ptr<hmcommon::HMGroup> NewGroup = std::make_shared<hmcommon::HMGroup>();
        NewGroup->m_info = testscommon::make_group_info(QUuid::createUuid(), GroupName);

        Error = NewGroup->m_users.add(NewUser->m_info); // Добавляем пользователя в группу
        ASSERT_FALSE(Error); // Ошибки быть не должно

        Error = Storage->addGroup(NewGroup->m_info); // Добавляем группу (в хранилище)
        ASSERT_FALSE(Error); // Ошибки быть не должно

        Error = NewUser->m_groups.add(NewGroup); // Привязываем пользователя к группе
        ASSERT_FALSE(Error); // Ошибки быть не должно

        Error = Storage->addGroupUser(NewGroup->m_info->m_uuid, NewUser->m_info->m_uuid); // Привязываем пользователя к группе (в хранилище)
        ASSERT_FALSE(Error); // Ошибки быть не должно
    }
    // Данные в хранилище сформированы

    BuildUser = Builder->buildUser(UserUUID, Error); // Пытаемся собрать пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(BuildUser, nullptr); // Должен вернуться валидный указатель
    ASSERT_NE(BuildUser->m_info, nullptr); // Должен вернуться валидный внутренний указатель

    // Тепреь сравниваем то что было изначально с тем, что собралось из хранилища
    EXPECT_EQ(*NewUser->m_info, *BuildUser->m_info); // Информация о пользователе полученная из хранилища должна совпасть с исходной
    ASSERT_EQ(NewUser->m_contacts.count(), BuildUser->m_contacts.count()); // Количество контактов должно совпасть
    ASSERT_EQ(NewUser->m_groups.count(), BuildUser->m_groups.count()); // Количество групп должно совпасть

    for (std::size_t ContactIndex = 0; ContactIndex < UserContactCount; ++ContactIndex) // Сравниваем все контакты пользователя
    {
        std::shared_ptr<hmcommon::HMUserInfo> UserContact1 = NewUser->m_contacts.get(ContactIndex, Error); // Получаем контакт исходного пользователя
        ASSERT_FALSE(Error); // Ошибки быть не должно
        ASSERT_NE(UserContact1, nullptr); // Должен вернуться валидный указатель

        std::shared_ptr<hmcommon::HMUserInfo> UserContact2 = BuildUser->m_contacts.get(UserContact1->m_uuid, Error); // Получаем контакт собранного пользователя (ВАЖНО! Получени должно быть по UUID)
        ASSERT_FALSE(Error); // Ошибки быть не должно
        ASSERT_NE(UserContact2, nullptr); // Должен вернуться валидный указатель

        EXPECT_EQ(*UserContact1, *UserContact2); // Сравниваем контакты
    }

    for (std::size_t GroupIndex = 0; GroupIndex < UserGrouptCount; ++GroupIndex) // Сравниваем все группы пользователя
    {
        std::shared_ptr<hmcommon::HMGroup> UserGroup1 = NewUser->m_groups.get(GroupIndex, Error); // Получаем группу исходного пользователя
        ASSERT_FALSE(Error); // Ошибки быть не должно
        ASSERT_NE(UserGroup1, nullptr); // Должен вернуться валидный указатель
        ASSERT_NE(UserGroup1->m_info, nullptr); // Должен вернуться валидный внутренний указатель

        std::shared_ptr<hmcommon::HMGroup> UserGroup2 = BuildUser->m_groups.get(UserGroup1->m_info->m_uuid, Error); // Получаем группу собранного пользователя (ВАЖНО! Получени должно быть по UUID)
        ASSERT_FALSE(Error); // Ошибки быть не должно
        ASSERT_NE(UserGroup2, nullptr); // Должен вернуться валидный указатель
        ASSERT_NE(UserGroup2->m_info, nullptr); // Должен вернуться валидный внутренний указатель

        EXPECT_EQ(*UserGroup1->m_info, *UserGroup2->m_info); // Сравниваем контакты
        // Количество пользователей каждой группы должно равнятся 1
        ASSERT_EQ(UserGroup1->m_users.count(), 1);
        ASSERT_EQ(UserGroup2->m_users.count(), 1);

        std::shared_ptr<hmcommon::HMUserInfo> Group1User = UserGroup1->m_users.get(0, Error); // Запрашиваем единственного пользователя группы
        ASSERT_FALSE(Error); // Ошибки быть не должно
        ASSERT_NE(Group1User, nullptr); // Должен вернуться валидный указатель

        std::shared_ptr<hmcommon::HMUserInfo> Group2User = UserGroup1->m_users.get(0, Error); // Запрашиваем единственного пользователя группы
        ASSERT_FALSE(Error); // Ошибки быть не должно
        ASSERT_NE(Group2User, nullptr); // Должен вернуться валидный указатель

        EXPECT_EQ(*Group1User, *Group2User); // Пользователи исходной и собранной должны совпадать
        // Единственный пользователь должен быть исходным пользователем
        EXPECT_EQ(*Group1User, *NewUser->m_info);
        EXPECT_EQ(*Group2User, *NewUser->m_info);
    }

    Builder = nullptr;
    Storage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief main - Входная точка тестировани функционала HMBuilder
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
