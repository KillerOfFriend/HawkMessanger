#ifndef HAWKSERVERCORECACHEDDATASTORAGETEST_HPP
#define HAWKSERVERCORECACHEDDATASTORAGETEST_HPP

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
 * @brief CachedDataStorage_AddUserTest - Тест физического хранилища, проверяющий добавление пользователя
 * @param inCachedDataStorage - Тестируемое физическое хранилище
 */
void CachedDataStorage_AddUserTest(std::unique_ptr<HMDataStorage> inCachedDataStorage)
{
    std::error_code Error;

    Error = inCachedDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inCachedDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUserInfo> NewUser = testscommon::make_user_info();

    Error = inCachedDataStorage->addUser(NewUser); // Пытаемся добавить пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inCachedDataStorage->addUser(NewUser); // Пытаемся добавить повторно
    EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserAlreadyExists)); // Должны получить сообщение что пользователь уже кеширован

    inCachedDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief CachedDataStorage_UpdateUserTest - Тест физического хранилища, проверяющий обновление пользователя
 * @param inCachedDataStorage - Тестируемое физическое хранилище
 */
void CachedDataStorage_UpdateUserTest(std::unique_ptr<HMDataStorage> inCachedDataStorage)
{
    std::error_code Error;

    Error = inCachedDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inCachedDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUserInfo> NewUser = testscommon::make_user_info();

    Error = inCachedDataStorage->addUser(NewUser); // Пытаемся добавить пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    NewUser->setName("New User Name");

    Error = inCachedDataStorage->updateUser(NewUser); // Пытаемся обновить пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    inCachedDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief CachedDataStorage_FindUserByUUIDTest - Тест физического хранилища, проверяющий поиск пользователя по UUID
 * @param inCachedDataStorage - Тестируемое физическое хранилище
 */
void CachedDataStorage_FindUserByUUIDTest(std::unique_ptr<HMDataStorage> inCachedDataStorage)
{
    std::error_code Error;

    Error = inCachedDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inCachedDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUserInfo> NewUser = testscommon::make_user_info();

    std::shared_ptr<hmcommon::HMUserInfo> FindRes = inCachedDataStorage->findUserByUUID(NewUser->m_uuid, Error); // Попытка получить не существующего пользователя

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // И метку, что пользователь не кеширован

    Error = inCachedDataStorage->addUser(NewUser); // Пытаемся добавить пользователя в кеш
    ASSERT_FALSE(Error); // Ошибки быть не должно

    const size_t SilCount = 5; // Добавим некоторое количество левых пользователей
    for (size_t Index = 0; Index < SilCount; ++Index)
    {
        QString TrashUserLogin = "TrashUser" + QString::number(Index); // Логины мусорных пользователей должны быть уникальными
        Error = inCachedDataStorage->addUser(testscommon::make_user_info(QUuid::createUuid(), TrashUserLogin)); // Пытаемся добавить пользователя
        ASSERT_FALSE(Error); // Ошибки быть не должно
    }

    FindRes = inCachedDataStorage->findUserByUUID(NewUser->m_uuid, Error);

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_EQ(FindRes, NewUser); // Проводим сравнение указателей (КОНЦЕПЦИЯ кеша: Объект всегда хранится под одним указателем)

    inCachedDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief CachedDataStorage_FindUserByAuthenticationTest - Тест физического хранилища, проверяющий поиск пользователя по данным аутентификации
 * @param inCachedDataStorage - Тестируемое физическое хранилище
 */
void CachedDataStorage_FindUserByAuthenticationTest(std::unique_ptr<HMDataStorage> inCachedDataStorage)
{
    std::error_code Error;

    Error = inCachedDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inCachedDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUserInfo> NewUser = testscommon::make_user_info();

    std::shared_ptr<hmcommon::HMUserInfo> FindRes = inCachedDataStorage->findUserByAuthentication(NewUser->getLogin(), NewUser->getPasswordHash(), Error); // Попытка получить не существующего пользователя

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // И метку, что пользователь не кеширован

    Error = inCachedDataStorage->addUser(NewUser); // Пытаемся добавить пользователя в кеш
    ASSERT_FALSE(Error); // Ошибки быть не должно

    const size_t SilCount = 5; // Добавим некоторое количество левых пользователей
    for (size_t Index = 0; Index < SilCount; ++Index)
    {
        QString TrashUserLogin = "TrashUser" + QString::number(Index); // Логины мусорных пользователей должны быть уникальными
        Error = inCachedDataStorage->addUser(testscommon::make_user_info(QUuid::createUuid(), TrashUserLogin)); // Пытаемся добавить пользователя
        ASSERT_FALSE(Error); // Ошибки быть не должно
    }

    FindRes = inCachedDataStorage->findUserByAuthentication(NewUser->getLogin(), NewUser->getPasswordHash(), Error); // Попытка получить не существующего пользователя

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_EQ(FindRes, NewUser); // Проводим сравнение указателей (КОНЦЕПЦИЯ кешА: Объект всегда хранится под одним указателем)

    inCachedDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief CachedDataStorage_RemoveUserTest - Тест физического хранилища, проверяющий удаление пользователя
 * @param inCachedDataStorage - Тестируемое физическое хранилище
 */
void CachedDataStorage_RemoveUserTest(std::unique_ptr<HMDataStorage> inCachedDataStorage)
{
    std::error_code Error;

    Error = inCachedDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inCachedDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUserInfo> NewUser = testscommon::make_user_info();

    Error = inCachedDataStorage->addUser(NewUser); // Пытаемся добавить пользователя в кеш
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inCachedDataStorage->removeUser(NewUser->m_uuid); // Пытаемся удалить добавленного пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<hmcommon::HMUserInfo> FindRes = inCachedDataStorage->findUserByUUID(NewUser->m_uuid, Error); // Попытка получить удалённого пользователя

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserNotExists)); // И метку, что пользователь не кеширован

    inCachedDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief CachedDataStorage_SetUserContactsTest - Тест физического хранилища, проверяющий присвоение контактов пользователю
 * @param inCachedDataStorage - Тестируемое физическое хранилище
 */
void CachedDataStorage_SetUserContactsTest(std::unique_ptr<HMDataStorage> inCachedDataStorage)
{
    std::error_code Error;

    Error = inCachedDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inCachedDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUserInfo> NewUser = testscommon::make_user_info(QUuid::createUuid(), "TestUser@login.com");
    std::shared_ptr<hmcommon::HMUserInfo> NewContact1 = testscommon::make_user_info(QUuid::createUuid(), "TestContact1@login.com");
    std::shared_ptr<hmcommon::HMUserInfo> NewContact2 = testscommon::make_user_info(QUuid::createUuid(), "TestContact2@login.com");

    std::shared_ptr<std::set<QUuid>> NewContactList = std::make_shared<std::set<QUuid>>();

    NewContactList->insert(NewContact1->m_uuid);
    NewContactList->insert(NewContact2->m_uuid);

    Error = inCachedDataStorage->setUserContacts(NewUser->m_uuid, NewContactList); // Пытаемся добавить список контактов без добавления пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно (Наличие пользователя в кеше не обязательно)

    inCachedDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief CachedDataStorage_AddUserContactTest - Тест физического хранилища, проверяющий добавление контакта пользователю
 * @param inCachedDataStorage - Тестируемое физическое хранилище
 */
void CachedDataStorage_AddUserContactTest(std::unique_ptr<HMDataStorage> inCachedDataStorage)
{
    std::error_code Error;

    Error = inCachedDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inCachedDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUserInfo> NewUser = testscommon::make_user_info(QUuid::createUuid(), "TestUser@login.com");
    std::shared_ptr<hmcommon::HMUserInfo> NewContact = testscommon::make_user_info(QUuid::createUuid(), "TestContact@login.com");

    Error = inCachedDataStorage->addUserContact(NewUser->m_uuid, NewContact->m_uuid);
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserContactRelationNotExists)); // Должны получить сообщение о том, что связь не существует

    std::shared_ptr<std::set<QUuid>> NewContactList = std::make_shared<std::set<QUuid>>();

    Error = inCachedDataStorage->setUserContacts(NewUser->m_uuid, NewContactList); // Добавляем пустой список контактов
    ASSERT_FALSE(Error); // Ошибки быть не должно (Наличие пользователя в кеше не обязательно)
    Error = inCachedDataStorage->addUserContact(NewUser->m_uuid, NewContact->m_uuid); // Добавляем контакт
    ASSERT_FALSE(Error); // Ошибки быть не должно (Наличие пользователя в кеше не обязательно)

    Error = inCachedDataStorage->addUserContact(NewUser->m_uuid, NewContact->m_uuid); // Повторно добавляем контакт
    ASSERT_EQ(Error.value(), static_cast<int32_t>(hmcommon::eSystemErrorEx::seAlredyInContainer)); // Должны получить сообщение о том, что контакт уже в кеше

    inCachedDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief CachedDataStorage_RemoveUserContactTest - Тест физического хранилища, проверяющий удаление контакта пользователю
 * @param inCachedDataStorage - Тестируемое физическое хранилище
 */
void CachedDataStorage_RemoveUserContactTest(std::unique_ptr<HMDataStorage> inCachedDataStorage)
{
    std::error_code Error;

    Error = inCachedDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inCachedDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUserInfo> NewUser = testscommon::make_user_info(QUuid::createUuid(), "TestUser@login.com");
    std::shared_ptr<hmcommon::HMUserInfo> NewContact = testscommon::make_user_info(QUuid::createUuid(), "TestContact@login.com");

    Error = inCachedDataStorage->removeUserContact(NewUser->m_uuid, NewContact->m_uuid); // Пытаемся удалсть не существующий контакт
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserContactRelationNotExists)); // Должны получить сообщение о том, что связи не существует

    std::shared_ptr<std::set<QUuid>> NewContactList = std::make_shared<std::set<QUuid>>();

    Error = inCachedDataStorage->setUserContacts(NewUser->m_uuid, NewContactList); // Добавляем пустой список контактов
    ASSERT_FALSE(Error); // Ошибки быть не должно (Наличие пользователя в кеше не обязательно)

    Error = inCachedDataStorage->addUserContact(NewUser->m_uuid, NewContact->m_uuid); // И добавляем контакт пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inCachedDataStorage->removeUserContact(NewUser->m_uuid, NewContact->m_uuid); // И наконец штатно удаляем связь
    ASSERT_FALSE(Error); // Ошибки быть не должно

    inCachedDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief CachedDataStorage_ClearUserContactsTest - Тест физического хранилища, проверяющий очистку контактов пользователей
 * @param inCachedDataStorage - Тестируемое физическое хранилище
 */
void CachedDataStorage_ClearUserContactsTest(std::unique_ptr<HMDataStorage> inCachedDataStorage)
{
    std::error_code Error;

    Error = inCachedDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inCachedDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUserInfo> NewUser = testscommon::make_user_info(QUuid::createUuid(), "TestUser@login.com");
    std::shared_ptr<hmcommon::HMUserInfo> NewContact1 = testscommon::make_user_info(QUuid::createUuid(), "TestContact1@login.com");
    std::shared_ptr<hmcommon::HMUserInfo> NewContact2 = testscommon::make_user_info(QUuid::createUuid(), "TestContact2@login.com");

    Error = inCachedDataStorage->clearUserContacts(NewUser->m_uuid); // Пытаемся удалить не сущестующую связь
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsSuccess)); // Вне зависимости от наличия в кеше, удаление валидно

    std::shared_ptr<std::set<QUuid>> NewContactList = std::make_shared<std::set<QUuid>>();

    Error = inCachedDataStorage->setUserContacts(NewUser->m_uuid, NewContactList); // Добавляем пустой список контактов
    ASSERT_FALSE(Error); // Ошибки быть не должно (Наличие пользователя в кеше не обязательно)

    Error = inCachedDataStorage->addUserContact(NewUser->m_uuid, NewUser->m_uuid); // Пытаемся добавить в список контактов самого себя
    ASSERT_EQ(Error.value(), static_cast<int32_t>(hmcommon::eSystemErrorEx::seIncorretData)); // Должны получить сообщение о том, что данные не корректны

    Error = inCachedDataStorage->addUserContact(NewUser->m_uuid, NewContact1->m_uuid); // И добавляем контакт1 пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inCachedDataStorage->addUserContact(NewUser->m_uuid, NewContact2->m_uuid); // И добавляем контакт2 пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inCachedDataStorage->clearUserContacts(NewUser->m_uuid); // Удаляем связь
    ASSERT_FALSE(Error); // Ошибки быть не должно

    inCachedDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief CachedDataStorage_GetUserContactListTest- Тест физического хранилища, проверяющий запрос списка контактов пользователей
 * @param inCachedDataStorage - Тестируемое физическое хранилище
 */
void CachedDataStorage_GetUserContactListTest(std::unique_ptr<HMDataStorage> inCachedDataStorage)
{
    std::error_code Error;

    Error = inCachedDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inCachedDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMUserInfo> NewUser = testscommon::make_user_info(QUuid::createUuid(), "TestUser@login.com");
    std::shared_ptr<hmcommon::HMUserInfo> NewContact1 = testscommon::make_user_info(QUuid::createUuid(), "TestContact1@login.com");
    std::shared_ptr<hmcommon::HMUserInfo> NewContact2 = testscommon::make_user_info(QUuid::createUuid(), "TestContact2@login.com");

    std::shared_ptr<std::set<QUuid>> FindRes = inCachedDataStorage->getUserContactList(NewUser->m_uuid, Error); // Пытаемся получить список контактов не существующего пользователя

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserContactRelationNotExists)); // Должны получить сообщение о том, что связь не существует

    std::shared_ptr<std::set<QUuid>> NewContactList = std::make_shared<std::set<QUuid>>();

    Error = inCachedDataStorage->setUserContacts(NewUser->m_uuid, NewContactList); // Добавляем пустой список контактов
    ASSERT_FALSE(Error); // Ошибки быть не должно (Наличие пользователя в кеше не обязательно)

    Error = inCachedDataStorage->addUserContact(NewUser->m_uuid, NewContact1->m_uuid); // И добавляем контакт1 пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inCachedDataStorage->addUserContact(NewUser->m_uuid, NewContact2->m_uuid); // И добавляем контакт2 пользователю
    ASSERT_FALSE(Error); // Ошибки быть не должно

    FindRes = inCachedDataStorage->getUserContactList(NewUser->m_uuid, Error); // Теперь пытваемся получить список контактов пользователя

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_NE(FindRes->find(NewContact1->m_uuid), FindRes->end()); // Должен содержать первый контакт
    EXPECT_NE(FindRes->find(NewContact2->m_uuid), FindRes->end()); // Должен содержать второй контакт

    inCachedDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief CachedDataStorage_GetUserGroupsTest - Тест физического хранилища, проверяющий запрос списка групп пользователей
 * @param inCachedDataStorage - Тестируемое физическое хранилище
 */
void CachedDataStorage_GetUserGroupsTest(std::unique_ptr<HMDataStorage> inCachedDataStorage)
{
    std::error_code Error;

    Error = inCachedDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inCachedDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroupInfo> NewGroup = testscommon::make_group_info(); // Создаём группу (В кеш добавлять не обязательно)
    std::shared_ptr<hmcommon::HMUserInfo> NewUser = testscommon::make_user_info(); // Создаём пользователя(В кеш добавлять не обязательно)

    // Кешируем полный список
    std::shared_ptr<std::set<QUuid>> UserUuids = std::make_shared<std::set<QUuid>>(); // Формируем пустой перечень участников группы
    UserUuids->insert(NewUser->m_uuid); // Добавляем пользователя в перечень

    Error = inCachedDataStorage->setGroupUsers(NewGroup->m_uuid, UserUuids); // Задаём перечень участников группы
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<std::set<QUuid>> UserGroups = inCachedDataStorage->getUserGroups(NewGroup->m_uuid, Error); // Запрашиваем список групп пользователя
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsUserGroupsRelationNotExists)); // СВЯЗИ ДЛЯ ПОЛЬЗОВАТЕЛЯ НЕ КЕШИРУЮТСЯ
    ASSERT_EQ(UserGroups, nullptr); // Должен вернуться nullptr

    inCachedDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief CachedDataStorage_AddGroupTest - Тест физического хранилища, проверяющий добавление группы
 * @param inCachedDataStorage - Тестируемое физическое хранилище
 */
void CachedDataStorage_AddGroupTest(std::unique_ptr<HMDataStorage> inCachedDataStorage)
{
    std::error_code Error;

    Error = inCachedDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inCachedDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroupInfo> NewGroup = testscommon::make_group_info();

    Error = inCachedDataStorage->addGroup(NewGroup);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inCachedDataStorage->addGroup(NewGroup);
    EXPECT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupAlreadyExists)); // Должны получить сообщение что группа уже кеширована

    inCachedDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief CachedDataStorage_UpdateGroupTest - Тест физического хранилища, проверяющий обновление группы
 * @param inCachedDataStorage - Тестируемое физическое хранилище
 */
void CachedDataStorage_UpdateGroupTest(std::unique_ptr<HMDataStorage> inCachedDataStorage)
{
    std::error_code Error;

    Error = inCachedDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inCachedDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroupInfo> NewGroup = testscommon::make_group_info();

    Error = inCachedDataStorage->addGroup(NewGroup);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    NewGroup->setName("New Group Name");

    Error = inCachedDataStorage->updateGroup(NewGroup);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    inCachedDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief CachedDataStorage_FindGroupByUUIDTest - Тест физического хранилища, проверяющий поиск группы
 * @param inCachedDataStorage - Тестируемое физическое хранилище
 */
void CachedDataStorage_FindGroupByUUIDTest(std::unique_ptr<HMDataStorage> inCachedDataStorage)
{
    std::error_code Error;

    Error = inCachedDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inCachedDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroupInfo> NewGroup = testscommon::make_group_info();

    std::shared_ptr<hmcommon::HMGroupInfo> FindRes = inCachedDataStorage->findGroupByUUID(NewGroup->m_uuid, Error); // Попытка получить не существующую группу

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupNotExists)); // И метку, что группа не кеширована

    Error = inCachedDataStorage->addGroup(NewGroup); // Пытаемся добавить группу в кеш
    ASSERT_FALSE(Error); // Ошибки быть не должно

    const size_t SilCount = 5; // Добавим некоторое количество левых групп
    for (size_t Index = 0; Index < SilCount; ++Index)
    {
        QString TrashGroupName = "TrashGroup" + QString::number(Index); // Логины мусорных пользователей должны быть уникальными
        Error = inCachedDataStorage->addGroup(testscommon::make_group_info(QUuid::createUuid(), TrashGroupName)); // Пытаемся добавить группу
        ASSERT_FALSE(Error); // Ошибки быть не должно
    }

    FindRes = inCachedDataStorage->findGroupByUUID(NewGroup->m_uuid, Error);

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указатель
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_EQ(FindRes, NewGroup); // Проводим сравнение указателей (КОНЦЕПЦИЯ кешА: Объект всегда хранится под одним указателем)

    inCachedDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief CachedDataStorage_RemoveGroupTest - Тест физического хранилища, проверяющий удаление группы
 * @param inCachedDataStorage - Тестируемое физическое хранилище
 */
void CachedDataStorage_RemoveGroupTest(std::unique_ptr<HMDataStorage> inCachedDataStorage)
{
    std::error_code Error;

    Error = inCachedDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inCachedDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroupInfo> NewGroup = testscommon::make_group_info();

    Error = inCachedDataStorage->addGroup(NewGroup); // Пытаемся добавить группу в кеш
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inCachedDataStorage->removeGroup(NewGroup->m_uuid); // Пытаемся удалить группу пользователя
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<hmcommon::HMGroupInfo> FindRes = inCachedDataStorage->findGroupByUUID(NewGroup->m_uuid, Error); // Попытка получить удалённую группу

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupNotExists)); // И метку, что группа не кеширована

    inCachedDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief CachedDataStorage_SetGroupUsersTest - Тест физического хранилища, проверяющий присвоение участниковы группе
 * @param inCachedDataStorage - Тестируемое физическое хранилище
 */
void CachedDataStorage_SetGroupUsersTest(std::unique_ptr<HMDataStorage> inCachedDataStorage)
{
    std::error_code Error;

    Error = inCachedDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inCachedDataStorage->is_open()); // Хранилище должно считаться открытым

    const size_t UsersCount = 5;
    std::shared_ptr<std::set<QUuid>> UserUUIDs = std::make_shared<std::set<QUuid>>(); // Инициализируем результат; // Контейнер UUID'ов пользователей

    for (size_t Index = 0; Index < UsersCount; ++Index)
    {
        QString TestUserLogin = "TestUser" + QString::number(Index); // Логины тестовых пользователей должны быть уникальными
        std::shared_ptr<hmcommon::HMUserInfo> User = testscommon::make_user_info(QUuid::createUuid(), TestUserLogin); // Создаём пользователя (в кеш добавлять не обязательно)

        UserUUIDs->insert(User->m_uuid); // Запоминаем UUID добавленого в хранилище пользователя
    }

    std::shared_ptr<hmcommon::HMGroupInfo> NewGroup = testscommon::make_group_info(); // Создаём группу (в кеш добавлять не обязательно)

    Error = inCachedDataStorage->setGroupUsers(NewGroup->m_uuid, UserUUIDs); // Пытаемся добавить пользователей
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<std::set<QUuid>> GroupUsers = inCachedDataStorage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем участников группы
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(GroupUsers, nullptr); // Должен вернуться валидный указатель
    EXPECT_EQ(*UserUUIDs, *GroupUsers); //

    inCachedDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief CachedDataStorage_AddGroupUserTest - Тест физического хранилища, проверяющий добавление участника в группу
 * @param inCachedDataStorage - Тестируемое физическое хранилище
 */
void CachedDataStorage_AddGroupUserTest(std::unique_ptr<HMDataStorage> inCachedDataStorage)
{
    std::error_code Error;

    Error = inCachedDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inCachedDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroupInfo> NewGroup = testscommon::make_group_info(); // Создаём группу (в кеш добавлять не обязательно)
    std::shared_ptr<hmcommon::HMUserInfo> NewUser = testscommon::make_user_info(); // Создаём пользователя (в кеш добавлять не обязательно)

    Error = inCachedDataStorage->addGroupUser(NewGroup->m_uuid, NewUser->m_uuid); // Добавляем участника в группу
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupUserRelationNotExists)); // Дложны получить сообщение, что нет связь не кеширована
    // Кешируем полный список
    std::shared_ptr<std::set<QUuid>> UserUuids = std::make_shared<std::set<QUuid>>(); // Формируем пустой перечень участников группы

    Error = inCachedDataStorage->setGroupUsers(NewGroup->m_uuid, UserUuids);
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inCachedDataStorage->addGroupUser(NewGroup->m_uuid, NewUser->m_uuid); // Добавляем участника в группу с уже кешированными данными
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<std::set<QUuid>> GroupUsers = inCachedDataStorage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем перечень пользователей группы
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(GroupUsers, nullptr); // Должен вернуться валидный указаетль
    EXPECT_NE(GroupUsers->find(NewUser->m_uuid), GroupUsers->cend()); // Пользователь должен быть успешно найден в перечне участников

    inCachedDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief CachedDataStorage_RemoveGroupUserTest - Тест физического хранилища, проверяющий удаление участника из группы
 * @param inCachedDataStorage - Тестируемое физическое хранилище
 */
void CachedDataStorage_RemoveGroupUserTest(std::unique_ptr<HMDataStorage> inCachedDataStorage)
{
    std::error_code Error;

    Error = inCachedDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inCachedDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroupInfo> NewGroup = testscommon::make_group_info(); // Создаём группу (в кеш добавлять не обязательно)
    std::shared_ptr<hmcommon::HMUserInfo> NewUser = testscommon::make_user_info(); // Создаём пользовтаеля (в кеш добавлять не обязательно)

    // Кешируем полный список
    std::shared_ptr<std::set<QUuid>> UserUuids = std::make_shared<std::set<QUuid>>(); // Формируем пустой перечень участников группы

    Error = inCachedDataStorage->setGroupUsers(NewGroup->m_uuid, UserUuids); // Задаём перечень участников группы
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = inCachedDataStorage->addGroupUser(NewGroup->m_uuid, NewUser->m_uuid); // Пытаемся добавить пользователя в группу
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<std::set<QUuid>> GroupUsers = inCachedDataStorage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем перечень пользователей группы
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(GroupUsers, nullptr); // Должен вернуться валидный указаетль
    EXPECT_NE(GroupUsers->find(NewUser->m_uuid), GroupUsers->cend()); // Пользователь должен быть успешно найден в перечне участников

    Error = inCachedDataStorage->removeGroupUser(NewGroup->m_uuid, NewUser->m_uuid); // Пытаемся удалить пользователя из группы
    ASSERT_FALSE(Error); // Ошибки быть не должно

    GroupUsers = inCachedDataStorage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем перечень пользователей группы
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(GroupUsers, nullptr); // Должен вернуться валидный указаетль
    EXPECT_EQ(GroupUsers->find(NewUser->m_uuid), GroupUsers->cend()); // Пользователь не должен быть найден в перечне участников

    inCachedDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief CachedDataStorage_ClearGroupUsersTest - Тест физического хранилища, проверяющий очистку участников группы
 * @param inCachedDataStorage - Тестируемое физическое хранилище
 */
void CachedDataStorage_ClearGroupUsersTest(std::unique_ptr<HMDataStorage> inCachedDataStorage)
{
    std::error_code Error;

    Error = inCachedDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inCachedDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroupInfo> NewGroup = testscommon::make_group_info(); // Создаём группу (в кеш добавлять не обязательно)

    const std::size_t UsersCount = 5; // Контейнер, хранащий пользователей
    std::shared_ptr<std::set<QUuid>> UserUUIDs = std::make_shared<std::set<QUuid>>(); // Контейнер UUID'ов пользователей

    for (size_t Index = 0; Index < UsersCount; ++Index)
    {
        QString TestUserLogin = "TestUser" + QString::number(Index); // Логины тестовых пользователей должны быть уникальными
        std::shared_ptr<hmcommon::HMUserInfo> User = testscommon::make_user_info(QUuid::createUuid(), TestUserLogin); // Создаём пользователя (в кеш добавлять не обязательно)

        UserUUIDs->insert(User->m_uuid); // Запоминаем UUID добавленого в хранилище пользователя
    }

    Error = inCachedDataStorage->setGroupUsers(NewGroup->m_uuid, UserUUIDs); // Пытаемся добавить пользователей в группу
    ASSERT_FALSE(Error); // Ошибки быть не должно

    std::shared_ptr<std::set<QUuid>> GroupUsers = inCachedDataStorage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем список участников группы
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(UserUUIDs, nullptr); // Должен вернуться валидный указаетль
    EXPECT_EQ(*UserUUIDs, *GroupUsers); // Перечни UUID'ов должны совпасть

    Error = inCachedDataStorage->clearGroupUsers(NewGroup->m_uuid); // Очищаем перечень участников группы
    ASSERT_FALSE(Error); // Ошибки быть не должно

    GroupUsers = inCachedDataStorage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем список участников группы
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(GroupUsers, nullptr); // Должен вернуться валидный указаетль
    EXPECT_TRUE(GroupUsers->empty()); // Перечень участников группы должен быть пуст

    inCachedDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief CachedDataStorage_GetGroupUserListTest - Тест физического хранилища, проверяющий запрос списка участников группы
 * @param inCachedDataStorage - Тестируемое физическое хранилище
 */
void CachedDataStorage_GetGroupUserListTest(std::unique_ptr<HMDataStorage> inCachedDataStorage)
{
    std::error_code Error;

    Error = inCachedDataStorage->open();
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(inCachedDataStorage->is_open()); // Хранилище должно считаться открытым

    std::shared_ptr<hmcommon::HMGroupInfo> NewGroup = testscommon::make_group_info(); // Создаём группу

    std::shared_ptr<std::set<QUuid>> GroupUsers = inCachedDataStorage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем список участников группы
    ASSERT_EQ(Error.value(), static_cast<int32_t>(eDataStorageError::dsGroupUserRelationNotExists)); // Получаем метку, связь не кеширована
    ASSERT_EQ(GroupUsers, nullptr); // Должен вернуться валидный указаетль

    const std::size_t UsersCount = 5; // Контейнер, хранащий пользователей
    std::shared_ptr<std::set<QUuid>> UserUUIDs = std::make_shared<std::set<QUuid>>(); // Контейнер UUID'ов пользователей

    for (size_t Index = 0; Index < UsersCount; ++Index)
    {
        QString TestUserLogin = "TestUser" + QString::number(Index); // Логины тестовых пользователей должны быть уникальными
        std::shared_ptr<hmcommon::HMUserInfo> User = testscommon::make_user_info(QUuid::createUuid(), TestUserLogin); // Создаём пользователя (в кеш добавлять не обязательно)
        UserUUIDs->insert(User->m_uuid); // Запоминаем UUID добавленого в хранилище пользователя
    }

    Error = inCachedDataStorage->setGroupUsers(NewGroup->m_uuid, UserUUIDs); // Пытаемся добавить пользователей в группу
    ASSERT_FALSE(Error); // Ошибки быть не должно

    GroupUsers = inCachedDataStorage->getGroupUserList(NewGroup->m_uuid, Error); // Запрашиваем список участников группы
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_NE(UserUUIDs, nullptr); // Должен вернуться валидный указаетль
    EXPECT_EQ(*UserUUIDs, *GroupUsers); // Перечни UUID'ов должны совпасть

    inCachedDataStorage->close();
}
//-----------------------------------------------------------------------------
/**
 * @brief CachedDataStorage_AddMessageTest - Тест физического хранилища, проверяющий добавление сообщения
 * @param inCachedDataStorage - Тестируемое физическое хранилище
 */
void CachedDataStorage_AddMessageTest(std::unique_ptr<HMDataStorage> inCachedDataStorage)
{
    ASSERT_TRUE(true); // Кеширование сообщений не поддерживается
}
//-----------------------------------------------------------------------------
/**
 * @brief CachedDataStorage_UpdateMessageTest - Тест физического хранилища, проверяющий обновление сообщения
 * @param inCachedDataStorage - Тестируемое физическое хранилище
 */
void CachedDataStorage_UpdateMessageTest(std::unique_ptr<HMDataStorage> inCachedDataStorage)
{
    ASSERT_TRUE(true); // Кеширование сообщений не поддерживается
}
//-----------------------------------------------------------------------------
/**
 * @brief CachedDataStorage_FindMessageTest - Тест физического хранилища, проверяющий поиск сообщения
 * @param inCachedDataStorage - Тестируемое физическое хранилище
 */
void CachedDataStorage_FindMessageTest(std::unique_ptr<HMDataStorage> inCachedDataStorage)
{
    ASSERT_TRUE(true); // Кеширование сообщений не поддерживается
}
//-----------------------------------------------------------------------------
/**
 * @brief CachedDataStorage_FindMessagesTest - Тест физического хранилища, проверяющий поиск сообщений
 * @param inCachedDataStorage - Тестируемое физическое хранилище
 */
void CachedDataStorage_FindMessagesTest(std::unique_ptr<HMDataStorage> inCachedDataStorage)
{
    ASSERT_TRUE(true); // Кеширование сообщений не поддерживается
}
//-----------------------------------------------------------------------------
/**
 * @brief CachedDataStorage_RemoveMessageTest - Тест физического хранилища, проверяющий удаление сообщения
 * @param inCachedDataStorage - Тестируемое физическое хранилище
 */
void CachedDataStorage_RemoveMessageTest(std::unique_ptr<HMDataStorage> inCachedDataStorage)
{
    ASSERT_TRUE(true); // Кеширование сообщений не поддерживается
}
//-----------------------------------------------------------------------------

#endif // HAWKSERVERCORECACHEDDATASTORAGETEST_HPP
