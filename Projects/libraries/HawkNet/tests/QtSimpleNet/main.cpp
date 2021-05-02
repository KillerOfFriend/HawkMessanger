#include <gtest/gtest.h>

#include <chrono>

#include <QCoreApplication>

#include <neterrorcategory.h>
#include <HawkNet.h>

#include "HawkNetTest.hpp"

//-----------------------------------------------------------------------------
// Builder
//-----------------------------------------------------------------------------
/**
 * @brief The QtSimpleNetTestBuilder class - Класс-конструктор объектов теста QtNetSimple
 */
class QtSimpleNetTestBuilder : public NetTestBuilder
{
public:

    /**
     * @brief QtSimpleNetTestBuilder - Конскруктор по умолчанию
     */
    QtSimpleNetTestBuilder() = default;

    /**
     * @brief ~QtSimpleNetTestBuilder - Виртуальный деструктор по умолчанию
     */
    virtual ~QtSimpleNetTestBuilder() override = default;

    /**
     * @brief make_server - Метод сконструирует сервер
     * @param inCallBacks - Калбэки сервера
     * @param inPort - Прослушиваемый порт
     * @return Вернёт указатель на интерфейс сервера
     */
    virtual std::unique_ptr<net::HMServer> make_server(net::ServCallbacks& inCallBacks,
                                                       const std::uint16_t inPort = C_PORT) override
    {
        return std::make_unique<net::HMQtSimpleAsyncServer>(inPort, inCallBacks);
    }

    /**
     * @brief make_client - Метод сконструерует соединение
     * @param inCallBacks - Калбэки соединения
     * @param inHost - Адрес хоста
     * @param inPort - Порт хоста
     * @return Веренёт указатель на интерфейс соединения
     */
    virtual std::unique_ptr<net::HMConnection> make_client(net::ConCallbacks& inCallBacks,
                                                           const std::string inHost = C_HOST,
                                                           const std::uint16_t inPort = C_PORT) override
    {
        return std::make_unique<net::HMQtSimpleAsyncConnection>(inHost, inPort, inCallBacks);
    }

    /**
     * @brief wait - Метод выполнит задержку потока для ожидания
     * @param inWaitTime - Время ожидания
     */
    virtual void wait(const std::chrono::milliseconds& inWaitTime) override
    {
        auto TimeOut = std::chrono::high_resolution_clock::now() + inWaitTime; // Определяем точку времени, до которой будем обрабатывать события Qt
        do
        {
            QCoreApplication::processEvents(); // Принудительно обрабатываем события Qt
        }
        while (std::chrono::high_resolution_clock::now() < TimeOut); // Пока текущее время дойдёт до тамаута
    }
};
//-----------------------------------------------------------------------------
// Tests
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест создания сервера
 */
TEST(QtSimpleNet, ServerStart)
{
    HawkNet_ServerStart(std::make_unique<QtSimpleNetTestBuilder>());
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест подключения клиента
 */
TEST(QtSimpleNet, ClientConnect)
{
    HawkNet_ClientConnect(std::make_unique<QtSimpleNetTestBuilder>());
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест подключения клиента
 */
TEST(QtSimpleNet, CliendReconnect)
{
    HawkNet_CliendReconnect(std::make_unique<QtSimpleNetTestBuilder>());
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест отключения клиента
 */
TEST(QtSimpleNet, ClientDisconnect)
{
    HawkNet_ClientDisconnect(std::make_unique<QtSimpleNetTestBuilder>());
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест отключения всех клиентов
 */
TEST(QtSimpleNet, ForcedDisconnectClient)
{
    HawkNet_ForcedDisconnectClient(std::make_unique<QtSimpleNetTestBuilder>());
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест отключения всех клиентов
 */
TEST(QtSimpleNet, ForcedDisconnectAllClients)
{
    HawkNet_ForcedDisconnectAllClients(std::make_unique<QtSimpleNetTestBuilder>());
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест отключения сервера
 */
TEST(QtSimpleNet, ServerStop)
{
    HawkNet_ServerStop(std::make_unique<QtSimpleNetTestBuilder>());
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест обработки отключения клиента с обеих сторон
 */
TEST(QtSimpleNet, CommonDisconnectEvents)
{
    HawkNet_CommonDisconnectEvents(std::make_unique<QtSimpleNetTestBuilder>());
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест присвоения идентификатора соединения
 */
TEST(QtSimpleNet, SetConnectionID)
{
    HawkNet_SetConnectionID(std::make_unique<QtSimpleNetTestBuilder>());
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест передачи данных: Сервер - клиент
 */
TEST(QtSimpleNet, ServerToClientSendData)
{
    HawkNet_ServerToClientSendData(std::make_unique<QtSimpleNetTestBuilder>());
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест передачи данны: Клиент - сервер
 */
TEST(QtSimpleNet, ClientToServerSendData)
{
    HawkNet_ClientToServerSendData(std::make_unique<QtSimpleNetTestBuilder>());
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест эхо сервера
 */
TEST(QtSimpleNet, Echo)
{
    HawkNet_Echo(std::make_unique<QtSimpleNetTestBuilder>());
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест нагрузки
 */
TEST(QtSimpleNet, StressTest)
{
    HawkNet_StressTest(std::make_unique<QtSimpleNetTestBuilder>());
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
    QCoreApplication App(argc, argv); // Обязательно создаём как приложение Qt

    ::testing::InitGoogleTest(&argc, argv);

    std::int32_t TestResult = RUN_ALL_TESTS(); // Выполняем тесты
    App.exit(EXIT_SUCCESS); // Завершаем работу QCoreApplication

    return TestResult; // Вернёт результат выполнения
}
//-----------------------------------------------------------------------------
