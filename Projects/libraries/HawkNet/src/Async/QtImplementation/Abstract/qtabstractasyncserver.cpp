#include "qtabstractasyncserver.h"

#include <neterrorcategory.h>

using namespace net;

//-----------------------------------------------------------------------------
HMQtAbstractAsyncServer::HMQtAbstractAsyncServer(const std::uint16_t inPort, const ServCallbacks& inCallbacks) :
    QObject(nullptr),
    HMAbstractAsyncServer(inCallbacks),
    m_port(inPort)
{

}
//-----------------------------------------------------------------------------
HMQtAbstractAsyncServer::~HMQtAbstractAsyncServer()
{
//    stop();
}
//-----------------------------------------------------------------------------
errors::error_code HMQtAbstractAsyncServer::start()
{
    errors::error_code Error = make_error_code(errors::eNetError::neSuccess); // Изначально метим как успех

    stop(); // Принудительная остановка

    m_server = makeServer(Error); // Формируем сервер

    if (!Error && m_server) // Если сервер успешно сформирован
    {
        Error = serverSigSlotConnect(); // Линкуем сигналы\слоты

        if (!Error) // Если линковка сигналов\слотов прошла успешно
        {
            if (!m_server->listen(QHostAddress::Any, m_port)) // Пытаемся запустить
                Error = make_error_code(errors::eNetError::neStartListenFail);
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
bool HMQtAbstractAsyncServer::isStarted() const
{
    bool Result = false;

    if (m_server) // Если сервер инициализирован
        Result = m_server->isListening(); // Вернём значение на основе его состояния

    return Result;
}
//-----------------------------------------------------------------------------
void HMQtAbstractAsyncServer::stop()
{
    if (!isStarted()) // Если сервер не запущен то нет смысла
        return;

    closeAllConnections(); // Закрываем все соединения

    if (m_server)
        m_server->close();
}
//-----------------------------------------------------------------------------
errors::error_code HMQtAbstractAsyncServer::serverSigSlotConnect()
{
    errors::error_code Error = make_error_code(errors::eNetError::neSuccess); // Изначально метим как успех

    if (!m_server)
        Error = make_error_code(errors::eNetError::neServerNotInit);
    else
    {
        QObject::connect(m_server.get(), &QTcpServer::acceptError, this, &HMQtAbstractAsyncServer::slot_acceptError); // Линкуем событие "Произошла ошибка"
        QObject::connect(m_server.get(), &QTcpServer::newConnection, this, &HMQtAbstractAsyncServer::slot_newConnection); // Линкуем событие "Новое соединение"
    }

    return Error;
}
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
void HMQtAbstractAsyncServer::slot_newConnection()
{
    if (!m_server)
        return;

    // Qt гарантирует удаление сокета, по этому задаём свой делетер, который просто занулит указатель
    QTcpSocketPtr NewSocket(m_server->nextPendingConnection(), [](QTcpSocket* inPtr){ inPtr = nullptr; });
    std::shared_ptr<HMQtAbstractAsyncConnection> NewConnection = makeConnection(std::move(NewSocket)); // Формируем новое соединение

    if (NewConnection) // Если соединение успешно сформировано
        onNewConnection(NewConnection); // Отправляем его на регистрацию
}
//-----------------------------------------------------------------------------
void HMQtAbstractAsyncServer::slot_acceptError(QAbstractSocket::SocketError socketError)
{
    //onError(convertingError(socketError)); // Преобразуем ошибку QtSocket в стандартную и отправляем обработчику
}
//-----------------------------------------------------------------------------
