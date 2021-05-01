#include "qtsimpleasyncconnection.h"

#include <QHostAddress>

#include <neterrorcategory.h>

using namespace net;

//-----------------------------------------------------------------------------
HMQtSimpleAsyncConnection::HMQtSimpleAsyncConnection(const std::string& inHost, const uint16_t inPort, const ConCallbacks &inCallbacks) :
    HMQtAbstractAsyncConnection(inHost, inPort, inCallbacks, std::make_unique<QTcpSocket>())
{
    errors::error_code ConnectError = connectionSigSlotConnect(); // Линкуем сигналы\сокеты
    assert(!ConnectError); // Сигналы должны слинковаться успешно
    ConnectError.clear();
}
//-----------------------------------------------------------------------------
HMQtSimpleAsyncConnection::HMQtSimpleAsyncConnection(std::unique_ptr<QTcpSocket>&& inSocket, const ConCallbacks& inCallbacks) :
    HMQtAbstractAsyncConnection(std::move(inSocket), inCallbacks)
{
    errors::error_code ConnectError = connectionSigSlotConnect(); // Линкуем сигналы\сокеты
    assert(!ConnectError); // Сигналы должны слинковаться успешно
    ConnectError.clear();
}
//-----------------------------------------------------------------------------
HMQtSimpleAsyncConnection::~HMQtSimpleAsyncConnection()
{
    disconnect();
}
//-----------------------------------------------------------------------------
errors::error_code HMQtSimpleAsyncConnection::connectionSigSlotConnect()
{
    errors::error_code Error = make_error_code(errors::eNetError::neSuccess); // Изначально метим как успех

    if (!m_socket)
        Error = make_error_code(errors::eNetError::neSocketNotInit);
    else
    {
        QObject::connect(m_socket.get(), &QTcpSocket::readyRead, this, &HMQtSimpleAsyncConnection::slot_onReadyRead); // Линкуем событие "К чтению готов"
        QObject::connect(m_socket.get(), &QTcpSocket::bytesWritten, this, &HMQtSimpleAsyncConnection::slot_onBytesWritten); // Линкуем событие "Байт записано"
        QObject::connect(m_socket.get(), &QTcpSocket::errorOccurred, this, &HMQtSimpleAsyncConnection::slot_onErrorOccurred); // Линкуем событие "Произошла ошибка"
        QObject::connect(m_socket.get(), &QTcpSocket::disconnected, this, &HMQtSimpleAsyncConnection::slot_onDisconnected); // Линкуем событие "Разрыв соеденения"
    }

    return Error;
}
//-----------------------------------------------------------------------------
