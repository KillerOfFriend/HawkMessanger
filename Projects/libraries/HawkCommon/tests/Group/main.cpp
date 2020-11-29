#include <gtest/gtest.h>

#include <group.h>
#include <systemerrorex.h>

//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест создания группы
 */
TEST(Group, Create)
{
    QUuid GroupUuid = QUuid::createUuid();
    QDateTime CreateTime = QDateTime::currentDateTime();

    hmcommon::HMGroup Group(GroupUuid, CreateTime);

    EXPECT_EQ(Group.m_uuid, GroupUuid);
    EXPECT_EQ(Group.m_registrationDate, CreateTime);
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест хранения параметров группы
 */
TEST(Group, CheckParams)
{
    std::error_code Error;

    const QString GroupName = "New_Group_Name";
    const std::size_t GroupUserCount = 5;
    std::array<QUuid, GroupUserCount> Users;

    hmcommon::HMGroup Group(QUuid::createUuid());

    Group.setName(GroupName);
    EXPECT_EQ(Group.getName(), GroupName);

    EXPECT_EQ(Group.isUsersEnpty(), true);

    for (std::size_t Index = 0; Index < GroupUserCount; ++Index)
    {
        Users[Index] = QUuid::createUuid();
        Error = Group.addUser(Users[Index]);
        ASSERT_FALSE(Error); // Ошибки быть не должно
        EXPECT_EQ(Group.usersCount(), Index + 1);
    }

    EXPECT_EQ(Group.isUsersEnpty(), false);

    Error = Group.addUser(Users[0]);
    ASSERT_TRUE(Error.value() == static_cast<int32_t>(hmcommon::eSystemErrorEx::seAlredyInContainer));

    for (std::size_t Index = 0; Index < GroupUserCount; ++Index)
    {
        ASSERT_TRUE(Group.contain(Users[Index]));

        QUuid User = Group.getUser(Index, Error);
        ASSERT_FALSE(Error); // Ошибки быть не должно
        EXPECT_EQ(User, Users[Index]); // И UUID пользователя должен совпадать
    }

    for (std::size_t Index = 0; Index < GroupUserCount; ++Index)
    {
        Error = Group.removeUser(Users[Index]);
        ASSERT_FALSE(Error); // Ошибки быть не должно

        ASSERT_FALSE(Group.contain(Users[Index]));
        EXPECT_EQ(Group.usersCount(), GroupUserCount - (Index + 1));
    }

    EXPECT_EQ(Group.isUsersEnpty(), true);
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
