#ifndef HAWKSERVERCOREHARDDATASTORAGETEST_HPP
#define HAWKSERVERCOREHARDDATASTORAGETEST_HPP

#include <thread>
#include <memory>
#include <systemerrorex.h>

#include <gtest/gtest.h>

#include <HawkCommonTestUtils.hpp>

#include <datastorage/datastorageerrorcategory.h>
#include <datastorage/interface/datastorageinterface.h>

//-----------------------------------------------------------------------------
using namespace hmservcommon::datastorage;
//-----------------------------------------------------------------------------
/**
 * @brief HardDataStorage_AddUserTest - Тест физического хранилища, проверяющий добавление пользователя
 * @param inHardDataStorage - Тестируемое физическое хранилище
 */
void HardDataStorage_AddUserTest(std::unique_ptr<HMDataStorage> inHardDataStorage)
{
    std::error_code Error;

    Error = inHardDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inHardDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUserInfo> NewUser = testscommon::make_user_info();

    Error = inHardDataStorage->addUser(NewUser); // Пытаемся добавить пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inHardDataStorage->addUser(NewUser); // Пытаемся добавить повторно
    EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserAlreadyExists)); // Должны получить сообщение о том, что пользователь с таким UUID уже зарегистрирован

    NewUser = testscommon::make_user_info(QUuid::createUuid()); // Формируем такого же пользователя но с другим UUID

    Error = inHardDataStorage->addUser(NewUser); // Пытаемся добавить с новым UUID
    EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserLoginAlreadyRegistered)); // Должны получить сообщение о том, что этот логин уже занят

    NewUser = testscommon::make_user_info(QUuid::createUuid(), "OtherUser@login.com"); // Формируем с другим UUID и логином

    Error = inHardDataStorage->addUser(NewUser); // Пытаемся добавить с другим UUID и логином
    ASSERT_FALSE(Error); // Ошибки быть не должно

    inHardDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief HardDataStorage_UpdateUserTest - Тест физического хранилища, проверяющий обновление пользователя
 * @param inHardDataStorage - Тестируемое физическое хранилище
 */
void HardDataStorage_UpdateUserTest(std::unique_ptr<HMDataStorage> inHardDataStorage)
{
    std::error_code Error;

    Error = inHardDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inHardDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUserInfo> NewUser = testscommon::make_user_info();

    Error = inHardDataStorage->addUser(NewUser); // Пытаемся добавить пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    NewUser->setName("New User Name");

    Error = inHardDataStorage->updateUser(NewUser); // Пытаемся обновить пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<hmcommon::HMUserInfo> FindRes = inHardDataStorage->findUserByUUID(NewUser->m_uuid, Error);
    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно
    EXPECT_EQ(NewUser->getName(), FindRes->getName()); // Имя должно измениться

    NewUser = testscommon::make_user_info(); // Формируем нового пользователя

    Error = inHardDataStorage->updateUser(NewUser); // Пытаемся обновить пользователя, не добавленного в хранилище
    EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // Должны получить сообщение о том, что нет такого пользователя

    inHardDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief HardDataStorage_FindUserByUUIDTest - Тест физического хранилища, проверяющий поиск пользователя по UUID
 * @param inHardDataStorage - Тестируемое физическое хранилище
 */
void HardDataStorage_FindUserByUUIDTest(std::unique_ptr<HMDataStorage> inHardDataStorage)
{
    std::error_code Error;

    Error = inHardDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inHardDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUserInfo> NewUser = testscommon::make_user_info();

    std::shared_ptr<hmcommon::HMUserInfo> FindRes = inHardDataStorage->findUserByUUID(NewUser->m_uuid, Error); // Попытка получить не существующего пользователя

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // И метку, что пользователь не найден в хранилище

    Error = inHardDataStorage->addUser(NewUser); // Пытаемся добавить пользователя в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    const size_t SilCount = 5; // Добавим некоторое количество левых пользователей
    for (size_t Index = 0; Index < SilCount; ++Index)
    {
        QString TrashUserLogin = "TrashUser" + QString::number(Index); // Логины мусорных пользователей должны быть уникальными
        Error = inHardDataStorage->addUser(testscommon::make_user_info(QUuid::createUuid(), TrashUserLogin)); // Пытаемся добавить пользователя
        ASSERT_FALSE(Error); // Ошибки быть не должно
    }

    FindRes = inHardDataStorage->findUserByUUID(NewUser->m_uuid, Error);

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_EQ(*NewUser, *FindRes); // Полное сравнение объектов должно пройти успешно

    inHardDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief HardDataStorage_FindUserByAuthenticationTest - Тест физического хранилища, проверяющий поиск пользователя по данным аутентификации
 * @param inHardDataStorage - Тестируемое физическое хранилище
 */
void HardDataStorage_FindUserByAuthenticationTest(std::unique_ptr<HMDataStorage> inHardDataStorage)
{
    std::error_code Error;

    Error = inHardDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inHardDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUserInfo> NewUser = testscommon::make_user_info();

    std::shared_ptr<hmcommon::HMUserInfo> FindRes = inHardDataStorage->findUserByAuthentication(NewUser->getLogin(), NewUser->getPasswordHash(), Error); // Попытка получить не существующего пользователя

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // И метку, что пользователь не добавлен в хранилище

    Error = inHardDataStorage->addUser(NewUser); // Пытаемся добавить пользователя в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    const size_t SilCount = 5; // Добавим некоторое количество левых пользователей
    for (size_t Index = 0; Index < SilCount; ++Index)
    {
        QString TrashUserLogin = "TrashUser" + QString::number(Index); // Логины мусорных пользователей должны быть уникальными
        Error = inHardDataStorage->addUser(testscommon::make_user_info(QUuid::createUuid(), TrashUserLogin)); // Пытаемся добавить пользователя
        ASSERT_FALSE(Error); // Ошибки быть не должно
    }

    FindRes = inHardDataStorage->findUserByAuthentication(NewUser->getLogin(), NewUser->getPasswordHash(), Error); // Попытка получить не существующего пользователя

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_EQ(*NewUser, *FindRes); // Полное сравнение объектов должно пройти успешно

    inHardDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief HardDataStorage_RemoveUserTest - Тест физического хранилища, проверяющий удаление пользователя
 * @param inHardDataStorage - Тестируемое физическое хранилище
 */
void HardDataStorage_RemoveUserTest(std::unique_ptr<HMDataStorage> inHardDataStorage)
{
    std::error_code Error;

    Error = inHardDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inHardDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUserInfo> NewUser = testscommon::make_user_info();

    Error = inHardDataStorage->addUser(NewUser); // Пытаемся добавить пользователя в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inHardDataStorage->removeUser(NewUser->m_uuid); // Пытаемся удалить добавленного пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<hmcommon::HMUserInfo> FindRes = inHardDataStorage->findUserByUUID(NewUser->m_uuid, Error); // Попытка получить удалённого пользователя

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // И метку, что пользователь не найден в хранилище

    Error = inHardDataStorage->removeUser(NewUser->m_uuid); // Пытаемся удалить не существующего пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    inHardDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief HardDataStorage_SetUserContactsTest - Тест физического хранилища, проверяющий присвоение контактов пользователю
 * @param inHardDataStorage - Тестируемое физическое хранилище
 */
void HardDataStorage_SetUserContactsTest(std::unique_ptr<HMDataStorage> inHardDataStorage)
{
    std::error_code Error;

    Error = inHardDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inHardDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUserInfo> NewUser = testscommon::make_user_info(QUuid::createUuid(), "TestUser@login.com");
    std::shared_ptr<hmcommon::HMUserInfo> NewContact1 = testscommon::make_user_info(QUuid::createUuid(), "TestContact1@login.com");
    std::shared_ptr<hmcommon::HMUserInfo> NewContact2 = testscommon::make_user_info(QUuid::createUuid(), "TestContact2@login.com");

    std::shared_ptr<std::set<QUuid>> NewContactList = std::make_shared<std::set<QUuid>>();

    NewContactList->insert(NewContact1->m_uuid);
    NewContactList->insert(NewContact2->m_uuid);

    Error = inHardDataStorage->setUserContacts(NewUser->m_uuid, NewContactList); // Пытаемся добавить список контактов без добавления пользователя
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // Должны получить сообщение о том, что не существует пользователя, к которому нужно привязать список

    Error = inHardDataStorage->addUser(NewUser); // Теперь добавим пользователя в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    inHardDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief HardDataStorage_AddUserContactTest - Тест физического хранилища, проверяющий добавление контакта пользователю
 * @param inHardDataStorage - Тестируемое физическое хранилище
 */
void HardDataStorage_AddUserContactTest(std::unique_ptr<HMDataStorage> inHardDataStorage)
{
    std::error_code Error;

    Error = inHardDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inHardDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUserInfo> NewUser = testscommon::make_user_info(QUuid::createUuid(), "TestUser@login.com");
    std::shared_ptr<hmcommon::HMUserInfo> NewContact = testscommon::make_user_info(QUuid::createUuid(), "TestContact@login.com");

    Error = inHardDataStorage->addUserContact(NewUser->m_uuid, NewContact->m_uuid); // Пытаемся добавить контакт без добавления пользователя
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // Должны получить сообщение о том, что пользователь (вледелец) не существует в хранилище

    Error = inHardDataStorage->addUser(NewUser); // Теперь добавим пользователя в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inHardDataStorage->addUserContact(NewUser->m_uuid, NewContact->m_uuid); // И теперь добавляем контакт
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // Должны получить сообщение о том, что пользователь (контакт) не существует в хранилище

    Error = inHardDataStorage->addUser(NewContact); // Теперь добавим пользователя, который будет выступать новым контактом
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inHardDataStorage->addUserContact(NewUser->m_uuid, NewContact->m_uuid); // И накнец штатно добавляем контакт пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно

    inHardDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief HardDataStorage_RemoveUserContactTest - Тест физического хранилища, проверяющий удаление контакта пользователю
 * @param inHardDataStorage - Тестируемое физическое хранилище
 */
void HardDataStorage_RemoveUserContactTest(std::unique_ptr<HMDataStorage> inHardDataStorage)
{
    std::error_code Error;

    Error = inHardDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inHardDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUserInfo> NewUser = testscommon::make_user_info(QUuid::createUuid(), "TestUser@login.com");
    std::shared_ptr<hmcommon::HMUserInfo> NewContact = testscommon::make_user_info(QUuid::createUuid(), "TestContact@login.com");

    Error = inHardDataStorage->removeUserContact(NewUser->m_uuid, NewContact->m_uuid); // Пытаемся удалить контакт без добавления пользователя
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // Должны получить сообщение о том, что пользователь (вледелец) не существует в хранилище

    Error = inHardDataStorage->addUser(NewUser); // Теперь добавим пользователя в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inHardDataStorage->removeUserContact(NewUser->m_uuid, NewContact->m_uuid); // И теперь добавляем контакт
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserContactNotExists)); // Должны получить сообщение о том, что у пользователя нет такого контакта

    Error = inHardDataStorage->addUser(NewContact); // Теперь добавим пользователя, который будет выступать новым контактом
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inHardDataStorage->addUserContact(NewUser->m_uuid, NewContact->m_uuid); // И добавляем контакт пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inHardDataStorage->removeUserContact(NewUser->m_uuid, NewContact->m_uuid); // И наконец штатно удаляем связь
    ASSERT_FALSE(Error); // Ошибки быть не должно

    inHardDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief HardDataStorage_ClearUserContactsTest - Тест физического хранилища, проверяющий очистку контактов пользователей
 * @param inHardDataStorage - Тестируемое физическое хранилище
 */
void HardDataStorage_ClearUserContactsTest(std::unique_ptr<HMDataStorage> inHardDataStorage)
{
    std::error_code Error;

    Error = inHardDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inHardDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUserInfo> NewUser = testscommon::make_user_info(QUuid::createUuid(), "TestUser@login.com");
    std::shared_ptr<hmcommon::HMUserInfo> NewContact1 = testscommon::make_user_info(QUuid::createUuid(), "TestContact1@login.com");
    std::shared_ptr<hmcommon::HMUserInfo> NewContact2 = testscommon::make_user_info(QUuid::createUuid(), "TestContact2@login.com");

    Error = inHardDataStorage->clearUserContacts(NewUser->m_uuid); // Пытаемся удалить не сущестующую связь
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // Должны получить сообщение о том, что пользователь в хранилище не найден

    Error = inHardDataStorage->addUser(NewUser);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inHardDataStorage->addUserContact(NewUser->m_uuid, NewUser->m_uuid); // Пытаемся добавить в список контактов самого себя
    ASSERT_EQ(Error.value(), static_cast<int32_t>(hmcommon::eSystemErrorEx::seIncorretData)); // Должны получить сообщение о том, что данные не корректны

    Error = inHardDataStorage->addUser(NewContact1);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inHardDataStorage->addUser(NewContact2);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inHardDataStorage->addUserContact(NewUser->m_uuid, NewContact1->m_uuid); // И добавляем контакт1 пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inHardDataStorage->addUserContact(NewUser->m_uuid, NewContact2->m_uuid); // И добавляем контакт2 пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inHardDataStorage->clearUserContacts(NewUser->m_uuid);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    inHardDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief HardDataStorage_GetUserContactListTest- Тест физического хранилища, проверяющий запрос списка контактов пользователей
 * @param inHardDataStorage - Тестируемое физическое хранилище
 */
void HardDataStorage_GetUserContactListTest(std::unique_ptr<HMDataStorage> inHardDataStorage)
{
    std::error_code Error;

    Error = inHardDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inHardDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUserInfo> NewUser = testscommon::make_user_info(QUuid::createUuid(), "TestUser@login.com");
    std::shared_ptr<hmcommon::HMUserInfo> NewContact1 = testscommon::make_user_info(QUuid::createUuid(), "TestContact1@login.com");
    std::shared_ptr<hmcommon::HMUserInfo> NewContact2 = testscommon::make_user_info(QUuid::createUuid(), "TestContact2@login.com");

    std::shared_ptr<std::set<QUuid>> FindRes = inHardDataStorage->getUserContactList(NewUser->m_uuid, Error); // Пытаемся получить список контактов не существующего пользователя
    ASSERT_EQ(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // Должны получить сообщение о том, что пользователь в хранилище не найден
    // Добавляем пользователей
    Error = inHardDataStorage->addUser(NewUser);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inHardDataStorage->addUser(NewContact1);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inHardDataStorage->addUser(NewContact2);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inHardDataStorage->addUserContact(NewUser->m_uuid, NewContact1->m_uuid); // И добавляем контакт1 пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inHardDataStorage->addUserContact(NewUser->m_uuid, NewContact2->m_uuid); // И добавляем контакт2 пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно
    // Начинаем проверять списки контактов
    FindRes = inHardDataStorage->getUserContactList(NewUser->m_uuid, Error); // Теперь пытваемся получить список контактов пользователя

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_NE(FindRes->find(NewContact1->m_uuid), FindRes->end()); // Должен содержать первый контакт
    EXPECT_NE(FindRes->find(NewContact2->m_uuid), FindRes->end()); // Должен содержать второй контакт

    FindRes = inHardDataStorage->getUserContactList(NewContact1->m_uuid, Error); // Проверяем список контактов второго пользователя

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_NE(FindRes->find(NewUser->m_uuid), FindRes->end()); // Должен содержать первый контакт

    FindRes = inHardDataStorage->getUserContactList(NewContact2->m_uuid, Error); // Проверяем список контактов первого пользователя

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_NE(FindRes->find(NewUser->m_uuid), FindRes->end()); // Должен содержать первый контакт

    inHardDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief HardDataStorage_GetUserGroupsTest - Тест физического хранилища, проверяющий запрос списка групп пользователей
 * @param inHardDataStorage - Тестируемое физическое хранилище
 */
void HardDataStorage_GetUserGroupsTest(std::unique_ptr<HMDataStorage> inHardDataStorage)
{
    std::error_code Error;

    Error = inHardDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inHardDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroupInfo> NewGroup = testscommon::make_group_info(); // Создаём группу
    Error = inHardDataStorage->addGroup(NewGroup); // Добавляем группу в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    const std::size_t UsersCount = 5; // Контейнер, хранащий пользователей
    std::shared_ptr<std::set<QUuid>> UserUUIDs = std::make_shared<std::set<QUuid>>(); // Контейнер UUID'ов пользователей
    std::shared_ptr<std::set<QUuid>> UserGroups = std::make_shared<std::set<QUuid>>(); // Перечень групп пользователя

    for (size_t Index = 0; Index < UsersCount; ++Index)
    {
        QString TestUserLogin = "TestUser" + QString::number(Index); // Логины тестовых пользователей должны быть уникальными
        std::shared_ptr<hmcommon::HMUserInfo> User = testscommon::make_user_info(QUuid::createUuid(), TestUserLogin);

        UserGroups = inHardDataStorage->getUserGroups(User->m_uuid, Error); // Запрашиваем перечень групп не существующего в хранилище пользователя
        ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // Получаем метку, что пользователь не найден в хранилище

        Error = inHardDataStorage->addUser(User); // Пытаемся добавить пользователя
        ASSERT_FALSE(Error); // Ошибки быть не должно

        UserGroups = inHardDataStorage->getUserGroups(User->m_uuid, Error); // Запрашиваем перечень групп добавленного в хранилище пользователя
        ASSERT_FALSE(Error); // Ошибки быть не должно
        ASSERT_NE(UserGroups, nullptr); // Должен вернуться валидный указаетль
        EXPECT_TRUE(UserGroups->empty()); // Перечень групп должен быть пустым

        UserUUIDs->insert(User->m_uuid); // Запоминаем UUID добавленого в хранилище пользователя
    }

    Error = inHardDataStorage->setGroupUsers(NewGroup->m_uuid, UserUUIDs); // Пытаемся добавить пользователей в группу
    ASSERT_FALSE(Error); // Ошибки быть не должно

    for (const QUuid& UserUUID : *UserUUIDs) // Перебираем всех добавленых в хранилище пользователей
    {
        UserGroups = inHardDataStorage->getUserGroups(UserUUID, Error); // Запрашиваем перечень групп пользователей
        ASSERT_FALSE(Error); // Ошибки быть не должно
        ASSERT_NE(UserGroups, nullptr); // Должен вернуться валидный указаетль
        EXPECT_NE(UserGroups->find(NewGroup->m_uuid), UserGroups->cend()); // Группа должна быть успешно найдена в списке групп пользователей
    }

    inHardDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief HardDataStorage_AddGroupTest - Тест физического хранилища, проверяющий добавление группы
 * @param inHardDataStorage - Тестируемое физическое хранилище
 */
void HardDataStorage_AddGroupTest(std::unique_ptr<HMDataStorage> inHardDataStorage)
{
    std::error_code Error;

    Error = inHardDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inHardDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroupInfo> NewGroup = testscommon::make_group_info();

    Error = inHardDataStorage->addGroup(NewGroup);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inHardDataStorage->addGroup(NewGroup);
    EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupUUIDAlreadyRegistered)); // Должны получить сообщение о том, что этот UUID уже зарегистрирован

    inHardDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief HardDataStorage_UpdateGroupTest - Тест физического хранилища, проверяющий обновление группы
 * @param inHardDataStorage - Тестируемое физическое хранилище
 */
void HardDataStorage_UpdateGroupTest(std::unique_ptr<HMDataStorage> inHardDataStorage)
{
    std::error_code Error;

    Error = inHardDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inHardDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroupInfo> NewGroup = testscommon::make_group_info();

    Error = inHardDataStorage->addGroup(NewGroup);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    NewGroup->setName("New Group Name");

    Error = inHardDataStorage->updateGroup(NewGroup);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<hmcommon::HMGroupInfo> FindRes = inHardDataStorage->findGroupByUUID(NewGroup->m_uuid, Error);
    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно
    EXPECT_EQ(NewGroup->getName(), FindRes->getName()); // Имя должно измениться

    NewGroup = testscommon::make_group_info(); // Формируем новую группу

    Error = inHardDataStorage->updateGroup(NewGroup); // Пытаемся обновить группу, не добавленную в хранилище
    EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupNotExists)); // Должны получить сообщение о том, что нет такой группы

    inHardDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief HardDataStorage_FindGroupByUUIDTest - Тест физического хранилища, проверяющий поиск группы
 * @param inHardDataStorage - Тестируемое физическое хранилище
 */
void HardDataStorage_FindGroupByUUIDTest(std::unique_ptr<HMDataStorage> inHardDataStorage)
{
    std::error_code Error;

    Error = inHardDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inHardDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroupInfo> NewGroup = testscommon::make_group_info();

    std::shared_ptr<hmcommon::HMGroupInfo> FindRes = inHardDataStorage->findGroupByUUID(NewGroup->m_uuid, Error); // Попытка получить не существующую группу

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupNotExists)); // И метку, что группа не найдена в хранилище

    Error = inHardDataStorage->addGroup(NewGroup); // Пытаемся добавить группу в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    const size_t SilCount = 5; // Добавим некоторое количество левых групп
    for (size_t Index = 0; Index < SilCount; ++Index)
    {
        QString TrashGroupName = "TrashGroup" + QString::number(Index); // Логины мусорных пользователей должны быть уникальными
        Error = inHardDataStorage->addGroup(testscommon::make_group_info(QUuid::createUuid(), TrashGroupName)); // Пытаемся добавить группу
        ASSERT_FALSE(Error); // Ошибки быть не должно
    }

    FindRes = inHardDataStorage->findGroupByUUID(NewGroup->m_uuid, Error);
    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_EQ(*NewGroup, *FindRes); // Полное сравнение объектов должно пройти успешно

    inHardDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief HardDataStorage_RemoveGroupTest - Тест физического хранилища, проверяющий удаление группы
 * @param inHardDataStorage - Тестируемое физическое хранилище
 */
void HardDataStorage_RemoveGroupTest(std::unique_ptr<HMDataStorage> inHardDataStorage)
{
    std::error_code Error;

    Error = inHardDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inHardDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroupInfo> NewGroup = testscommon::make_group_info();

    Error = inHardDataStorage->addGroup(NewGroup); // Пытаемся добавить группу в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inHardDataStorage->removeGroup(NewGroup->m_uuid); // Пытаемся удалить группу пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<hmcommon::HMGroupInfo> FindRes = inHardDataStorage->findGroupByUUID(NewGroup->m_uuid, Error); // Попытка получить удалённую группу

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupNotExists)); // И метку, что группа не найдена в хранилище

    Error = inHardDataStorage->removeGroup(NewGroup->m_uuid); // Пытаемся удалить не существующую группу
    ASSERT_FALSE(Error); // Ошибки быть не должно

    inHardDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief HardDataStorage_SetGroupUsersTest - Тест физического хранилища, проверяющий присвоение участниковы группе
 * @param inHardDataStorage - Тестируемое физическое хранилище
 */
void HardDataStorage_SetGroupUsersTest(std::unique_ptr<HMDataStorage> inHardDataStorage)
{
    std::error_code Error;

    Error = inHardDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inHardDataStorage->is_open()); // Хранилище должно считаться открытым

    const size_t UsersCount = 5;
    std::shared_ptr<std::set<QUuid>> UserUUIDs = std::make_shared<std::set<QUuid>>(); // Инициализируем результат; // Контейнер UUID'ов пользователей

    for (size_t Index = 0; Index < UsersCount; ++Index)
    {
        QString TestUserLogin = "TestUser" + QString::number(Index); // Логины тестовых пользователей должны быть уникальными
        std::shared_ptr<hmcommon::HMUserInfo> User = testscommon::make_user_info(QUuid::createUuid(), TestUserLogin);

        Error = inHardDataStorage->addUser(User); // Пытаемся добавить пользователя
        ASSERT_FALSE(Error); // Ошибки быть не должно
        UserUUIDs->insert(User->m_uuid); // Запоминаем UUID добавленого в хранилище пользователя
    }

    std::shared_ptr<hmcommon::HMGroupInfo> NewGroup = testscommon::make_group_info(); // Создаём группу

    Error = inHardDataStorage->setGroupUsers(NewGroup->m_uuid, UserUUIDs); // Пытаемся добавить пользователей в не существующую в хранилище группу
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupNotExists)); // Получаем метку, что группа не найдена в хранилище

    Error = inHardDataStorage->addGroup(NewGroup); // Добавляем группу в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inHardDataStorage->setGroupUsers(NewGroup->m_uuid, UserUUIDs); // Пытаемся добавить пользователей в группу
    ASSERT_FALSE(Error); // Ошибки быть не должно
    // Теперь проверяем что связи созданы успешно
    std::shared_ptr<std::set<QUuid>> GroupUsers = inHardDataStorage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем список участников группы
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(GroupUsers, nullptr); // Должен вернуться валидный указаетль
    EXPECT_EQ(*UserUUIDs, *GroupUsers); // Перечни UUID'ов должны совпасть

    for (const QUuid& UserUUID : *UserUUIDs) // Перебираем всех добавленых в хранилище пользователей
    {
        std::shared_ptr<std::set<QUuid>> UserGroups = inHardDataStorage->getUserGroups(UserUUID, Error); // Запрашиваем перечень групп пользователей
        ASSERT_FALSE(Error); // Ошибки быть не должно
        ASSERT_NE(UserGroups, nullptr); // Должен вернуться валидный указаетль
        EXPECT_NE(UserGroups->find(NewGroup->m_uuid), UserGroups->cend()); // Группа должна быть успешно найдена в списке групп пользователей
    }

    inHardDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief HardDataStorage_AddGroupUserTest - Тест физического хранилища, проверяющий добавление участника в группу
 * @param inHardDataStorage - Тестируемое физическое хранилище
 */
void HardDataStorage_AddGroupUserTest(std::unique_ptr<HMDataStorage> inHardDataStorage)
{
    std::error_code Error;

    Error = inHardDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inHardDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroupInfo> NewGroup = testscommon::make_group_info(); // Создаём группу
    std::shared_ptr<hmcommon::HMUserInfo> NewUser = testscommon::make_user_info(); // Создаём пользовтаеля

    Error = inHardDataStorage->addGroupUser(NewGroup->m_uuid, NewUser->m_uuid); // Пытаемся добавить пользователя в не существующую в хранилище группу
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupNotExists)); // Получаем метку, что группа не найдена в хранилище

    Error = inHardDataStorage->addGroup(NewGroup); // Добавляем группу в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inHardDataStorage->addGroupUser(NewGroup->m_uuid, NewUser->m_uuid); // Пытаемся добавить не существующего в хранилище пользователя в группу
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // Получаем метку, что пользователь не найдена в хранилище

    Error = inHardDataStorage->addUser(NewUser); // Добавляем пользователя в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inHardDataStorage->addGroupUser(NewGroup->m_uuid, NewUser->m_uuid); // Пытаемся добавить пользователя в группу
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<std::set<QUuid>> GroupUsers = inHardDataStorage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем список групп пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(GroupUsers, nullptr); // Должен вернуться валидный указаетль
    EXPECT_NE(GroupUsers->find(NewUser->m_uuid), GroupUsers->cend()); // Пользователь должен быть успешно найден среди участников группы

    std::shared_ptr<std::set<QUuid>> UserGroups = inHardDataStorage->getUserGroups(NewUser->m_uuid, Error); // Запрашиваем список пользователей группы
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(UserGroups, nullptr); // Должен вернуться валидный указаетль
    EXPECT_NE(UserGroups->find(NewGroup->m_uuid), UserGroups->cend()); // Группа должна быть успешно найдена в списке групп пользователей

    Error = inHardDataStorage->addGroupUser(NewGroup->m_uuid, NewUser->m_uuid); // Пытаемся повторно добавить пользователя в группу
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupUserRelationAlredyExists)); // Получаем метку, что группа и пользователь уже связаны

    inHardDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief HardDataStorage_RemoveGroupUserTest - Тест физического хранилища, проверяющий удаление участника из группы
 * @param inHardDataStorage - Тестируемое физическое хранилище
 */
void HardDataStorage_RemoveGroupUserTest(std::unique_ptr<HMDataStorage> inHardDataStorage)
{
    std::error_code Error;

    Error = inHardDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inHardDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroupInfo> NewGroup = testscommon::make_group_info(); // Создаём группу
    Error = inHardDataStorage->addGroup(NewGroup); // Добавляем группу в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<hmcommon::HMUserInfo> NewUser = testscommon::make_user_info(); // Создаём пользовтаеля
    Error = inHardDataStorage->addUser(NewUser); // Добавляем пользователя в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inHardDataStorage->removeGroupUser(NewGroup->m_uuid, NewUser->m_uuid); // Пытаемся удалить из группы, не существующего в ней пользователя
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupUserRelationNotExists)); // Получаем метку, что группа и пользователь не связаны

    Error = inHardDataStorage->addGroupUser(NewGroup->m_uuid, NewUser->m_uuid); // Пытаемся добавить пользователя в группу
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<std::set<QUuid>> GroupUsers = inHardDataStorage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем список групп пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(GroupUsers, nullptr); // Должен вернуться валидный указаетль
    EXPECT_NE(GroupUsers->find(NewUser->m_uuid), GroupUsers->cend()); // Пользователь должен быть успешно найден среди участников группы

    std::shared_ptr<std::set<QUuid>> UserGroups = inHardDataStorage->getUserGroups(NewUser->m_uuid, Error); // Запрашиваем список пользователей группы
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(UserGroups, nullptr); // Должен вернуться валидный указаетль
    EXPECT_NE(UserGroups->find(NewGroup->m_uuid), UserGroups->cend()); // Группа должна быть успешно найдена в списке групп пользователей

    Error = inHardDataStorage->removeGroupUser(NewGroup->m_uuid, NewUser->m_uuid); // Пытаемся удалить пользователя из группы
    ASSERT_FALSE(Error); // Ошибки быть не должно

    GroupUsers = inHardDataStorage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем список групп пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(GroupUsers, nullptr); // Должен вернуться валидный указаетль
    EXPECT_EQ(GroupUsers->find(NewUser->m_uuid), GroupUsers->cend()); // Пользователь не должен быть найден среди участников группы

    UserGroups = inHardDataStorage->getUserGroups(NewUser->m_uuid, Error); // Запрашиваем список пользователей группы
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(UserGroups, nullptr); // Должен вернуться валидный указаетль
    EXPECT_EQ(UserGroups->find(NewGroup->m_uuid), UserGroups->cend()); // Группа не должна быть найдена в списке групп пользователей

    inHardDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief HardDataStorage_ClearGroupUsersTest - Тест физического хранилища, проверяющий очистку участников группы
 * @param inHardDataStorage - Тестируемое физическое хранилище
 */
void HardDataStorage_ClearGroupUsersTest(std::unique_ptr<HMDataStorage> inHardDataStorage)
{
    std::error_code Error;

    Error = inHardDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inHardDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroupInfo> NewGroup = testscommon::make_group_info(); // Создаём группу

    Error = inHardDataStorage->clearGroupUsers(NewGroup->m_uuid); // Пытаемся очистить перечень участников не существующей группы
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupNotExists)); // Получаем метку, что группа не существует

    Error = inHardDataStorage->addGroup(NewGroup); // Добавляем группу в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    const std::size_t UsersCount = 5; // Контейнер, хранащий пользователей
    std::shared_ptr<std::set<QUuid>> UserUUIDs = std::make_shared<std::set<QUuid>>(); // Контейнер UUID'ов пользователей

    for (size_t Index = 0; Index < UsersCount; ++Index)
    {
        QString TestUserLogin = "TestUser" + QString::number(Index); // Логины тестовых пользователей должны быть уникальными
        std::shared_ptr<hmcommon::HMUserInfo> User = testscommon::make_user_info(QUuid::createUuid(), TestUserLogin);

        Error = inHardDataStorage->addUser(User); // Пытаемся добавить пользователя
        ASSERT_FALSE(Error); // Ошибки быть не должно
        UserUUIDs->insert(User->m_uuid); // Запоминаем UUID добавленого в хранилище пользователя
    }

    Error = inHardDataStorage->setGroupUsers(NewGroup->m_uuid, UserUUIDs); // Пытаемся добавить пользователей в группу
    ASSERT_FALSE(Error); // Ошибки быть не должно
    // Теперь проверяем что связи созданы успешно
    std::shared_ptr<std::set<QUuid>> GroupUsers = inHardDataStorage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем список участников группы
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(UserUUIDs, nullptr); // Должен вернуться валидный указаетль
    EXPECT_EQ(*UserUUIDs, *GroupUsers); // Перечни UUID'ов должны совпасть

    for (const QUuid& UserUUID : *UserUUIDs) // Перебираем всех добавленых в хранилище пользователей
    {
        std::shared_ptr<std::set<QUuid>> UserGroups = inHardDataStorage->getUserGroups(UserUUID, Error); // Запрашиваем перечень групп пользователей
        ASSERT_FALSE(Error); // Ошибки быть не должно
        ASSERT_NE(UserGroups, nullptr); // Должен вернуться валидный указаетль
        EXPECT_NE(UserGroups->find(NewGroup->m_uuid), UserGroups->cend()); // Группа должна быть успешно найдена в списке групп пользователей
    }

    Error = inHardDataStorage->clearGroupUsers(NewGroup->m_uuid); // Очищаем перечень участников группы
    ASSERT_FALSE(Error); // Ошибки быть не должно
    // Теперь проверяем что связи разорваны
    GroupUsers = inHardDataStorage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем список участников группы
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(GroupUsers, nullptr); // Должен вернуться валидный указаетль
    EXPECT_TRUE(GroupUsers->empty()); // Перечень участников группы должен быть пуст

    for (const QUuid& UserUUID : *UserUUIDs) // Перебираем всех добавленых в хранилище пользователей
    {
        std::shared_ptr<std::set<QUuid>> UserGroups = inHardDataStorage->getUserGroups(UserUUID, Error); // Запрашиваем перечень групп пользователей
        ASSERT_FALSE(Error); // Ошибки быть не должно
        ASSERT_NE(UserGroups, nullptr); // Должен вернуться валидный указаетль
        EXPECT_EQ(UserGroups->find(NewGroup->m_uuid), UserGroups->cend()); // Группа не должна быть найдена в списке групп пользователей
    }

    inHardDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief HardDataStorage_GetGroupUserListTest - Тест физического хранилища, проверяющий запрос списка участников группы
 * @param inHardDataStorage - Тестируемое физическое хранилище
 */
void HardDataStorage_GetGroupUserListTest(std::unique_ptr<HMDataStorage> inHardDataStorage)
{
    std::error_code Error;

    Error = inHardDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inHardDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroupInfo> NewGroup = testscommon::make_group_info(); // Создаём группу

    std::shared_ptr<std::set<QUuid>> GroupUsers = inHardDataStorage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем список участников не существующей в хранилище группы
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupNotExists)); // Получаем метку, что группы не существует

    Error = inHardDataStorage->addGroup(NewGroup); // Добавляем группу в хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно

    GroupUsers = inHardDataStorage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем список участников только что созданной группы
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(GroupUsers, nullptr); // Должен вернуться валидный указаетль
    EXPECT_TRUE(GroupUsers->empty()); // Перечень участников должен быть пуст

    const std::size_t UsersCount = 5; // Контейнер, хранащий пользователей
    std::shared_ptr<std::set<QUuid>> UserUUIDs = std::make_shared<std::set<QUuid>>(); // Контейнер UUID'ов пользователей

    for (size_t Index = 0; Index < UsersCount; ++Index)
    {
        QString TestUserLogin = "TestUser" + QString::number(Index); // Логины тестовых пользователей должны быть уникальными
        std::shared_ptr<hmcommon::HMUserInfo> User = testscommon::make_user_info(QUuid::createUuid(), TestUserLogin);

        Error = inHardDataStorage->addUser(User); // Пытаемся добавить пользователя
        ASSERT_FALSE(Error); // Ошибки быть не должно
        UserUUIDs->insert(User->m_uuid); // Запоминаем UUID добавленого в хранилище пользователя
    }

    Error = inHardDataStorage->setGroupUsers(NewGroup->m_uuid, UserUUIDs); // Пытаемся добавить пользователей в группу
    ASSERT_FALSE(Error); // Ошибки быть не должно

    GroupUsers = inHardDataStorage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем список участников группы
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(UserUUIDs, nullptr); // Должен вернуться валидный указаетль
    EXPECT_EQ(*UserUUIDs, *GroupUsers); // Перечни UUID'ов должны совпасть

    inHardDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief HardDataStorage_AddMessageTest - Тест физического хранилища, проверяющий добавление сообщения
 * @param inHardDataStorage - Тестируемое физическое хранилище
 */
void HardDataStorage_AddMessageTest(std::unique_ptr<HMDataStorage> inHardDataStorage)
{
    std::error_code Error; // Метка ошибки

    Error = inHardDataStorage->open(); // Пытаемся открыть хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inHardDataStorage->is_open()); // Хранилище должно считаться открытым

    // Сообщениее может быть добавлено только в группу
    std::shared_ptr<hmcommon::HMGroupInfo> NewGroup = testscommon::make_group_info();

    hmcommon::MsgData TextData(hmcommon::eMsgType::mtText, "Текст сообщения"); // Формируем данные сообщения
    std::shared_ptr<hmcommon::HMGroupInfoMessage> NewMessage = testscommon::make_groupmessage(TextData, QUuid::createUuid(), NewGroup->m_uuid); // Формируем сообщение

    Error = inHardDataStorage->addMessage(NewMessage); // Пытаемся добавить сообщение без группы
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupNotExists)); // И метку, что группа не найдена в хранилище

    Error = inHardDataStorage->addGroup(NewGroup); // Теперь добавим группу
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inHardDataStorage->addMessage(NewMessage); // Пытаемся добавить сообщение
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inHardDataStorage->addMessage(NewMessage); // Пытаемся добавить сообщение повторно
    EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsMessageAlreadyExists)); // Должны получить сообщение о том, что этот UUID уже зарегистрирован

    inHardDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief HardDataStorage_UpdateMessageTest - Тест физического хранилища, проверяющий обновление сообщения
 * @param inHardDataStorage - Тестируемое физическое хранилище
 */
void HardDataStorage_UpdateMessageTest(std::unique_ptr<HMDataStorage> inHardDataStorage)
{
    std::error_code Error; // Метка ошибки

    Error = inHardDataStorage->open(); // Пытаемся открыть хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inHardDataStorage->is_open()); // Хранилище должно считаться открытым

    // Сообщениее может быть добавлено только в группу
    std::shared_ptr<hmcommon::HMGroupInfo> NewGroup = testscommon::make_group_info();

    Error = inHardDataStorage->addGroup(NewGroup);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    hmcommon::MsgData TextData(hmcommon::eMsgType::mtText, "Текст сообщения"); // Формируем данные сообщения
    std::shared_ptr<hmcommon::HMGroupInfoMessage> NewMessage = testscommon::make_groupmessage(TextData, QUuid::createUuid(), NewGroup->m_uuid); // Формируем сообщение

    Error = inHardDataStorage->updateMessage(NewMessage); // Пытаемся обновить сообщение не добавляя
    EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsMessageNotExists)); // Должны получить сообщение о том, что не удалось найти сообщение для обновления

    Error = inHardDataStorage->addMessage(NewMessage); // Пытаемся добавить сообщение
    ASSERT_FALSE(Error); // Ошибки быть не должно

    TextData.m_data = QString("Новый текст сообщения").toLocal8Bit(); // Меняем текст
    NewMessage->setMessage(TextData);

    Error = inHardDataStorage->updateMessage(NewMessage); // Пытаемся обновить сообщение
    ASSERT_FALSE(Error); // Ошибки быть не должно

    inHardDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief HardDataStorage_FindMessageTest - Тест физического хранилища, проверяющий поиск сообщения
 * @param inHardDataStorage - Тестируемое физическое хранилище
 */
void HardDataStorage_FindMessageTest(std::unique_ptr<HMDataStorage> inHardDataStorage)
{
    std::error_code Error; // Метка ошибки

    Error = inHardDataStorage->open(); // Пытаемся открыть хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inHardDataStorage->is_open()); // Хранилище должно считаться открытым

    // Сообщениее может быть добавлено только в группу
    std::shared_ptr<hmcommon::HMGroupInfo> NewGroup = testscommon::make_group_info();

    hmcommon::MsgData TextData(hmcommon::eMsgType::mtText, "Текст сообщения"); // Формируем данные сообщения
    std::shared_ptr<hmcommon::HMGroupInfoMessage> NewMessage = testscommon::make_groupmessage(TextData, QUuid::createUuid(), NewGroup->m_uuid); // Формируем сообщение

    std::shared_ptr<hmcommon::HMGroupInfoMessage> FindRes = inHardDataStorage->findMessage(NewMessage->m_uuid, Error); // Пытаемся найти не добавленное сообщение

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsMessageNotExists)); // Должны получить сообщение о том, что сообщение не найдено

    Error = inHardDataStorage->addGroup(NewGroup); // Добавим группу
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inHardDataStorage->addMessage(NewMessage); // Пытаемся добавить сообщение
    ASSERT_FALSE(Error); // Ошибки быть не должно

    FindRes = inHardDataStorage->findMessage(NewMessage->m_uuid, Error); // Пытаемся найти не добавленное сообщение

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    inHardDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief HardDataStorage_FindMessagesTest - Тест физического хранилища, проверяющий поиск сообщений
 * @param inHardDataStorage - Тестируемое физическое хранилище
 */
void HardDataStorage_FindMessagesTest(std::unique_ptr<HMDataStorage> inHardDataStorage)
{
    std::error_code Error; // Метка ошибки

    Error = inHardDataStorage->open(); // Пытаемся открыть хранилище
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inHardDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroupInfo> NewGroup = testscommon::make_group_info();

    Error = inHardDataStorage->addGroup(NewGroup); // добавляем группу сообщения
    ASSERT_FALSE(Error); // Ошибки быть не должно

    const size_t MESSAGES = 5;
    std::array<std::shared_ptr<hmcommon::HMGroupInfoMessage>, MESSAGES> Messages;
    std::array<QDateTime, MESSAGES> Times;

    for (std::size_t Index = 0; Index < MESSAGES; ++Index)
    {
        //QDateTime(QDate(), QTime::currentTime()); // Запоминаем время
        Times[Index] = QDateTime::currentDateTime();
        hmcommon::MsgData TextData(hmcommon::eMsgType::mtText, ("Текст сообщения " + QString::number(Index)).toLocal8Bit()); // Формируем данные сообщения
        Messages[Index] = testscommon::make_groupmessage(TextData, QUuid::createUuid(), NewGroup->m_uuid, Times[Index]); // Формируем сообщение группы

        Error = inHardDataStorage->addMessage(Messages[Index]);
        ASSERT_FALSE(Error); // Ошибки быть не должно
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    hmcommon::MsgRange TimeRange(Times[0], Times[MESSAGES - 2]); // Выбираем временной интервал со временни первого до времени предпоследнего сообщения
    std::vector<std::shared_ptr<hmcommon::HMGroupInfoMessage>> FindRes = inHardDataStorage->findMessages(NewGroup->m_uuid, TimeRange, Error); // Получаем резульата

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

    inHardDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief HardDataStorage_RemoveMessageTest - Тест физического хранилища, проверяющий удаление сообщения
 * @param inHardDataStorage - Тестируемое физическое хранилище
 */
void HardDataStorage_RemoveMessageTest(std::unique_ptr<HMDataStorage> inHardDataStorage)
{
    std::error_code Error;

    Error = inHardDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inHardDataStorage->is_open()); // Хранилище должно считаться открытым

    // Сообщениее может быть добавлено только в группу
    std::shared_ptr<hmcommon::HMGroupInfo> NewGroup = testscommon::make_group_info();

    hmcommon::MsgData TextData(hmcommon::eMsgType::mtText, "Текст сообщения"); // Формируем данные сообщения
    std::shared_ptr<hmcommon::HMGroupInfoMessage> NewMessage = testscommon::make_groupmessage(TextData, QUuid::createUuid(), NewGroup->m_uuid); // Формируем сообщение

    Error = inHardDataStorage->addGroup(NewGroup); // Добавим группу
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inHardDataStorage->removeGroup(NewGroup->m_uuid); // Пытаемся удалить добавленное сообщение
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<hmcommon::HMGroupInfoMessage> FindRes = inHardDataStorage->findMessage(NewGroup->m_uuid, Error); // Попытка получить удалённое сообщение

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsMessageNotExists)); // И метку, что сообщение не найдено в хранилище

    Error = inHardDataStorage->removeGroup(NewGroup->m_uuid); // Пытаемся удалить не существующее сообщение
    ASSERT_FALSE(Error); // Ошибки быть не должно

    inHardDataStorage->close();
}
//-----------------------------------------------------------------------------

#endif // HAWKSERVERCOREHARDDATASTORAGETEST_HPP
