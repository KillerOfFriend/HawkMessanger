#include <gtest/gtest.h>

#include <groupinfo.h>
#include <systemerrorex.h>

#include <HawkCommonTestUtils.hpp>

//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест создания группы
 */
TEST(GroupInfo, Create)
{
    QUuid GroupUuid = QUuid::createUuid();
    QDateTime CreateTime = QDateTime::currentDateTime();

    hmcommon::HMGroupInfo Group(GroupUuid, CreateTime);

    EXPECT_EQ(Group.m_uuid, GroupUuid);
    EXPECT_EQ(Group.m_registrationDate, CreateTime);
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест хранения параметров группы
 */
TEST(GroupInfo, CheckParams)
{
    hmcommon::error_code Error;

    const QString GroupName = "New_Group_Name";
    const std::size_t GroupUserCount = 5;
    std::array<std::shared_ptr<hmcommon::HMUserInfo>, GroupUserCount> Users;

    hmcommon::HMGroupInfo Group(QUuid::createUuid());

    Group.setName(GroupName);
    EXPECT_EQ(Group.getName(), GroupName);

//    EXPECT_EQ(Group.isUsersEmpty(), true);

//    for (std::size_t Index = 0; Index < GroupUserCount; ++Index)
//    {
//        Users[Index] = testscommon::make_user();
//        Error = Group.addUser(Users[Index]);
//        ASSERT_FALSE(Error); // Ошибки быть не должно
//        EXPECT_EQ(Group.usersCount(), Index + 1);
//    }

//    EXPECT_EQ(Group.isUsersEmpty(), false);

//    Error = Group.addUser(Users[0]);
//    ASSERT_TRUE(Error.value() == static_cast<int32_t>(hmcommon::eSystemErrorEx::seAlredyInContainer));

//    for (std::size_t Index = 0; Index < GroupUserCount; ++Index)
//    {
//        ASSERT_TRUE(Group.contain(Users[Index]->m_uuid));

//        std::shared_ptr<hmcommon::HMUserInfo> User = Group.getUser(Index, Error);
//        ASSERT_FALSE(Error); // Ошибки быть не должно
//        EXPECT_EQ(User, Users[Index]); // И UUID пользователя должен совпадать
//    }

//    for (std::size_t Index = 0; Index < GroupUserCount; ++Index)
//    {
//        Error = Group.removeUser(Users[Index]->m_uuid);
//        ASSERT_FALSE(Error); // Ошибки быть не должно

//        ASSERT_FALSE(Group.contain(Users[Index]->m_uuid));
//        EXPECT_EQ(Group.usersCount(), GroupUserCount - (Index + 1));
//    }

//    EXPECT_EQ(Group.isUsersEmpty(), true);
}
//-----------------------------------------------------------------------------
/**
 * @brief main - Входная точка тестировани функционала HMGroupInfo
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
