#include "qtabstractasyncconnection.h"

#include <QHostAddress>

#include <cassert>
#include <neterrorcategory.h>

using namespace net;

//-----------------------------------------------------------------------------
HMQtAbstractAsyncConnection::HMQtAbstractAsyncConnection(const std::string& inHost, const uint16_t inPort, const ConCallbacks& inCallbacks) :
    QObject(nullptr),
    HMAbstractAsyncConnection(inCallbacks),
    m_host(inHost),
    m_port(inPort)
{
    // Тут создавать сокет нельзя!
}
//-----------------------------------------------------------------------------
HMQtAbstractAsyncConnection::HMQtAbstractAsyncConnection(std::unique_ptr<QTcpSocket>&& inSocket, const ConCallbacks& inCallbacks) :
    QObject(nullptr),
    HMAbstractAsyncConnection(inCallbacks),
    m_socket(std::move(inSocket))
{
    assert(m_socket != nullptr); // Сокет должен быть валидным
    errors::error_code ConnectError = connectionSigSlotConnect(); // Линкуем сигналы\сокеты
    assert(!ConnectError); // Сигналы должны слинковаться успешно
    ConnectError.clear();

    m_host = m_socket->peerAddress().toString().toStdString();
    m_port = m_socket->localPort();
}
//-----------------------------------------------------------------------------
errors::error_code HMQtAbstractAsyncConnection::connect(const std::chrono::milliseconds inWaitTime)
{
    errors::error_code Error = make_error_code(errors::eNetError::neSuccess); // Изначально метим как успех

    disconnect(); // Принудительный разрыв соединения

    if (!m_socket) // Если сокет не инициализирован
    {
        m_socket = makeSocket(Error); // Инициализируем новый сокет
        if (!Error) // Если сокет создан
            Error = connectionSigSlotConnect(); // Линкуем сигналы\слоты
    }

    if (!Error && m_socket) // Если сокет успешно сформирован
    {   // Пытаемся подключиться
        m_socket->connectToHost(QString::fromStdString(m_host), static_cast<quint16>(m_port),
                               QIODevice::ReadWrite, QAbstractSocket::AnyIPProtocol);

        if (m_socket->state() != QAbstractSocket::ConnectedState) // Всё ещё не подключён
            if (!m_socket->waitForConnected(inWaitTime.count())) // Ожидаем подключения указанное время
                Error = make_error_code(errors::eNetError::neTimeOut);
    }

    return Error;
}
//-----------------------------------------------------------------------------
void HMQtAbstractAsyncConnection::disconnect()
{
    if (!isConnected())
        return;

    if (m_socket) // Только если сокет инициализирован
    {
        m_socket->abort(); // Прерываем текущее действие
        m_socket->close(); // Закрываем соединение

        if (m_socket->state() != QAbstractSocket::UnconnectedState) // Всё ещё не отключён
            m_socket->waitForDisconnected(); // Ожидаем разрыва соединения
    }

    // И очищаем буферы
    m_writeBuffer.clear();
    m_readBuffer.clear();

    HMAbstractAsyncConnection::disconnect(); // Вызываем метод предка
}
//-----------------------------------------------------------------------------
eConnectionStatus HMQtAbstractAsyncConnection::status() const
{
    eConnectionStatus Result = eConnectionStatus::csUnknown;

    if (m_socket)
    {
        switch (m_socket->state())
        {
            case QTcpSocket::UnconnectedState:  { Result = eConnectionStatus::csDisconnected; break; }
            case QTcpSocket::ClosingState:      { Result = eConnectionStatus::csConnecting; break; }

            case QTcpSocket::HostLookupState:
            case QTcpSocket::ConnectingState:   { Result = eConnectionStatus::csConnecting; break; }

            case QTcpSocket::ConnectedState:    { Result = eConnectionStatus::csConnected; break; }

            default:                            { Result = eConnectionStatus::csUnknown; }
        }
    }

    return Result;
}
//-----------------------------------------------------------------------------
void HMQtAbstractAsyncConnection::prepateNextData(oByteStream&& inData)
{
    m_writeBuffer = QByteArray::fromStdString(inData.str()); // Преобразуем в понятный Qt контейнер
}
//-----------------------------------------------------------------------------
void HMQtAbstractAsyncConnection::write()
{
    if (!isConnected() || m_writeBuffer.isEmpty())
        return;

    m_socket->write(m_writeBuffer); // Начинаем отправку
}
//-----------------------------------------------------------------------------
errors::error_code HMQtAbstractAsyncConnection::convertingError(const QAbstractSocket::SocketError inQtSocketError)
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
errors::error_code HMQtAbstractAsyncConnection::connectionSigSlotConnect()
{
    errors::error_code Error = make_error_code(errors::eNetError::neSuccess); // Изначально метим как успех

    if (!m_socket)
        Error = make_error_code(errors::eNetError::neSocketNotInit);
    else
    {
        QObject::connect(m_socket.get(), &QTcpSocket::readyRead, this, &HMQtAbstractAsyncConnection::slot_onReadyRead); // Линкуем событие "К чтению готов"
        QObject::connect(m_socket.get(), &QTcpSocket::bytesWritten, this, &HMQtAbstractAsyncConnection::slot_onBytesWritten); // Линкуем событие "Байт записано"
        QObject::connect(m_socket.get(), &QTcpSocket::errorOccurred, this, &HMQtAbstractAsyncConnection::slot_onErrorOccurred); // Линкуем событие "Произошла ошибка"
        QObject::connect(m_socket.get(), &QTcpSocket::disconnected, this, &HMQtAbstractAsyncConnection::slot_onDisconnected); // Линкуем событие "Разрыв соеденения"
    }

    return Error;
}
//-----------------------------------------------------------------------------
void HMQtAbstractAsyncConnection::slot_onBytesWritten(qint64 inBytes)
{
    m_writeBuffer = m_writeBuffer.mid(inBytes); // Выкидываем из контейнера отправленые байты

    if (!m_writeBuffer.isEmpty()) // Если отправлена не вся последовательность
        write(); // Продолжаем запись
    else // Если вся последовательнось отправлена
        afterWrite(); // Обрабатываем завершение записи
}
//-----------------------------------------------------------------------------
void HMQtAbstractAsyncConnection::slot_onReadyRead()
{
    if (!m_socket)
        return;

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
void HMQtAbstractAsyncConnection::slot_onDisconnected()
{
    onDisconnect();
}
//-----------------------------------------------------------------------------
void HMQtAbstractAsyncConnection::slot_onErrorOccurred(QAbstractSocket::SocketError inError)
{
    qDebug() << inError;
    onError(convertingError(inError)); // Преобразуем ошибку QtSocket в стандартную и отправляем обработчику
}
//-----------------------------------------------------------------------------
