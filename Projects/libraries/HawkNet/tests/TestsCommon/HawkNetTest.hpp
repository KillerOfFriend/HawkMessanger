#include <gtest/gtest.h>

#include <thread>
#include <chrono>
#include <random>

#include <QCoreApplication>

#include <neterrorcategory.h>
#include <HawkNet.h>

//-----------------------------------------------------------------------------
// Const
//-----------------------------------------------------------------------------
static const std::string C_HOST = "127.0.0.1";              ///< Адрес хоста по умолчанию
constexpr std::uint16_t C_PORT = 57427;                     ///< Порт по умолчанию
// Условные единицы времени ожидания
constexpr auto C_PROC_STOP = std::chrono::milliseconds(10); ///< Пауза между обработками    10ms
constexpr auto C_WAIT_FAST = C_PROC_STOP * 2;               ///< Короткое ожидание          20ms
constexpr auto C_WAIT_NORM = C_PROC_STOP * 5;               ///< Нормальное ожидание        50ms
constexpr auto C_WAIT_LONG = C_PROC_STOP * 10;              ///< Долгое ожидание            100ms
//-----------------------------------------------------------------------------
// Builder
//-----------------------------------------------------------------------------
/**
 * @brief The NetTestBuilder class - Интерфейс класса-конструктора объектов теста
 */
class NetTestBuilder
{
public:

    /**
     * @brief NetTestBuilder - Конскруктор по умолчанию
     */
    NetTestBuilder() = default;

    /**
     * @brief ~NetTestBuilder - Виртуальный деструктор по умолчанию
     */
    virtual ~NetTestBuilder() = default;

    /**
     * @brief make_server - Метод сконструирует сервер
     * @param inCallBacks - Калбэки сервера
     * @param inPort - Прослушиваемый порт
     * @return Вернёт указатель на интерфейс сервера
     */
    virtual std::unique_ptr<net::HMServer> make_server(net::ServCallbacks& inCallBacks,
                                                       const std::uint16_t inPort = C_PORT) = 0;

    /**
     * @brief make_client - Метод сконструерует соединение
     * @param inCallBacks - Калбэки соединения
     * @param inHost - Адрес хоста
     * @param inPort - Порт хоста
     * @return Веренёт указатель на интерфейс соединения
     */
    virtual std::unique_ptr<net::HMConnection> make_client(net::ConCallbacks& inCallBacks,
                                                           const std::string inHost = C_HOST,
                                                           const std::uint16_t inPort = C_PORT) = 0;

    /**
     * @brief wait - Метод выполнит задержку потока для ожидания
     * @param inWaitTime - Время ожидания
     */
    virtual void wait(const std::chrono::milliseconds& inWaitTime) = 0;
};
//-----------------------------------------------------------------------------
// Tests
//-----------------------------------------------------------------------------
/**
 * @brief HawkNet_ServerStart - Тест создания сервера
 * @param inBuilder - Сборщик объектов теста
 */
void HawkNet_ServerStart(std::unique_ptr<NetTestBuilder>&& inBuilder)
{
    ASSERT_TRUE(inBuilder); // Требуется валидный билдер
    errors::error_code Error; // Метка ошибки
    net::ServCallbacks EmptpCallBacks; // Пустые обработчики

    std::unique_ptr<net::HMServer> Server = inBuilder->make_server(EmptpCallBacks, C_PORT); // Создаём сервер
    ASSERT_FALSE(Server->isStarted()); // Сервер должен считаться не запущеным

    Error = Server->start(); // Пытаемся запустить сервер
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Server->isStarted()); // Сервер должен считаться запущеным

    {
        std::unique_ptr<net::HMServer> SecondServer = inBuilder->make_server(EmptpCallBacks, C_PORT); // Создаём второ сервер на том же потре
        Error = SecondServer->start(); // Пытаемся запустить сервер
        ASSERT_EQ(Error.value(), static_cast<std::int32_t>(errors::eNetError::neStartListenFail)); // Должны полуить ошибку т.к. порт занят
        ASSERT_FALSE(SecondServer->isStarted()); // Сервер должен считаться не запущеным
    }

    Server->stop();
    ASSERT_FALSE(Server->isStarted()); // Сервер должен считаться не запущеным
}
//-----------------------------------------------------------------------------
/**
 * @brief HawkNet_ClientConnect - Тест подключения клиента
 * @param inBuilder - Сборщик объектов теста
 */
void HawkNet_ClientConnect(std::unique_ptr<NetTestBuilder>&& inBuilder)
{
    ASSERT_TRUE(inBuilder); // Требуется валидный билдер
    errors::error_code Error; // Метка ошибки
    net::ServCallbacks ServCallBacks;
    net::ConCallbacks ClientCallBacks;

    std::unique_ptr<net::HMConnection> Client = inBuilder->make_client(ClientCallBacks);
    Error = Client->connect(); // Пытаемся подключится к несуществующему серверу
    ASSERT_EQ(Error.value(), static_cast<std::int32_t>(errors::eNetError::neTimeOut));
    ASSERT_FALSE(Client->isConnected()); // Клиент должен считаться не подключённым

    inBuilder->wait(C_WAIT_FAST); // Ожидаем

    std::unique_ptr<net::HMServer> Server = inBuilder->make_server(ServCallBacks); // Создаём сервер
    Error = Server->start(); // Пытаемся запустить сервер
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Server->isStarted()); // Сервер должен считаться запущеным
    ASSERT_EQ(Server->connectionCount(), 0); // Подключений быть не должно

    inBuilder->wait(C_WAIT_FAST); // Ожидаем

    Error = Client->connect(); // Пытаемся подключится
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Client->isConnected()); // Клиент должен считаться подключённым

    inBuilder->wait(C_WAIT_FAST); // Ожидаем

    ASSERT_EQ(Server->connectionCount(), 1); // Должно быть 1 подключение

    Client->disconnect();
    inBuilder->wait(C_WAIT_NORM); // Ожидаем
    ASSERT_FALSE(Client->isConnected()); // Клиент должен считаться не подключённым
    ASSERT_EQ(Server->connectionCount(), 0); // Подключений быть не должно

    Server->stop();
    ASSERT_FALSE(Server->isStarted()); // Сервер должен считаться не запущеным
}
//-----------------------------------------------------------------------------
/**
 * @brief HawkNet_CliendReconnect - Тест подключения клиента
 * @param inBuilder - Сборщик объектов теста
 */
void HawkNet_CliendReconnect(std::unique_ptr<NetTestBuilder>&& inBuilder)
{
    ASSERT_TRUE(inBuilder); // Требуется валидный билдер
    errors::error_code Error; // Метка ошибки
    net::ServCallbacks ServCallBacks;
    net::ConCallbacks ClientCallBacks;

    std::unique_ptr<net::HMServer> Server = inBuilder->make_server(ServCallBacks, C_PORT); // Создаём сервер
    Error = Server->start(); // Пытаемся запустить сервер
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Server->isStarted()); // Сервер должен считаться запущеным
    ASSERT_EQ(Server->connectionCount(), 0); // Подключений быть не должно

    inBuilder->wait(C_WAIT_FAST); // Ожидаем

    std::unique_ptr<net::HMConnection> Client = inBuilder->make_client(ClientCallBacks);
    Error = Client->connect(); // Пытаемся подключится к серверу
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Client->isConnected()); // Клиент должен считаться подключённым

    inBuilder->wait(C_WAIT_FAST); // Ожидаем
    ASSERT_EQ(Server->connectionCount(), 1); // Должно быть 1 подключение

    Error = Client->connect(); // Подключаемся, будучи уже подключёнными
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Client->isConnected()); // Клиент должен считаться подключённым

    inBuilder->wait(C_WAIT_FAST); // Ожидаем
    ASSERT_EQ(Server->connectionCount(), 1); // Должно быть 1 подключение

    Client->disconnect(); // Разрываем соедиение со стороны клиента
    inBuilder->wait(C_WAIT_FAST); // Ожидаем

    Server->stop();
}
//-----------------------------------------------------------------------------
/**
 * @brief HawkNet_ClientDisconnect - Тест отключения клиента
 * @param inBuilder - Сборщик объектов теста
 */
void HawkNet_ClientDisconnect(std::unique_ptr<NetTestBuilder>&& inBuilder)
{
    ASSERT_TRUE(inBuilder); // Требуется валидный билдер
    errors::error_code Error; // Метка ошибки
    std::size_t ConnectionID = 0; // Идентификатор подключения

    net::ServCallbacks ServCallBacks;
    net::ConCallbacks ClientCallBacks;
    ServCallBacks.m_NewConnectionCallBack = [&](const std::size_t inID) -> void { ConnectionID = inID; };

    std::unique_ptr<net::HMServer> Server = inBuilder->make_server(ServCallBacks, C_PORT); // Создаём сервер
    Error = Server->start(); // Пытаемся запустить сервер
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Server->isStarted()); // Сервер должен считаться запущеным
    ASSERT_EQ(Server->connectionCount(), 0); // Подключений быть не должно

    inBuilder->wait(C_WAIT_FAST); // Ожидаем

    std::unique_ptr<net::HMConnection> Client = inBuilder->make_client(ClientCallBacks);
    Error = Client->connect(); // Пытаемся подключится к серверу
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Client->isConnected()); // Клиент должен считаться подключённым

    inBuilder->wait(C_WAIT_FAST); // Ожидаем
    ASSERT_EQ(Server->connectionCount(), 1); // Должно быть 1 подключение

    Client->disconnect();
    inBuilder->wait(C_WAIT_NORM); // Ожидаем

    ASSERT_EQ(Server->connectionCount(), 0); // Должно быть 0 подключений

    Error = Client->connect(); // Пытаемся переподключится к серверу
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Client->isConnected()); // Клиент должен считаться подключённым

    inBuilder->wait(C_WAIT_FAST); // Ожидаем
    ASSERT_EQ(Server->connectionCount(), 1); // Должно быть 1 подключение
    ASSERT_NE(ConnectionID, 0); // Идентификатор подключения должен быть проинициализирован
    ASSERT_TRUE(Server->connected(ConnectionID)); // Подключение должно быть зарегестрировано

    Client = nullptr; // Принудительный сброс клиента
    inBuilder->wait(C_WAIT_FAST); // Ожидаем
    ASSERT_FALSE(Server->connected(ConnectionID)); // Подключение не должно быть зарегестрировано
    ASSERT_EQ(Server->connectionCount(), 0); // Должно быть 0 подключений

    Server->stop();
}
//-----------------------------------------------------------------------------
/**
 * @brief HawkNet_ForcedDisconnectClient - Тест отключения всех клиентов
 * @param inBuilder - Сборщик объектов теста
 */
void HawkNet_ForcedDisconnectClient(std::unique_ptr<NetTestBuilder>&& inBuilder)
{
    ASSERT_TRUE(inBuilder); // Требуется валидный билдер
    errors::error_code Error; // Метка ошибки
    std::size_t ConnectionID = 0; // Идентификатор подключения

    net::ServCallbacks ServCallBacks;
    net::ConCallbacks ClientCallBacks;
    ServCallBacks.m_NewConnectionCallBack = [&](const std::size_t inID) -> void { ConnectionID = inID; };

    std::unique_ptr<net::HMServer> Server = inBuilder->make_server(ServCallBacks); // Создаём сервер
    Error = Server->start(); // Пытаемся запустить сервер
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Server->isStarted()); // Сервер должен считаться запущеным
    ASSERT_EQ(Server->connectionCount(), 0); // Подключений быть не должно

    inBuilder->wait(C_WAIT_FAST); // Ожидаем

    std::unique_ptr<net::HMConnection> Client = inBuilder->make_client(ClientCallBacks);
    Error = Client->connect(); // Пытаемся подключится к серверу
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Client->isConnected()); // Клиент должен считаться подключённым

    inBuilder->wait(C_WAIT_FAST); // Ожидаем
    ASSERT_EQ(Server->connectionCount(), 1); // Должно быть 1 подключение
    ASSERT_NE(ConnectionID, 0); // Идентификатор подключения должен быть проинициализирован
    ASSERT_TRUE(Server->connected(ConnectionID)); // Подключение должно быть зарегестрировано

    Server->closeConnection(ConnectionID); // Закрываем соединение со стороны сервера
    inBuilder->wait(C_WAIT_FAST); // Ожидаем
    ASSERT_EQ(Server->connectionCount(), 0); // Должно быть 0 подключений
    ASSERT_FALSE(Client->isConnected()); // Клиент должен считаться не подключённым

    Server->stop();
}
//-----------------------------------------------------------------------------
/**
 * @brief HawkNet_ForcedDisconnectAllClients - Тест отключения всех клиентов
 * @param inBuilder - Сборщик объектов теста
 */
void HawkNet_ForcedDisconnectAllClients(std::unique_ptr<NetTestBuilder>&& inBuilder)
{
    ASSERT_TRUE(inBuilder); // Требуется валидный билдер
    errors::error_code Error; // Метка ошибки
    net::ServCallbacks ServCallBacks;
    net::ConCallbacks ClientCallBacks;

    std::unique_ptr<net::HMServer> Server = inBuilder->make_server(ServCallBacks); // Создаём сервер
    Error = Server->start(); // Пытаемся запустить сервер
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Server->isStarted()); // Сервер должен считаться запущеным
    ASSERT_EQ(Server->connectionCount(), 0); // Подключений быть не должно

    inBuilder->wait(C_WAIT_FAST); // Ожидаем

    std::vector<std::unique_ptr<net::HMConnection>> Clients(20); // Подключаемые клиенты

    for (auto& Client : Clients) // Подключаем всех клиентов
    {
        Client = inBuilder->make_client(ClientCallBacks);
        Error = Client->connect(); // Пытаемся подключится к серверу
        ASSERT_FALSE(Error); // Ошибки быть не должно
        ASSERT_TRUE(Client->isConnected()); // Клиент дaолжен считаться подключённым

        inBuilder->wait(C_WAIT_FAST); // Ожидаем
    }

    ASSERT_EQ(Server->connectionCount(), Clients.size()); // Должно быть соответствующее количество подключений

    Server->closeAllConnections(); // Принудительно закрываем все соединения
    inBuilder->wait(C_WAIT_NORM); // Ожидаем
    ASSERT_EQ(Server->connectionCount(), 0); // Должно быть 0 подключений

    for (auto& Client : Clients) // Все подключения должны считаться отключёнными
        ASSERT_FALSE(Client->isConnected());

    Server->stop();
}
//-----------------------------------------------------------------------------
/**
 * @brief HawkNet_ServerStop - Тест отключения сервера
 * @param inBuilder - Сборщик объектов теста
 */
void HawkNet_ServerStop(std::unique_ptr<NetTestBuilder>&& inBuilder)
{
    ASSERT_TRUE(inBuilder); // Требуется валидный билдер
    errors::error_code Error; // Метка ошибки
    net::ServCallbacks ServCallBacks;
    net::ConCallbacks ClientCallBacks;

    errors::error_code onClient_ServerStopError; // Ошибка, возникшая при остановке сервера

    ClientCallBacks.m_ErrorCallBack = [&](const errors::error_code inError, const std::size_t) { onClient_ServerStopError = inError; };

    std::unique_ptr<net::HMServer> Server = inBuilder->make_server(ServCallBacks, C_PORT); // Создаём сервер
    Error = Server->start(); // Пытаемся запустить сервер
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Server->isStarted()); // Сервер должен считаться запущеным
    ASSERT_EQ(Server->connectionCount(), 0); // Подключений быть не должно

    inBuilder->wait(C_WAIT_FAST); // Ожидаем

    std::unique_ptr<net::HMConnection> Client = inBuilder->make_client(ClientCallBacks);
    Error = Client->connect(); // Пытаемся подключится к серверу
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Client->isConnected()); // Клиент должен считаться подключённым

    inBuilder->wait(C_WAIT_FAST); // Ожидаем
    ASSERT_EQ(Server->connectionCount(), 1); // Должно быть 1 подключение

    Server->stop();
    inBuilder->wait(C_WAIT_FAST); // Ожидаем

    ASSERT_FALSE(Client->isConnected()); // Клиент не должен считаться подключённым
}
//-----------------------------------------------------------------------------
/**
 * @brief HawkNet_CommonDisconnectEvents - Тест обработки отключения клиента с обеих сторон
 * @param inBuilder - Сборщик объектов теста
 */
void HawkNet_CommonDisconnectEvents(std::unique_ptr<NetTestBuilder>&& inBuilder)
{
    ASSERT_TRUE(inBuilder); // Требуется валидный билдер
    errors::error_code Error; // Метка ошибки

    // Инициализируем обрабутку событий
    bool OnClient_Disconnect = false; // Клиент обработал собылие отключения

    net::ConCallbacks ClientCallBacks; // События клиента
    ClientCallBacks.m_DisconnectCallBack =  [&](const std::size_t) -> void                  { OnClient_Disconnect = true; };

    bool OnServ_ClientDisconnect = false; // Сервер обработал событие отключения клиента

    net::ServCallbacks ServCallBacks; // События сервера
    ServCallBacks.m_conCalbacks.m_DisconnectCallBack =  [&](const std::size_t) -> void      { OnServ_ClientDisconnect = true; };

    // Инициализируем соединения
    std::unique_ptr<net::HMServer> Server = inBuilder->make_server(ServCallBacks); // Создаём сервер
    Error = Server->start(); // Пытаемся запустить сервер
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Server->isStarted()); // Сервер должен считаться запущеным

    inBuilder->wait(C_WAIT_FAST); // Ожидаем

    std::unique_ptr<net::HMConnection> Client = inBuilder->make_client(ClientCallBacks);
    Error = Client->connect(); // Пытаемся подключится
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Client->isConnected()); // Клиент должен считаться подключённым

    inBuilder->wait(C_WAIT_FAST); // Ожидаем

    // Все события должны оставаться не активными
    ASSERT_FALSE(OnServ_ClientDisconnect);
    ASSERT_FALSE(OnClient_Disconnect);

    Client->disconnect(); // Разрываем соедиение со стороны клиента
    inBuilder->wait(C_WAIT_NORM); // Ожидаем

    // События отключения должны активироваться
    ASSERT_TRUE(OnServ_ClientDisconnect);
    ASSERT_TRUE(OnClient_Disconnect);

    Server->stop();
}
//-----------------------------------------------------------------------------
/**
 * @brief HawkNet_SetConnectionID - Тест присвоения идентификатора соединения
 * @param inBuilder - Сборщик объектов теста
 */
void HawkNet_SetConnectionID(std::unique_ptr<NetTestBuilder>&& inBuilder)
{
    ASSERT_TRUE(inBuilder); // Требуется валидный билдер
    errors::error_code Error; // Метка ошибки
    std::vector<std::size_t> ConnectionsID; // Контейнер идентификаторов соединений

    // Инициализируем обрабутку событий
    net::ServCallbacks ServCallBacks; // События сервера
    ServCallBacks.m_NewConnectionCallBack = [&](const std::size_t inID) -> void { ConnectionsID.push_back(inID); };

    net::ConCallbacks ClientCallBacks; // События клиент

    // Инициализируем соединения
    std::unique_ptr<net::HMServer> Server = inBuilder->make_server(ServCallBacks); // Создаём сервер
    Error = Server->start(); // Пытаемся запустить сервер
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Server->isStarted()); // Сервер должен считаться запущеным

    inBuilder->wait(C_WAIT_FAST); // Ожидаем

    // Подключаем первого клиента
    std::unique_ptr<net::HMConnection> Client1 = inBuilder->make_client(ClientCallBacks);
    Error = Client1->connect(); // Пытаемся подключится
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Client1->isConnected()); // Клиент должен считаться подключённым

    inBuilder->wait(C_WAIT_FAST); // Ожидаем
    ASSERT_EQ(Server->connectionCount(), 1); // Должно быть 1 подключение

    // Подключаем второго клиента
    std::unique_ptr<net::HMConnection> Client2 = inBuilder->make_client(ClientCallBacks);
    Error = Client2->connect(); // Пытаемся подключится
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Client2->isConnected()); // Клиент должен считаться подключённым

    inBuilder->wait(C_WAIT_FAST); // Ожидаем
    ASSERT_EQ(Server->connectionCount(), 2); // Должно быть 2 подключения

    ASSERT_EQ(ConnectionsID.size(), 2); // Должно быть 2 соединения
    ASSERT_NE(ConnectionsID[0], 0); // Идентификатор первого соединения должен быть инициализирован
    ASSERT_NE(ConnectionsID[1], 0); // Идентификатор второго соединения должен быть инициализирован
    std::size_t NewConnectionID = 24680; // Переменная получит идентификатор нового соединения

    Error = Server->changeConnectionID(ConnectionsID[0] - 1, 0); // Пытаемся подменить несуществующий ID
    ASSERT_EQ(Error.value(), static_cast<std::int32_t>(errors::eNetError::neClientNotFound)); // Должны получить ошибку об отсутствии такого ID

    Error = Server->changeConnectionID(ConnectionsID[0], ConnectionsID[1]); // Пытаемся подменить идентификтором, который уже существует
    ASSERT_EQ(Error.value(), static_cast<std::int32_t>(errors::eNetError::neClientIdAlredyExists)); // Должны получить ошибку об существовании такого ID

    Error = Server->changeConnectionID(ConnectionsID[1], NewConnectionID); // Подменяем существующий ID
    ASSERT_FALSE(Error); // Ошибки быть не должно

    Client1->disconnect();
    Client2->disconnect();
    inBuilder->wait(C_WAIT_NORM); // Ожидаем
    Server->stop();
}
//-----------------------------------------------------------------------------
/**
 * @brief HawkNet_ServerToClientSendData - Тест передачи данных: Сервер - клиент
 * @param inBuilder - Сборщик объектов теста
 */
void HawkNet_ServerToClientSendData(std::unique_ptr<NetTestBuilder>&& inBuilder)
{
    ASSERT_TRUE(inBuilder); // Требуется валидный билдер
    errors::error_code Error; // Метка ошибки
    std::string TextData = R"_({ name: "John", age: 31, city: "New York" })_";
    net::iByteStream ClientReceived; // Полученные клиентом данные

    // Инициализируем обрабутку событий
    std::size_t ConnectionID = 0; // Идентификатор соединения
    bool OnServ_ClientError = false; // Сервер, соединение обработало событие "ошибка"
    bool OnClient_Error = false; // Клиент обработал событие "ошибка"

    net::ServCallbacks ServCallBacks; // События сервера
    ServCallBacks.m_conCalbacks.m_ErrorCallBack =   [&](const errors::error_code, const std::size_t) -> void                { OnServ_ClientError = true; };
    ServCallBacks.m_NewConnectionCallBack =         [&](const std::size_t inID) -> void { ConnectionID = inID; };

    net::ConCallbacks ClientCallBacks; // События клиента
    ClientCallBacks.m_DataCallBack =                [&](net::iByteStream&& inData, const std::size_t) -> void               { ClientReceived = std::move(inData); };
    ClientCallBacks.m_ErrorCallBack =               [&](const errors::error_code, const std::size_t) -> void                { OnClient_Error = true; };

    // Инициализируем соединения
    std::unique_ptr<net::HMServer> Server = inBuilder->make_server(ServCallBacks); // Создаём сервер
    Error = Server->start(); // Пытаемся запустить сервер
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Server->isStarted()); // Сервер должен считаться запущеным

    inBuilder->wait(C_WAIT_FAST); // Ожидаем

    std::unique_ptr<net::HMConnection> Client = inBuilder->make_client(ClientCallBacks);
    Error = Client->connect(); // Пытаемся подключится
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Client->isConnected()); // Клиент должен считаться подключённым

    inBuilder->wait(C_WAIT_FAST); // Ожидаем
    ASSERT_NE(ConnectionID, 0); // Идентификатор соединения должен быть инициализирован

    Error = Server->send(ConnectionID - 1, net::oByteStream(TextData)); // Оправляем данные несуществующему клиенту
    ASSERT_EQ(Error.value(), static_cast<std::int32_t>(errors::eNetError::neClientNotFound)); // Должны получить сообщение, что нет такого соединения

    Error = Server->send(ConnectionID, net::oByteStream(TextData)); // Оправляем данные указанному клиенту
    ASSERT_FALSE(Error); // Ошибки быть не должно
    inBuilder->wait(C_WAIT_FAST); // Ожидаем

    ASSERT_EQ(ClientReceived.str(), TextData); // Полученные данные должны совпасть
    // Ошибок быть не должно
    ASSERT_FALSE(OnServ_ClientError);
    ASSERT_FALSE(OnClient_Error);

    Client->disconnect();
    inBuilder->wait(C_WAIT_NORM); // Ожидаем
    Server->stop();
}
//-----------------------------------------------------------------------------
/**
 * @brief HawkNet_ClientToServerSendData - Тест передачи данны: Клиент - сервер
 * @param inBuilder - Сборщик объектов теста
 */
void HawkNet_ClientToServerSendData(std::unique_ptr<NetTestBuilder>&& inBuilder)
{
    ASSERT_TRUE(inBuilder); // Требуется валидный билдер
    errors::error_code Error; // Метка ошибки
    std::string TextData = R"_({ name: "John", age: 31, city: "New York" })_";
    net::iByteStream ServerConnectionReceived; // Полученные сервером данные

    // Инициализируем обрабутку событий
    bool OnServ_ClientError = false; // Сервер, соединение обработало событие "ошибка"
    bool OnClient_Error = false; // Клиент обработал событие "ошибка"

    net::ServCallbacks ServCallBacks; // События сервера
    ServCallBacks.m_conCalbacks.m_DataCallBack =    [&](net::iByteStream&& inData, const std::size_t) -> void               { ServerConnectionReceived = std::move(inData); };
    ServCallBacks.m_conCalbacks.m_ErrorCallBack =   [&](const errors::error_code, const std::size_t) -> void                { OnServ_ClientError = true; };

    net::ConCallbacks ClientCallBacks; // События клиента
    ClientCallBacks.m_ErrorCallBack =               [&](const errors::error_code, const std::size_t) -> void                { OnClient_Error = true; };

    // Инициализируем соединения
    std::unique_ptr<net::HMServer> Server = inBuilder->make_server(ServCallBacks); // Создаём сервер
    Error = Server->start(); // Пытаемся запустить сервер
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Server->isStarted()); // Сервер должен считаться запущеным

    inBuilder->wait(C_WAIT_FAST); // Ожидаем

    std::unique_ptr<net::HMConnection> Client = inBuilder->make_client(ClientCallBacks);
    Error = Client->send(net::oByteStream(TextData));
    ASSERT_EQ(Error.value(), static_cast<std::int32_t>(errors::eNetError::neNotConnected)); // Должны получить сообщение, что нет соединения

    Error = Client->connect(); // Пытаемся подключится
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Client->isConnected()); // Клиент должен считаться подключённым

    inBuilder->wait(C_WAIT_FAST); // Ожидаем

    Error = Client->send(net::oByteStream(TextData));
    ASSERT_FALSE(Error); // Ошибки быть не должно
    inBuilder->wait(C_WAIT_FAST); // Ожидаем

    ASSERT_EQ(ServerConnectionReceived.str(), TextData); // Полученные данные должны совпасть
    // Ошибок быть не должно
    ASSERT_FALSE(OnServ_ClientError);
    ASSERT_FALSE(OnClient_Error);

    Client->disconnect();
    inBuilder->wait(C_WAIT_NORM); // Ожидаем
    Server->stop();
}
//-----------------------------------------------------------------------------
/**
 * @brief HawkNet_Echo - Тест эхо сервера
 * @param inBuilder - Сборщик объектов теста
 */
void HawkNet_Echo(std::unique_ptr<NetTestBuilder>&& inBuilder)
{
    ASSERT_TRUE(inBuilder); // Требуется валидный билдер
    errors::error_code Error; // Метка ошибки
    std::string TextData = R"_({ name: "John", age: 31, city: "New York" })_";
    net::iByteStream EchoConnectionReceived; // Возвращённые данные

    // Инициализируем обрабутку событий
    bool OnServ_ClientError = false; // Сервер, соединение обработало событие "ошибка"
    bool OnClient_Error = false; // Клиент обработал событие "ошибка"
    std::function<std::map<std::size_t, errors::error_code>(net::oByteStream&& inData)> SendToAll; // Функция отправки данных

    net::ServCallbacks ServCallBacks; // События сервера
    ServCallBacks.m_conCalbacks.m_DataCallBack =    [&](net::iByteStream&& inData, const std::size_t) -> void
    {
        net::oByteStream EchoData(inData.str());
        if (!SendToAll) return;
        SendToAll(std::move(EchoData));
        inBuilder->wait(C_WAIT_FAST); // Ожидаем
    };
    ServCallBacks.m_conCalbacks.m_ErrorCallBack =   [&](const errors::error_code, const std::size_t) -> void                { OnServ_ClientError = true; };

    net::ConCallbacks ClientCallBacks; // События клиента
    ClientCallBacks.m_DataCallBack =                [&](net::iByteStream&& inData, const std::size_t) -> void               { EchoConnectionReceived = std::move(inData); };
    ClientCallBacks.m_ErrorCallBack =               [&](const errors::error_code, const std::size_t) -> void                { OnClient_Error = true; };

    // Инициализируем соединения
    std::unique_ptr<net::HMServer> Server = inBuilder->make_server(ServCallBacks); // Создаём сервер
    Error = Server->start(); // Пытаемся запустить сервер
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Server->isStarted()); // Сервер должен считаться запущеным

    inBuilder->wait(C_WAIT_FAST); // Ожидаем
    SendToAll = std::bind(&net::HMServer::sendToAll, Server.get(), std::placeholders::_1); // Определяем функцию отправки всем подключйнным соединениям

    std::unique_ptr<net::HMConnection> Client = inBuilder->make_client(ClientCallBacks);
    Error = Client->connect(); // Пытаемся подключится
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Client->isConnected()); // Клиент должен считаться подключённым

    inBuilder->wait(C_WAIT_FAST); // Ожидаем

    Error = Client->send(net::oByteStream(TextData));
    ASSERT_FALSE(Error); // Ошибки быть не должно
    inBuilder->wait(C_WAIT_FAST); // Ожидаем

    ASSERT_EQ(EchoConnectionReceived.str(), TextData); // Полученные данные должны совпасть
    // Ошибок быть не должно
    ASSERT_FALSE(OnServ_ClientError);
    ASSERT_FALSE(OnClient_Error);

    Client->disconnect();
    inBuilder->wait(C_WAIT_NORM); // Ожидаем
    Server->stop();
}
//-----------------------------------------------------------------------------
/**
 * @brief HawkNet_StressTest - Тест нагрузки
 * @param inBuilder - Сборщик объектов теста
 */
void HawkNet_StressTest(std::unique_ptr<NetTestBuilder>&& inBuilder)
{
    ASSERT_TRUE(inBuilder); // Требуется валидный билдер
    errors::error_code Error; // Метка ошибки
    std::string Data(10240, 'z'); // Формируем длинное сообщение

    std::atomic<std::size_t> DataAcceptCount;
    std::atomic_init(&DataAcceptCount, 0);
    std::atomic<std::size_t> Server_ClientError;
    std::atomic_init(&Server_ClientError, 0);
    std::atomic<std::size_t> ClientError;
    std::atomic_init(&ClientError, 0);

    // Инициализируем обрабутку событий
    net::ServCallbacks ServCallBacks; // События сервера
    ServCallBacks.m_conCalbacks.m_DataCallBack =    [&](net::iByteStream&&, const std::size_t) -> void                  { DataAcceptCount++; };
    ServCallBacks.m_conCalbacks.m_ErrorCallBack =   [&](const errors::error_code, const std::size_t) -> void            { Server_ClientError++; };

    net::ConCallbacks ClientCallBacks; // События клиента
    ClientCallBacks.m_ErrorCallBack =               [&](const errors::error_code, const std::size_t) -> void            { ClientError++; };
    std::vector<std::unique_ptr<net::HMConnection>> Clients(100); // Подключаемые клиенты

    // Инициализируем соединения
    std::unique_ptr<net::HMServer> Server = inBuilder->make_server(ServCallBacks); // Создаём сервер
    Error = Server->start(); // Пытаемся запустить сервер
    ASSERT_FALSE(Error); // Ошибки быть не должно
    ASSERT_TRUE(Server->isStarted()); // Сервер должен считаться запущеным

    inBuilder->wait(C_WAIT_FAST); // Ожидаем

    for (auto& Client : Clients)
    {
        Client = inBuilder->make_client(ClientCallBacks);
        Error = Client->connect();
        ASSERT_FALSE(Error);

        inBuilder->wait(C_WAIT_FAST); // Ожидаем
    }

    ASSERT_EQ(Server->connectionCount(), Clients.size()); // Должно быть 100 подключений

    std::default_random_engine RandonEngine; // Рандомайзер
    std::uniform_int_distribution<std::size_t> Randge(0, Clients.size() - 1); // Диапозон (доступа к клиентам

    std::size_t SendCount = 0; // Счётчик отправленых сообщений
    auto EndOfTime = std::chrono::high_resolution_clock::now() + std::chrono::seconds(5);

    do
    {
        for (std::size_t Index = 0; Index < 5; ++Index)
        {
            Error = Clients[Randge(RandonEngine)]->send(net::oByteStream(Data)); // Отправляем событие случайному клиенту
            ASSERT_FALSE(Error);
            SendCount++;
        }

        inBuilder->wait(C_WAIT_FAST); // Ожидаем
    }
    while (std::chrono::high_resolution_clock::now() < EndOfTime);

    // Ошибок быть не должно
    ASSERT_EQ(Server_ClientError, 0);
    ASSERT_EQ(ClientError, 0);

    ASSERT_EQ(SendCount, DataAcceptCount);

    for (auto& Client : Clients)
        Client->disconnect();

    inBuilder->wait(C_WAIT_LONG); // Ожидаем
    Server->stop();
}
//-----------------------------------------------------------------------------
