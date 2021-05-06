#include <gtest/gtest.h>

#include <chrono>

#include <HawkNet.h>
#include <nlohmann/json.hpp>

//-----------------------------------------------------------------------------
static const std::string Data = "0123456789";
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест обёртывания данных не требующих замены символов
 */
TEST(NetUtils, WrapWithoutReplace)
{
    net::oByteStream outS(Data);

    outS = net::wrap(std::move(outS)); // Оборачиваем строку, не содержащую символа разделителя потока
    // Обёртывания не должно произойти
    EXPECT_EQ(outS.str(), Data); // Данные не толжны измениться
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест обёртывания данных
 */
TEST(NetUtils, Wrap)
{
    const std::string Seq = {'{', net::C_DATA_REPLACER, '}'};

    {
        std::string Data1 = net::C_DATA_SEPARATOR + Data; // Вставляем разделитель в начало
        const std::string Ret1 = Seq + Data;

        net::oByteStream outS(Data1);
        outS = net::wrap(std::move(outS)); // Оборачиваем строку

        nlohmann::json WrapInfo;
        std::string WrappedData = "";
        ASSERT_TRUE(net::getWrapAndMessageData(outS.str(), WrapInfo, WrappedData)); // Получаем информацию об обёртке и обёрнутые данные

        EXPECT_EQ(WrapInfo[net::C_WRAP_HEAD][net::C_WRAP_SEQUENCE].get<std::string>(), Seq); // Замещающая последовательность идентична
        EXPECT_EQ(WrapInfo[net::C_WRAP_HEAD][net::C_WRAP_COUNT].get<std::uint64_t>(), 1); // Количество замен = 1
        EXPECT_EQ(WrappedData, Ret1); // Запакованные данные должны быть идентичны ожидаемым
    }

    {
        std::string Data2 = Data + net::C_DATA_SEPARATOR; // Вставляем разделитель в конец
        const std::string Ret2 = Data + Seq;

        net::oByteStream outS(Data2);
        outS = net::wrap(std::move(outS)); // Оборачиваем строку

        nlohmann::json WrapInfo;
        std::string WrappedData = "";
        ASSERT_TRUE(net::getWrapAndMessageData(outS.str(), WrapInfo, WrappedData)); // Получаем информацию об обёртке и обёрнутые данные

        EXPECT_EQ(WrapInfo[net::C_WRAP_HEAD][net::C_WRAP_SEQUENCE].get<std::string>(), Seq); // Замещающая последовательность идентична
        EXPECT_EQ(WrapInfo[net::C_WRAP_HEAD][net::C_WRAP_COUNT].get<std::uint64_t>(), 1); // Количество замен = 1
        EXPECT_EQ(WrappedData, Ret2); // Запакованные данные должны быть идентичны ожидаемым
    }

    {
        std::string Data3 = Data;
        Data3.insert(5, 1, net::C_DATA_SEPARATOR); // Вставляем разделитель в середину

        std::string Ret3 = Data;
        Ret3.insert(5, Seq); // Вставляем разделитель в середину

        net::oByteStream outS(Data3);
        outS = net::wrap(std::move(outS)); // Оборачиваем строку

        nlohmann::json WrapInfo;
        std::string WrappedData = "";
        ASSERT_TRUE(net::getWrapAndMessageData(outS.str(), WrapInfo, WrappedData)); // Получаем информацию об обёртке и обёрнутые данные

        EXPECT_EQ(WrapInfo[net::C_WRAP_HEAD][net::C_WRAP_SEQUENCE].get<std::string>(), Seq); // Замещающая последовательность идентична
        EXPECT_EQ(WrapInfo[net::C_WRAP_HEAD][net::C_WRAP_COUNT].get<std::uint64_t>(), 1); // Количество замен = 1
        EXPECT_EQ(WrappedData, Ret3); // Запакованные данные должны быть идентичны ожидаемым
    }

    {
        std::string Data4 = Data;
        Data4.insert(5, 1, net::C_DATA_SEPARATOR); // Вставляем разделитель в середину
        Data4 = net::C_DATA_SEPARATOR + Data4 + net::C_DATA_SEPARATOR; // И с обеих сторон

        std::string Ret3 = Data;
        Ret3.insert(5, Seq); // Вставляем разделитель в середину
        Ret3 = Seq + Ret3 + Seq; // И с обеих сторон

        net::oByteStream outS(Data4);
        outS = net::wrap(std::move(outS)); // Оборачиваем строку

        nlohmann::json WrapInfo;
        std::string WrappedData = "";
        ASSERT_TRUE(net::getWrapAndMessageData(outS.str(), WrapInfo, WrappedData)); // Получаем информацию об обёртке и обёрнутые данные

        EXPECT_EQ(WrapInfo[net::C_WRAP_HEAD][net::C_WRAP_SEQUENCE].get<std::string>(), Seq); // Замещающая последовательность идентична
        EXPECT_EQ(WrapInfo[net::C_WRAP_HEAD][net::C_WRAP_COUNT].get<std::uint64_t>(), 3); // Количество замен = 3
        EXPECT_EQ(WrappedData, Ret3); // Запакованные данные должны быть идентичны ожидаемым
    }
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест развёртывания полученных данных
 */
TEST(NetUtils, UnWrap)
{
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
 * @brief TEST - Тест иммитирующий отправку\приём данных
 */
TEST(NetUtils, SendReceiveEmitation)
{
    std::string Text = Data;
    Text.insert(5, 1, net::C_DATA_SEPARATOR); // Вставляем разделитель в середину
    Text = net::C_DATA_SEPARATOR + Text + net::C_DATA_SEPARATOR; // И с обеих сторон
    // Суровым ванлайнером имитируем отправкуи\приём с обёртыванием\развёртыванием
    std::string ReceiveText = net::unwrap(net::iByteStream(net::wrap(net::oByteStream(Text)).str())).str();
    ASSERT_EQ(Text, ReceiveText);
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
