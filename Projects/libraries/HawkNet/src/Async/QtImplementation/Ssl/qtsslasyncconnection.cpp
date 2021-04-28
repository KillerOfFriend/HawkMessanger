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
#include "sslfuncs.h"
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
    {   // Пытаемся подключиться

        sslSocket()->setPeerVerifyMode(QSslSocket::QueryPeer);

        sslSocket()->connectToHostEncrypted(QString::fromStdString(m_host), static_cast<quint16>(m_port),
                               QIODevice::ReadWrite, QAbstractSocket::AnyIPProtocol);

        //if (!S->waitForEncrypted(inWaitTime.count()))
        if (!sslSocket()->waitForConnected(inWaitTime.count()))
            Error = make_error_code(errors::eNetError::neTimeOut);
    }

    return Error;
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
    QObject::connect(sslSocket(), &QSslSocket::readyRead, this, &HMQtSslAsyncConnection::slot_onReadyRead); // Линкуем событие "К чтению готов"
    //QObject::connect(sslSocket(), &QSslSocket::bytesWritten, this, &HMQtSslAsyncConnection::slot_onBytesWritten); // Линкуем событие "Байт записано"
    QObject::connect(sslSocket(), &QSslSocket::errorOccurred, this, &HMQtSslAsyncConnection::slot_onErrorOccurred); // Линкуем событие "Произошла ошибка"
    QObject::connect(sslSocket(), &QSslSocket::disconnected, this, &HMQtSslAsyncConnection::slot_onDisconnected); // Линкуем событие "разрыв соеденения"

    QObject::connect(sslSocket(), &QSslSocket::encrypted, this, &HMQtSslAsyncConnection::slot_onEncrypted);
    QObject::connect(sslSocket(), &QSslSocket::peerVerifyError, this, &HMQtSslAsyncConnection::slot_onPeerVerifyError);
    QObject::connect(sslSocket(), &QSslSocket::sslErrors, this, &HMQtSslAsyncConnection::slot_onSslErrors);
    QObject::connect(sslSocket(), &QSslSocket::modeChanged, this, &HMQtSslAsyncConnection::slot_onModeChanged);
    QObject::connect(sslSocket(), &QSslSocket::encryptedBytesWritten, this, &HMQtSslAsyncConnection::slot_onEncryptedBytesWritten);
    QObject::connect(sslSocket(), &QSslSocket::preSharedKeyAuthenticationRequired, this, &HMQtSslAsyncConnection::slot_onPreSharedKeyAuthenticationRequired);
    QObject::connect(sslSocket(), &QSslSocket::newSessionTicketReceived, this, &HMQtSslAsyncConnection::slot_onNewSessionTicketReceived);
    QObject::connect(sslSocket(), &QSslSocket::alertSent, this, &HMQtSslAsyncConnection::slot_onAlertSent);
    QObject::connect(sslSocket(), &QSslSocket::alertReceived, this, &HMQtSslAsyncConnection::slot_onAlertReceived);
    QObject::connect(sslSocket(), &QSslSocket::handshakeInterruptedOnError, this, &HMQtSslAsyncConnection::slot_onHandshakeInterruptedOnError);
}
//-----------------------------------------------------------------------------
void HMQtSslAsyncConnection::slot_onEncrypted()
{
    qDebug() << "HMQtSslAsyncConnection::slot_onEncrypted";
}
//-----------------------------------------------------------------------------
void HMQtSslAsyncConnection::slot_onPeerVerifyError(const QSslError &error)
{
    qDebug() << "HMQtSslAsyncConnection::slot_onPeerVerifyError" << error;
}
//-----------------------------------------------------------------------------
void HMQtSslAsyncConnection::slot_onSslErrors(const QList<QSslError> &errors)
{
    static const std::set<QSslError::SslError> IgnoreErrors =
    {
        QSslError::SslError::SelfSignedCertificate,
        QSslError::SslError::SelfSignedCertificateInChain
    };
    for (auto Err : errors)
    {
        if (IgnoreErrors.find(Err.error()) != IgnoreErrors.end())
            qDebug() << "HMQtSslAsyncConnection::slot_onSslErrors" << Err;
    }
}
//-----------------------------------------------------------------------------
void HMQtSslAsyncConnection::slot_onModeChanged(QSslSocket::SslMode newMode)
{
    qDebug() << "HMQtSslAsyncConnection::slot_onModeChanged" << newMode;
}
//-----------------------------------------------------------------------------
void HMQtSslAsyncConnection::slot_onEncryptedBytesWritten(qint64 totalBytes)
{
    //qDebug() << "HMQtSslAsyncConnection::slot_onEncryptedBytesWritten" << totalBytes;
    slot_onBytesWritten(totalBytes);
}
//-----------------------------------------------------------------------------
void HMQtSslAsyncConnection::slot_onPreSharedKeyAuthenticationRequired(QSslPreSharedKeyAuthenticator *authenticator)
{
    qDebug() << "HMQtSslAsyncConnection::slot_onPreSharedKeyAuthenticationRequired";
}
//-----------------------------------------------------------------------------
void HMQtSslAsyncConnection::slot_onNewSessionTicketReceived()
{
    qDebug() << "HMQtSslAsyncConnection::slot_onNewSessionTicketReceived";
}
//-----------------------------------------------------------------------------
void HMQtSslAsyncConnection::slot_onAlertSent(QSsl::AlertLevel level, QSsl::AlertType type, const QString &description)
{
    qDebug() << "HMQtSslAsyncConnection::slot_onAlertSent" << static_cast<quint32>(level) << static_cast<quint32>(type) << description;
}
//-----------------------------------------------------------------------------
void HMQtSslAsyncConnection::slot_onAlertReceived(QSsl::AlertLevel level, QSsl::AlertType type, const QString &description)
{
    qDebug() << "HMQtSslAsyncConnection::slot_onAlertReceived" << static_cast<quint32>(level) << static_cast<quint32>(type) << description;
}
//-----------------------------------------------------------------------------
void HMQtSslAsyncConnection::slot_onHandshakeInterruptedOnError(const QSslError &error)
{
    qDebug() << "HMQtSslAsyncConnection::slot_onHandshakeInterruptedOnError" << error;
}
//-----------------------------------------------------------------------------
QSslSocket* HMQtSslAsyncConnection::sslSocket() const
{
    return dynamic_cast<QSslSocket*>(m_socket.get());
}
//-----------------------------------------------------------------------------
