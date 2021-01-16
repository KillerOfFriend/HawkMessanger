#include <gtest/gtest.h>

#include <userlist.h>
#include <systemerrorex.h>

#include <HawkCommonTestUtils.hpp>

//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест создания списка контактов
 */
TEST(UsertList, Create)
{
    hmcommon::HMUserInfoList UsertList;

    EXPECT_TRUE(UsertList.isEmpty());
    EXPECT_EQ(UsertList.count(), 0);
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест добавления контакта
 */
TEST(UsertList, CheckAddContact)
{
    std::error_code Error; // Метка ошибки
    hmcommon::HMUserInfoList UsertList; // Список контактов

    std::shared_ptr<hmcommon::HMUserInfo> NewContact = testscommon::make_user_info();

    EXPECT_TRUE(UsertList.isEmpty());
    EXPECT_EQ(UsertList.count(), 0);

    Error = UsertList.add(NewContact); // Добавляем новый контакт
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_FALSE(UsertList.isEmpty());
    EXPECT_EQ(UsertList.count(), 1);
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест поиска не существующего контакта
 */
TEST(UsertList, FindNotExistsContact)
{
    std::error_code Error; // Метка ошибки
    hmcommon::HMUserInfoList ContactList; // Список контактов

    std::shared_ptr<hmcommon::HMUserInfo> NewContact = testscommon::make_user_info();

    std::shared_ptr<hmcommon::HMUserInfo> FindRes = ContactList.get(0, Error);

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_TRUE(Error.value() == static_cast<int32_t>(hmcommon::eSystemErrorEx::seContainerEmpty)); // И метку, что контейнер пуст

    FindRes = ContactList.get(NewContact->m_uuid, Error);

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_TRUE(Error.value() == static_cast<int32_t>(hmcommon::eSystemErrorEx::seNotInContainer)); // И метку, что контакт в контейнере не найден
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест поиска существующего контакта
 */
TEST(UsertList, FindExistsContact)
{
    std::error_code Error; // Метка ошибки
    hmcommon::HMUserInfoList ContactList; // Список контактов

    std::shared_ptr<hmcommon::HMUserInfo> NewContact = testscommon::make_user_info();

    Error = ContactList.add(NewContact); // Добавляем новый контакт
    ASSERT_FALSE(Error); // Ошибки быть не должно;

    std::shared_ptr<hmcommon::HMUserInfo> FindRes = ContactList.get(0, Error);

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указаетль
    ASSERT_FALSE(Error); // Ошибки быть не должно

    FindRes = ContactList.get(NewContact->m_uuid, Error);

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указаетль
    ASSERT_FALSE(Error); // Ошибки быть не должно
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест удаление контакта
 */
TEST(UsertList, CheckRemoveContact)
{
    std::error_code Error; // Метка ошибки
    hmcommon::HMUserInfoList UsertList; // Список контактов

    std::shared_ptr<hmcommon::HMUserInfo> NewContact1 = testscommon::make_user_info();
    std::shared_ptr<hmcommon::HMUserInfo> NewContact2 = testscommon::make_user_info();

    Error = UsertList.add(NewContact1); // Добавляем новый контакт
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = UsertList.add(NewContact2); // Добавляем новый контакт
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_FALSE(UsertList.isEmpty());
    EXPECT_EQ(UsertList.count(), 2);

    Error = UsertList.remove(NewContact1->m_uuid); // Первого удалим по UUID
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = UsertList.remove(0); // Втогого по порядковому номеру (После удаления первого Index == 0)
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_TRUE(UsertList.isEmpty());
    EXPECT_EQ(UsertList.count(), 0);
}
//-----------------------------------------------------------------------------
/**
 * @brief main - Входная точка тестировани функционала HMUserInfoList
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
