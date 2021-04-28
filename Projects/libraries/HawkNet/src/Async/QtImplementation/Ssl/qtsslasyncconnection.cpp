#include "qtsslasyncconnection.h"

#include <set>

#include <neterrorcategory.h>

using namespace net;

//-----------------------------------------------------------------------------
HMQtSslAsyncConnection::HMQtSslAsyncConnection(const std::string& inHost, const uint16_t inPort, const ConCallbacks& inCallbacks) :
    HMQtAbstractAsyncConnection(inHost, inPort, inCallbacks)
{

}
//-----------------------------------------------------------------------------
HMQtSslAsyncConnection::HMQtSslAsyncConnection(std::unique_ptr<QTcpSocket>&& inSocket, const ConCallbacks& inCallbacks) :
    HMQtAbstractAsyncConnection(std::move(inSocket), inCallbacks)
{

}
//-----------------------------------------------------------------------------
HMQtSslAsyncConnection::~HMQtSslAsyncConnection()
{
    disconnect();
}
//-----------------------------------------------------------------------------
errors::error_code HMQtSslAsyncConnection::connect(const std::chrono::milliseconds inWaitTime)
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
    {
        if (!QSslSocket::supportsSsl()) // Проверяем поддержку SSL
            Error = make_error_code(errors::eNetError::neNoSslSupport);
        else
        {   // Пытаемся подключиться
            sslSocket()->setPeerVerifyMode(QSslSocket::QueryPeer);

            sslSocket()->connectToHostEncrypted(QString::fromStdString(m_host), static_cast<quint16>(m_port),
                                   QIODevice::ReadWrite, QAbstractSocket::AnyIPProtocol);

            //if (!sslSocket()->waitForEncrypted(inWaitTime.count())) // По хорошему требуется, но блокирует поток в тестах...
            if (!sslSocket()->waitForConnected(inWaitTime.count()))
                Error = make_error_code(errors::eNetError::neTimeOut);
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
errors::error_code HMQtSslAsyncConnection::convertingError(const QSslError &inQSslError)
{
    errors::eNetError neErrorCode;

    static const std::set<QSslError::SslError> IgnoreErrors =
    {   // Общий перечень игнорируемых ошибок
        QSslError::SslError::SelfSignedCertificate,             // The certificate is self-signed, and untrusted
        QSslError::SslError::SelfSignedCertificateInChain,      //
        QSslError::SslError::HostNameMismatch                   // The host name did not match any of the valid hosts for this certificate
    };

    if (IgnoreErrors.find(inQSslError.error()) != IgnoreErrors.end()) // Если ошибка из числа игнорируемых
    {
        neErrorCode = errors::eNetError::neIgnored; // Помечаем как проигнорированную
        qWarning() << inQSslError; // Выдадим предупреждение
    }
    else // Если ошибка не игнорируемая
    {   // Преобразуем её к стандартной
        switch (inQSslError.error())
        {
            // ToDo
            default:                                                { neErrorCode = errors::eNetError::neUnknownQtSocketError; }
        }
    }

    return make_error_code(neErrorCode);
}
//-----------------------------------------------------------------------------
std::unique_ptr<QTcpSocket> HMQtSslAsyncConnection::makeSocket(errors::error_code& outError)
{
    outError = make_error_code(errors::eNetError::neSuccess); // Изначально метим как успех
    return std::make_unique<QSslSocket>();
}
//-----------------------------------------------------------------------------
errors::error_code HMQtSslAsyncConnection::connectionSigSlotConnect()
{
    errors::error_code Error = make_error_code(errors::eNetError::neSuccess); // Изначально метим как успех

    if (!m_socket)
        Error = make_error_code(errors::eNetError::neSocketNotInit);
    else
    {
        QObject::connect(sslSocket(), &QSslSocket::readyRead, this, &HMQtSslAsyncConnection::slot_onReadyRead); // Линкуем событие "К чтению готов"
        QObject::connect(sslSocket(), &QSslSocket::errorOccurred, this, &HMQtSslAsyncConnection::slot_onErrorOccurred); // Линкуем событие "Произошла ошибка"
        QObject::connect(sslSocket(), &QSslSocket::disconnected, this, &HMQtSslAsyncConnection::slot_onDisconnected); // Линкуем событие "разрыв соеденения"

        QObject::connect(sslSocket(), &QSslSocket::encrypted, this, &HMQtSslAsyncConnection::slot_onEncrypted); // Линкуем событие "Установлено защищённое соединение"
        QObject::connect(sslSocket(), &QSslSocket::peerVerifyError, this, &HMQtSslAsyncConnection::slot_onPeerVerifyError); // Линкуем событие "Ошибка идентификации надежно однорангового узла"
        QObject::connect(sslSocket(), &QSslSocket::sslErrors, this, &HMQtSslAsyncConnection::slot_onSslErrors); // Линкуем событие "Ошибка SSL"

        QObject::connect(sslSocket(), &QSslSocket::encryptedBytesWritten, this, &HMQtSslAsyncConnection::slot_onEncryptedBytesWritten); // Линкуем событие "Зашифрованных байт записано"
        QObject::connect(sslSocket(), &QSslSocket::handshakeInterruptedOnError, this, &HMQtSslAsyncConnection::slot_onHandshakeInterruptedOnError); // Линкуем событие "Прерывание рукопожатия по ошибке"
    }

    return Error;
}
//-----------------------------------------------------------------------------
void HMQtSslAsyncConnection::slot_onEncrypted()
{
    qInfo() << "An encrypted connection is established!";
}
//-----------------------------------------------------------------------------
void HMQtSslAsyncConnection::slot_onPeerVerifyError(const QSslError &inSslError)
{
    onError(convertingError(inSslError)); // Отправляем ошибку на верхний уровень абстракции
}
//-----------------------------------------------------------------------------
void HMQtSslAsyncConnection::slot_onSslErrors(const QList<QSslError> &inSslErrors)
{
    for (const auto& SslError : inSslErrors) // Перебираем полученные ошибки
        onError(convertingError(SslError)); // Отправляем её на верхний уровень абстракции
}
//-----------------------------------------------------------------------------
void HMQtSslAsyncConnection::slot_onEncryptedBytesWritten(qint64 inBytesWritten)
{
    slot_onBytesWritten(inBytesWritten); // Передаём событие на верхний уровень абстракции
}
//-----------------------------------------------------------------------------
void HMQtSslAsyncConnection::slot_onHandshakeInterruptedOnError(const QSslError &inSslError)
{
    onError(convertingError(inSslError)); // Отправляем ошибку на верхний уровень абстракции
    disconnect(); // Нет рукопожатия, нет защищённого соединения
}
//-----------------------------------------------------------------------------
QSslSocket* HMQtSslAsyncConnection::sslSocket() const
{
    return dynamic_cast<QSslSocket*>(m_socket.get());
}
//-----------------------------------------------------------------------------
