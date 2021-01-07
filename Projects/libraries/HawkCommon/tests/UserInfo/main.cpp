#include <gtest/gtest.h>

#include <userinfo.h>

#include <QCryptographicHash>

//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест создания пользователя
 */
TEST(UserInfo, Create)
{
    QUuid UserUuid = QUuid::createUuid();
    QDateTime CreateTime = QDateTime::currentDateTime();

    hmcommon::HMUserInfo User(UserUuid, CreateTime);

    EXPECT_EQ(User.m_uuid, UserUuid);
    EXPECT_EQ(User.m_registrationDate, CreateTime);
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест хранения параметров пользователя
 */
TEST(UserInfo, CheckParams)
{
    const QString UserLogin = "UserLogin@random.net";
    const QString UserName = "New User Name";
    const QString UserPassword = "User_P@$$word";
    const QDate UserBirthday = QDate::currentDate();

    hmcommon::HMUserInfo User(QUuid::createUuid());

    User.setLogin(UserLogin);
    EXPECT_EQ(User.getLogin(), UserLogin);

    User.setName(UserName);
    EXPECT_EQ(User.getName(), UserName);

    EXPECT_EQ(User.setPassword(QString()), false);

    QByteArray PassHash = QCryptographicHash::hash(UserPassword.toLocal8Bit(), QCryptographicHash::Algorithm::Sha512);
    User.setPasswordHash(PassHash);
    EXPECT_EQ(User.getPasswordHash(), PassHash);

    User.setPassword(UserPassword);
    EXPECT_EQ(User.getPasswordHash(), PassHash);

    User.setBirthday(UserBirthday);
    EXPECT_EQ(User.getBirthday(), UserBirthday);

    User.setSex(hmcommon::eSex::sMale);
    EXPECT_EQ(User.getSex(), hmcommon::eSex::sMale);
    User.setSex(hmcommon::eSex::sFemale);
    EXPECT_EQ(User.getSex(), hmcommon::eSex::sFemale);
    User.setSex(hmcommon::eSex::sNotSpecified);
    EXPECT_EQ(User.getSex(), hmcommon::eSex::sNotSpecified);
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
