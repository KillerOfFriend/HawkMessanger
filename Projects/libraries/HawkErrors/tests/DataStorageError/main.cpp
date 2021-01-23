#include <gtest/gtest.h>

#include <errorcode.h>
#include <datastorageerrorcategory.h>

//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест создания ошибки
 */
TEST(DataStorageError, Create)
{
    errors::error_code Error = make_error_code(errors::eDataStorageError::dsSuccess);

    ASSERT_EQ(errors::C_SUCCESS, Error.value()); // Должен вернуться коду спеха
    ASSERT_FALSE(Error); // Ошибка не должна быть засчитана

    const std::uint32_t Begin = errors::C_CATEGORY_DATASTORAGE_START;
    const std::uint32_t End = static_cast<std::uint32_t>(errors::eDataStorageError::dsCount);

    for (std::uint32_t ErrorCode = Begin; ErrorCode < End; ++ErrorCode)
    {   // Создадим все входящие в категорию ошибки
        Error = make_error_code(static_cast<errors::eDataStorageError>(ErrorCode));

        ASSERT_TRUE(Error); // Ошибка должна быть засчитана
        ASSERT_EQ(ErrorCode, Error.value()); // Код должен соответствовать
        // Преверяем, что сообщение не содержит "неизвестной ошибки"
        std::size_t FindRes = Error.message().find(errors::C_ERROR_UNKNOWN_TEXT);
        ASSERT_EQ(FindRes, std::string::npos); // Результат поиска должен соответствовать "не найдено"
    }
}
//-----------------------------------------------------------------------------
/**
 * @brief main - Входная точка тестировани функционала errors::error_code
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
