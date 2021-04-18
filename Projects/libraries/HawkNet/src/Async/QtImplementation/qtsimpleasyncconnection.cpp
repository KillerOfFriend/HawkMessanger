#include "qtsimpleasyncconnection.h"

#include <QHostAddress>

#include <neterrorcategory.h>

using namespace net;

//-----------------------------------------------------------------------------
HMQtSimpleAsyncConnection::HMQtSimpleAsyncConnection(const std::string& inHost, const uint16_t inPort, const ConCallbacks &inCallbacks) :
    QObject(nullptr),
    HMAbstractAsyncConnection(inCallbacks),
    m_host(inHost),
    m_port(inPort)
{
    m_socket = std::make_unique<QTcpSocket>();

    QObject::connect(m_socket.get(), &QTcpSocket::readyRead, this, &HMQtSimpleAsyncConnection::slot_onReadyRead); // Линкуем событие "К чению готов"
    QObject::connect(m_socket.get(), &QTcpSocket::bytesWritten, this, &HMQtSimpleAsyncConnection::slot_onBytesWritten); // Линкуем событие "Байт записано"
    QObject::connect(m_socket.get(), &QTcpSocket::errorOccurred, this, &HMQtSimpleAsyncConnection::slot_onErrorOccurred); // Линкуем событие "Произошла ошибка"
    QObject::connect(m_socket.get(), &QTcpSocket::disconnected, this, &HMQtSimpleAsyncConnection::slot_onDisconnected); // Линкуем событие "разрыв соеденения"
}
//-----------------------------------------------------------------------------
HMQtSimpleAsyncConnection::HMQtSimpleAsyncConnection(QTcpSocketPtr &&inConnection, const ConCallbacks& inCallbacks) :
    QObject(nullptr),
    HMAbstractAsyncConnection(inCallbacks),
    m_socket(std::move(inConnection))
{
    assert(m_socket != nullptr);

    QObject::connect(m_socket.get(), &QTcpSocket::readyRead, this, &HMQtSimpleAsyncConnection::slot_onReadyRead); // Линкуем событие "К чению готов"
    QObject::connect(m_socket.get(), &QTcpSocket::bytesWritten, this, &HMQtSimpleAsyncConnection::slot_onBytesWritten); // Линкуем событие "Байт записано"
    QObject::connect(m_socket.get(), &QTcpSocket::errorOccurred, this, &HMQtSimpleAsyncConnection::slot_onErrorOccurred); // Линкуем событие "Произошла ошибка"
    QObject::connect(m_socket.get(), &QTcpSocket::disconnected, this, &HMQtSimpleAsyncConnection::slot_onDisconnected); // Линкуем событие "разрыв соеденения"

    m_host = m_socket->peerAddress().toString().toStdString();
    m_port = m_socket->localPort();
}
//-----------------------------------------------------------------------------
HMQtSimpleAsyncConnection::~HMQtSimpleAsyncConnection()
{
    disconnect();
}
//-----------------------------------------------------------------------------
errors::error_code HMQtSimpleAsyncConnection::connect(const std::chrono::milliseconds inWaitTime)
{
    errors::error_code Error = make_error_code(errors::eNetError::neSuccess); // Изначально метим как успех

    disconnect();

    m_socket->connectToHost(QString::fromStdString(m_host), static_cast<quint16>(m_port),
                           QIODevice::ReadWrite, QAbstractSocket::AnyIPProtocol);

    if (!m_socket->waitForConnected(inWaitTime.count())) // Ожидаем подключения указанное время
        Error = make_error_code(errors::eNetError::neTimeOut);
    else
    {
        if (!isConnected())
            Error = make_error_code(errors::eNetError::neCreateConnectionFail);
    }

    return Error;
}
//-----------------------------------------------------------------------------
bool HMQtSimpleAsyncConnection::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}
//-----------------------------------------------------------------------------
void HMQtSimpleAsyncConnection::disconnect()
{
    if (!isConnected())
        return;

    m_socket->abort(); // Прерываем текущее действие
    m_socket->close(); // Закрываем соединение
    m_socket->waitForDisconnected();
    // И очищаем буферы
    m_writeBuffer.clear();
    m_readBuffer.clear();

    HMAbstractAsyncConnection::disconnect(); // Вызываем метод предка
}
//-----------------------------------------------------------------------------
void HMQtSimpleAsyncConnection::prepateNextData(oByteStream&& inData)
{
    m_writeBuffer = QByteArray::fromStdString(inData.str()); // Преобразуем в понятный Qt контейнер
}
//-----------------------------------------------------------------------------
void HMQtSimpleAsyncConnection::write()
{
    if (!isConnected() || m_writeBuffer.isEmpty())
        return;

    m_socket->write(m_writeBuffer); // Начинаем отправку
}
//-----------------------------------------------------------------------------
errors::error_code HMQtSimpleAsyncConnection::convertingError(const QAbstractSocket::SocketError inQtSocketError)
{
    errors::eNetError neErrorCode;

    switch (inQtSocketError)
    {
        case QAbstractSocket::ConnectionRefusedError:           { neErrorCode = errors::eNetError::neConnectionRefusedError; break; }
        case QAbstractSocket::RemoteHostClosedError:            { neErrorCode = errors::eNetError::neRemoteHostClosedError; break; }
        case QAbstractSocket::HostNotFoundError:                { neErrorCode = errors::eNetError::neHostNotFoundError; break; }
        case QAbstractSocket::SocketAccessError:                { neErrorCode = errors::eNetError::neSocketAccessError; break; }
        case QAbstractSocket::SocketResourceError:              { neErrorCode = errors::eNetError::neSocketResourceError; break; }
        case QAbstractSocket::SocketTimeoutError:               { neErrorCode = errors::eNetError::neSocketTimeoutError; break; }
        case QAbstractSocket::DatagramTooLargeError:            { neErrorCode = errors::eNetError::neDatagramTooLargeError; break; }
        case QAbstractSocket::NetworkError:                     { neErrorCode = errors::eNetError::neNetworkError; break; }
        case QAbstractSocket::AddressInUseError:                { neErrorCode = errors::eNetError::neAddressInUseError; break; }
        case QAbstractSocket::SocketAddressNotAvailableError:   { neErrorCode = errors::eNetError::neSocketAddressNotAvailableError; break; }
        case QAbstractSocket::UnsupportedSocketOperationError:  { neErrorCode = errors::eNetError::neUnsupportedSocketOperationError; break; }
        case QAbstractSocket::UnfinishedSocketOperationError:   { neErrorCode = errors::eNetError::neUnfinishedSocketOperationError; break; }
        case QAbstractSocket::ProxyAuthenticationRequiredError: { neErrorCode = errors::eNetError::neProxyAuthenticationRequiredError; break; }
        case QAbstractSocket::SslHandshakeFailedError:          { neErrorCode = errors::eNetError::neSslHandshakeFailedError; break; }
        case QAbstractSocket::ProxyConnectionRefusedError:      { neErrorCode = errors::eNetError::neProxyConnectionRefusedError; break; }
        case QAbstractSocket::ProxyConnectionClosedError:       { neErrorCode = errors::eNetError::neProxyConnectionClosedError; break; }
        case QAbstractSocket::ProxyConnectionTimeoutError:      { neErrorCode = errors::eNetError::neProxyConnectionTimeoutError; break; }
        case QAbstractSocket::ProxyNotFoundError:               { neErrorCode = errors::eNetError::neProxyNotFoundError; break; }
        case QAbstractSocket::ProxyProtocolError:               { neErrorCode = errors::eNetError::neProxyProtocolError; break; }
        case QAbstractSocket::OperationError:                   { neErrorCode = errors::eNetError::neOperationError; break; }
        case QAbstractSocket::SslInternalError:                 { neErrorCode = errors::eNetError::neSslInternalError; break; }
        case QAbstractSocket::SslInvalidUserDataError:          { neErrorCode = errors::eNetError::neSslInvalidUserDataError; break; }
        case QAbstractSocket::TemporaryError:                   { neErrorCode = errors::eNetError::neTemporaryError; break; }
        default:                                                { neErrorCode = errors::eNetError::neUnknownQtSocketError; }
    }

    return make_error_code(neErrorCode);
}
//-----------------------------------------------------------------------------
void HMQtSimpleAsyncConnection::slot_onBytesWritten(qint64 inBytes)
{
    m_writeBuffer = m_writeBuffer.mid(inBytes); // Выкидываем из контейнера отправленые байты

    if (!m_writeBuffer.isEmpty()) // Если отправлена не вся последовательность
        write(); // Продолжаем запись
    else // Если вся последовательнось отправлена
        afterWrite(); // Обрабатываем завершение записи
}
//-----------------------------------------------------------------------------
void HMQtSimpleAsyncConnection::slot_onReadyRead()
{
    m_readBuffer += m_socket->readAll(); // Читаем пришедший объём данных

    auto SepPos = m_readBuffer.indexOf(C_DATA_SEPARATOR); // Ищим разделитель потока

    while (SepPos > 0) // Если в буфере найден разделитель потока
    {
        iByteStream OutData(m_readBuffer.left(SepPos).data()); // Формируем поток из полученных данных
        onReadEnd(std::move(OutData)); // Завершаем чтение порции данных

        m_readBuffer = m_readBuffer.mid(SepPos + 1); // Удаляем из буфера прочитанные данные
        SepPos = m_readBuffer.indexOf(C_DATA_SEPARATOR); // Ещё раз ищим разделитель потока
    }
}
//-----------------------------------------------------------------------------
void HMQtSimpleAsyncConnection::slot_onDisconnected()
{
    onDisconnect();
}
//-----------------------------------------------------------------------------
void HMQtSimpleAsyncConnection::slot_onErrorOccurred(QAbstractSocket::SocketError inError)
{
    onError(convertingError(inError)); // Преобразуем ошибку QtSocket в стандартную и отправляем обработчику
}
//-----------------------------------------------------------------------------
