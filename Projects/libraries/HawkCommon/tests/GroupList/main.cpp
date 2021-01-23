#include <gtest/gtest.h>

#include <grouplist.h>
#include <systemerrorex.h>

#include <HawkCommonTestUtils.hpp>

//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMGroup> make_group()
{
    std::shared_ptr<hmcommon::HMGroup> NewGroup = std::make_shared<hmcommon::HMGroup>();
    NewGroup->m_info = testscommon::make_group_info();
    return NewGroup;
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест создания списка контактов
 */
TEST(GrouptList, Create)
{
    hmcommon::HMGroupList GroupList;

    EXPECT_TRUE(GroupList.isEmpty());
    EXPECT_EQ(GroupList.count(), 0);
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест добавления группы
 */
TEST(GrouptList, CheckAddContact)
{
    errors::error_code Error; // Метка ошибки
    hmcommon::HMGroupList GroupList; // Создаём список групп

    std::shared_ptr<hmcommon::HMGroup> NewGroup = make_group();

    EXPECT_TRUE(GroupList.isEmpty());
    EXPECT_EQ(GroupList.count(), 0);

    Error = GroupList.add(NewGroup); // Добавляем новый контакт
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_FALSE(GroupList.isEmpty());
    EXPECT_EQ(GroupList.count(), 1);
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест поиска не существующей группы
 */
TEST(GrouptList, FindNotExistsContact)
{
    errors::error_code Error; // Метка ошибки
    hmcommon::HMGroupList GroupList; // Создаём список групп

    std::shared_ptr<hmcommon::HMGroup> NewGroup = make_group();

    std::shared_ptr<hmcommon::HMGroup> FindRes = GroupList.get(0, Error);

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_TRUE(Error.value() == static_cast<int32_t>(errors::eSystemErrorEx::seContainerEmpty)); // И метку, что контейнер пуст

    FindRes = GroupList.get(NewGroup->m_info->m_uuid, Error);

    ASSERT_EQ(FindRes, nullptr); // Должен вернуться nullptr
    ASSERT_TRUE(Error.value() == static_cast<int32_t>(errors::eSystemErrorEx::seNotInContainer)); // И метку, что контакт в контейнере не найден
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест поиска существующей группы
 */
TEST(GrouptList, FindExistsContact)
{
    errors::error_code Error; // Метка ошибки
    hmcommon::HMGroupList GroupList; // Создаём список групп

    std::shared_ptr<hmcommon::HMGroup> NewGroup = make_group();

    Error = GroupList.add(NewGroup); // Добавляем новую группу
    ASSERT_FALSE(Error); // Ошибки быть не должно;

    std::shared_ptr<hmcommon::HMGroup> FindRes = GroupList.get(0, Error);

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указаетль
    ASSERT_FALSE(Error); // Ошибки быть не должно

    FindRes = GroupList.get(NewGroup->m_info->m_uuid, Error);

    ASSERT_NE(FindRes, nullptr); // Должен вернуться валидный указаетль
    ASSERT_FALSE(Error); // Ошибки быть не должно
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест удаление контакта
 */
TEST(GrouptList, CheckRemoveContact)
{
    errors::error_code Error; // Метка ошибки
    hmcommon::HMGroupList GroupList; // Создаём список групп

    std::shared_ptr<hmcommon::HMGroup> NewGroup1 = make_group();
    std::shared_ptr<hmcommon::HMGroup> NewGroup2 = make_group();

    Error = GroupList.add(NewGroup1); // Добавляем новую группу
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = GroupList.add(NewGroup2); // Добавляем новую группу
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_FALSE(GroupList.isEmpty());
    EXPECT_EQ(GroupList.count(), 2);

    Error = GroupList.remove(NewGroup1->m_info->m_uuid); // Первую удалим по UUID
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Error = GroupList.remove(0); // Вторую по порядковому номеру (После удаления первого Index == 0)
    ASSERT_FALSE(Error); // Ошибки быть не должно

    EXPECT_TRUE(GroupList.isEmpty());
    EXPECT_EQ(GroupList.count(), 0);
}
//-----------------------------------------------------------------------------
/**
 * @brief main - Входная точка тестировани функционала HMGroupList
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
