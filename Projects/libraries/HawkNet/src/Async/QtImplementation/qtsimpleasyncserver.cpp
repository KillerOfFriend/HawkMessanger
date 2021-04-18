#include "qtsimpleasyncserver.h"

#include <neterrorcategory.h>

#include "qtsimpleasyncconnection.h"

using namespace net;

//-----------------------------------------------------------------------------
HMQtSimpleAsyncServer::HMQtSimpleAsyncServer(const uint16_t inPort, const ServCallbacks& inCallbacks) :
    QObject(nullptr),
    HMAbstractAsyncServer(inCallbacks),
    m_port(inPort)
{
    QObject::connect(&m_server, &QTcpServer::acceptError, this, &HMQtSimpleAsyncServer::slot_acceptError); // Линкуем событие "Произошла ошибка"
    QObject::connect(&m_server, &QTcpServer::newConnection, this, &HMQtSimpleAsyncServer::slot_newConnection); // Линкуем событие "Новое соединение"
}
//-----------------------------------------------------------------------------
HMQtSimpleAsyncServer::~HMQtSimpleAsyncServer()
{
    stop();
}
//-----------------------------------------------------------------------------
errors::error_code HMQtSimpleAsyncServer::start()
{
    errors::error_code Error = make_error_code(errors::eNetError::neSuccess); // Изначально метим как успех

    stop();

    if (!m_server.listen(QHostAddress::Any, m_port))
        Error = make_error_code(errors::eNetError::neStartListenFail);

    return Error;
}
//-----------------------------------------------------------------------------
bool HMQtSimpleAsyncServer::isStarted() const
{
    return m_server.isListening();
}
//-----------------------------------------------------------------------------
void HMQtSimpleAsyncServer::stop()
{
    if (!isStarted())
        return;

    closeAllConnections(); // Закрываем все соединения
    m_server.close();
}
//-----------------------------------------------------------------------------


void HMQtSimpleAsyncServer::slot_newConnection()
{
    // Qt гарантирует удаление сокета, по этому задаём свой делетер, который просто занулит указатель
    std::unique_ptr<QTcpSocket, QTcpSocketDeleter> NewSocket(m_server.nextPendingConnection(), [](QTcpSocket* inPtr){ inPtr = nullptr; });

    if (NewSocket)
        onNewConnection(std::make_shared<HMQtSimpleAsyncConnection>(std::move(NewSocket), m_Callbacks.m_conCalbacks)); // Формируем новое соединение и отправляем его на регистрацию
}
//-----------------------------------------------------------------------------
void HMQtSimpleAsyncServer::slot_acceptError(QAbstractSocket::SocketError socketError)
{
    if (m_Callbacks.m_ErrorCallBack)
    {
        errors::error_code Error;
        /*
         * TODO реализовать определение ошибки
         * Конвертация SocketError -> errors::error_code
         * https://doc.qt.io/qt-5/qtnetwork-fortuneclient-example.html
         */
        m_Callbacks.m_ErrorCallBack(Error);
    }
}
//-----------------------------------------------------------------------------
