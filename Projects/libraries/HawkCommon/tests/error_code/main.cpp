#include <gtest/gtest.h>

#include <errorcode.h>


//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест создания ошибки
 */
TEST(ErrorCode, Create)
{
    hmcommon::error_code Error;
    EXPECT_FALSE(Error); // Созданый пустой объект не должен являться ошибкой

    Error = std::make_error_code(std::errc::bad_address);
    EXPECT_TRUE(Error); // При присвоении значения объект должен являться ошибкой
    EXPECT_EQ(Error.value(), static_cast<std::int32_t>(std::errc::bad_address)); // И код ошибки должен совпасть
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест сравнения
 */
TEST(ErrorCode, Compare)
{
    hmcommon::error_code Error1 = std::make_error_code(std::errc::connection_aborted);
    hmcommon::error_code Error2 = std::make_error_code(std::errc::connection_refused);

    EXPECT_NE(Error1, Error2);
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест присваивания
 */
TEST(ErrorCode, Assignment)
{
    hmcommon::error_code Error1 = std::make_error_code(std::errc::connection_aborted); // Инициализация стандартной ошибкой
    EXPECT_EQ(Error1.value(), static_cast<std::int32_t>(std::errc::connection_aborted)); // И код ошибки должен совпасть
    hmcommon::error_code Error2 = std::make_error_code(std::errc::connection_refused); // Инициализация стандартной ошибкой
    EXPECT_EQ(Error2.value(), static_cast<std::int32_t>(std::errc::connection_refused)); // И код ошибки должен совпасть

    hmcommon::error_code Error3(Error1); // Копирование расширенной ошибки
    EXPECT_EQ(Error3, Error1); // Должны совпасть

    hmcommon::error_code Error4(std::make_error_code(std::errc::connection_refused)); // Копирование стандартной ошибки
    EXPECT_EQ(Error4, Error2); // Должны совпасть

    Error1 = Error2; // Присвоение расширеной ошибки
    EXPECT_EQ(Error1, Error2); // Должны совпасть

    std::error_code StdErr = std::make_error_code(std::errc::connection_reset);
    Error2 = StdErr; // Присвоение стандартной ошибки
    EXPECT_EQ(Error2, StdErr); // Должны совпасть
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест сообщений
 */
TEST(ErrorCode, Messages)
{
    hmcommon::error_code Error = std::make_error_code(std::errc::bad_address);

    std::string StdStr = Error.message();
    EXPECT_EQ(StdStr, "Bad address");

    QString QStr = Error.message_qstr();
    EXPECT_EQ(QStr, "Bad address");

    EXPECT_EQ(StdStr, QStr.toStdString());
}
//-----------------------------------------------------------------------------
/**
 * @brief main - Входная точка тестировани функционала hmcommon::error_code
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
