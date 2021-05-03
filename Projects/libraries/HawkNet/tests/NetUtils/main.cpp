#include <gtest/gtest.h>

#include <chrono>

#include <HawkNet.h>
#include <nlohmann/json.hpp>

//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест обёртывания данных не требующих замены символов
 */
TEST(NetUtils, WrapWithoutReplace)
{
    const std::string Data = "01234567890";
    net::oByteStream outS(Data);

    outS = net::wrap(std::move(outS)); // Оборачиваем строку, не содержащую символа разделителя потока

    nlohmann::json Json = nlohmann::json::parse(outS.str(), nullptr, false);
    ASSERT_FALSE(Json.is_discarded());

    EXPECT_TRUE(Json["HEAD"]["replace_sequence"].get<std::string>().empty()); // Замещающая последовательность не задана
    EXPECT_EQ(Json["HEAD"]["replace_count"].get<std::uint64_t>(), 0); // Количество замен = 0
    EXPECT_EQ(Data, Json["DATA"].get<std::string>()); // Запакованные данные должны быть идентичны исзодным
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест обёртывания данных
 */
TEST(NetUtils, Wrap)
{
    const std::string Data = "01234567890";
    const std::string Seq = {'{', net::C_DATA_REPLACER, '}'};

    {
        std::string Data1 = net::C_DATA_SEPARATOR + Data; // Вставляем разделитель в начало

        net::oByteStream outS(Data1);

        outS = net::wrap(std::move(outS)); // Оборачиваем строку

        nlohmann::json Json = nlohmann::json::parse(outS.str(), nullptr, false);
        ASSERT_FALSE(Json.is_discarded());

        const std::string Ret1 = Seq + Data;

        EXPECT_EQ(Json["HEAD"]["replace_sequence"].get<std::string>(), Seq); // Замещающая последовательность идентична
        EXPECT_EQ(Json["HEAD"]["replace_count"].get<std::uint64_t>(), 1); // Количество замен = 1
        EXPECT_EQ(Json["DATA"].get<std::string>(), Ret1); // Запакованные данные должны быть идентичны исходным
    }

    {
        std::string Data2 = Data + net::C_DATA_SEPARATOR; // Вставляем разделитель в конец

        net::oByteStream outS(Data2);

        outS = net::wrap(std::move(outS)); // Оборачиваем строку

        nlohmann::json Json = nlohmann::json::parse(outS.str(), nullptr, false);
        ASSERT_FALSE(Json.is_discarded());

        const std::string Ret2 = Data + Seq;

        EXPECT_EQ(Json["HEAD"]["replace_sequence"].get<std::string>(), Seq); // Замещающая последовательность идентична
        EXPECT_EQ(Json["HEAD"]["replace_count"].get<std::uint64_t>(), 1); // Количество замен = 1
        EXPECT_EQ(Json["DATA"].get<std::string>(), Ret2); // Запакованные данные должны быть идентичны исходным
    }

    {
        std::string Data3 = Data;
        Data3.insert(5, 1, net::C_DATA_SEPARATOR); // Вставляем разделитель в середину

        net::oByteStream outS(Data3);

        outS = net::wrap(std::move(outS)); // Оборачиваем строку

        nlohmann::json Json = nlohmann::json::parse(outS.str(), nullptr, false);
        ASSERT_FALSE(Json.is_discarded());

        std::string Ret3 = Data;
        Ret3.insert(5, Seq); // Вставляем разделитель в середину

        EXPECT_EQ(Json["HEAD"]["replace_sequence"].get<std::string>(), Seq); // Замещающая последовательность идентична
        EXPECT_EQ(Json["HEAD"]["replace_count"].get<std::uint64_t>(), 1); // Количество замен = 1
        EXPECT_EQ(Json["DATA"].get<std::string>(), Ret3); // Запакованные данные должны быть идентичны исходным
    }

    {
        std::string Data4 = Data;
        Data4.insert(5, 1, net::C_DATA_SEPARATOR); // Вставляем разделитель в середину
        Data4 = net::C_DATA_SEPARATOR + Data4 + net::C_DATA_SEPARATOR; // И с обеих сторон

        net::oByteStream outS(Data4);

        outS = net::wrap(std::move(outS)); // Оборачиваем строку

        nlohmann::json Json = nlohmann::json::parse(outS.str(), nullptr, false);
        ASSERT_FALSE(Json.is_discarded());

        std::string Ret3 = Data;
        Ret3.insert(5, Seq); // Вставляем разделитель в середину
        Ret3 = Seq + Ret3 + Seq; // И с обеих сторон

        EXPECT_EQ(Json["HEAD"]["replace_sequence"].get<std::string>(), Seq); // Замещающая последовательность идентична
        EXPECT_EQ(Json["HEAD"]["replace_count"].get<std::uint64_t>(), 3); // Количество замен = 3
        EXPECT_EQ(Json["DATA"].get<std::string>(), Ret3); // Запакованные данные должны быть идентичны исходным
    }
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест развёртывания полученных данных
 */
TEST(NetUtils, UnWrap)
{
    const std::string Data = "01234567890";

    {
        std::string Data1 = net::C_DATA_SEPARATOR + Data; // Вставляем разделитель в начало

        net::oByteStream outS(Data1);

        outS = net::wrap(std::move(outS)); // Оборачиваем строку
        net::iByteStream inS(outS.str()); // Формируем обёрнцтые входные данные
        inS = net::unwrap(std::move(inS)); // Разворачиваем обёртку

        EXPECT_EQ(Data1, inS.str()); // Исходные и развёрнутые данные должны совпасть
    }

    {
        std::string Data2 = Data + net::C_DATA_SEPARATOR; // Вставляем разделитель в начало

        net::oByteStream outS(Data2);

        outS = net::wrap(std::move(outS)); // Оборачиваем строку
        net::iByteStream inS(outS.str()); // Формируем обёрнцтые входные данные
        inS = net::unwrap(std::move(inS)); // Разворачиваем обёртку

        EXPECT_EQ(Data2, inS.str()); // Исходные и развёрнутые данные должны совпасть
    }

    {
        std::string Data3 = Data;
        Data3.insert(5, 1, net::C_DATA_SEPARATOR); // Вставляем разделитель в середину

        net::oByteStream outS(Data3);

        outS = net::wrap(std::move(outS)); // Оборачиваем строку
        net::iByteStream inS(outS.str()); // Формируем обёрнцтые входные данные
        inS = net::unwrap(std::move(inS)); // Разворачиваем обёртку

        EXPECT_EQ(Data3, inS.str()); // Исходные и развёрнутые данные должны совпасть
    }

    {
        std::string Data4 = Data;
        Data4.insert(5, 1, net::C_DATA_SEPARATOR); // Вставляем разделитель в середину
        Data4 = net::C_DATA_SEPARATOR + Data4 + net::C_DATA_SEPARATOR; // И с обеих сторон

        net::oByteStream outS(Data4);

        outS = net::wrap(std::move(outS)); // Оборачиваем строку
        net::iByteStream inS(outS.str()); // Формируем обёрнцтые входные данные
        inS = net::unwrap(std::move(inS)); // Разворачиваем обёртку

        EXPECT_EQ(Data4, inS.str()); // Исходные и развёрнутые данные должны совпасть
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
    return RUN_ALL_TESTS(); // Выполняем тесты
}
//-----------------------------------------------------------------------------
