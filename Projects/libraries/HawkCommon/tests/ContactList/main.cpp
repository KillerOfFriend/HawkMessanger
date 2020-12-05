#include <gtest/gtest.h>

#include <user.h>
#include <contactlist.h>
#include <systemerrorex.h>

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
 * @brief TEST - Тест создания списка контактов
 */
TEST(ContactList, Create)
{
    hmcommon::HMContactList ContactList;

    EXPECT_TRUE(ContactList.isEmpty());
    EXPECT_EQ(ContactList.contactsCount(), 0);
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест добавления контакта
 */
TEST(ContactList, CheckAddContact)
{
    std::error_code Error; // Метка ошибки
    hmcommon::HMContactList ContactList; // Список контактов

    std::shared_ptr<hmcommon::HMUser> NewContact = make_user();

    EXPECT_TRUE(ContactList.isEmpty());
    EXPECT_EQ(ContactList.contactsCount(), 0);

    Error = ContactList.addContact(NewContact); // Добавляем новый контакт
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_FALSE(ContactList.isEmpty());
    EXPECT_EQ(ContactList.contactsCount(), 1);
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест поиска не существующего контакта
 */
TEST(ContactList, FindNotExistsContact)
{
    std::error_code Error; // Метка ошибки
    hmcommon::HMContactList ContactList; // Список контактов

    std::shared_ptr<hmcommon::HMUser> NewContact = make_user();

    std::shared_ptr<hmcommon::HMUser> FindRes = ContactList.getContact(0, Error);

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_TRUE(Error.value() == static_cast<int32_t>(hmcommon::eSystemErrorEx::seContainerEmpty)); // И метку, что контейнер пуст

    FindRes = ContactList.getContact(NewContact->m_uuid, Error);

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_TRUE(Error.value() == static_cast<int32_t>(hmcommon::eSystemErrorEx::seNotInContainer)); // И метку, что контакт в контейнере не найден
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест поиска существующего контакта
 */
TEST(ContactList, FindExistsContact)
{
    std::error_code Error; // Метка ошибки
    hmcommon::HMContactList ContactList; // Список контактов

    std::shared_ptr<hmcommon::HMUser> NewContact = make_user();

    Error = ContactList.addContact(NewContact); // Добавляем новый контакт
    ASSERT_FALSE(Error); // Ошибки быть не должно;

    std::shared_ptr<hmcommon::HMUser> FindRes = ContactList.getContact(0, Error);

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указаетль
    ASSERT_FALSE(Error); // Ошибки быть не должно

    FindRes = ContactList.getContact(NewContact->m_uuid, Error);

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указаетль
    ASSERT_FALSE(Error); // Ошибки быть не должно
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест удаление контакта
 */
TEST(ContactList, CheckRemoveContact)
{
    std::error_code Error; // Метка ошибки
    hmcommon::HMContactList ContactList; // Список контактов

    std::shared_ptr<hmcommon::HMUser> NewContact1 = make_user();
    std::shared_ptr<hmcommon::HMUser> NewContact2 = make_user();

    Error = ContactList.addContact(NewContact1); // Добавляем новый контакт
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = ContactList.addContact(NewContact2); // Добавляем новый контакт
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_FALSE(ContactList.isEmpty());
    EXPECT_EQ(ContactList.contactsCount(), 2);

    Error = ContactList.removeContact(NewContact1->m_uuid); // Первого удалим по UUID
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = ContactList.removeContact(0); // Втогого по порядковому номеру (После удаления первого Index == 0)
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_TRUE(ContactList.isEmpty());
    EXPECT_EQ(ContactList.contactsCount(), 0);
}
//-----------------------------------------------------------------------------
/**
 * @brief main - Входная точка тестировани функционала HMContactList
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
