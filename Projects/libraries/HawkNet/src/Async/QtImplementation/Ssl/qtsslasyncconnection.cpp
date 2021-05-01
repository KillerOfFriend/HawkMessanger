#include "qtsslasyncconnection.h"

#include <set>

#include <neterrorcategory.h>

using namespace net;

//-----------------------------------------------------------------------------
HMQtSslAsyncConnection::HMQtSslAsyncConnection(const std::string& inHost, const uint16_t inPort, const ConCallbacks& inCallbacks) :
    HMQtAbstractAsyncConnection(inHost, inPort, inCallbacks, std::make_unique<QSslSocket>())
{
    errors::error_code ConnectError = connectionSigSlotConnect(); // Линкуем сигналы\сокеты
    assert(!ConnectError); // Сигналы должны слинковаться успешно
    ConnectError.clear();
}
//-----------------------------------------------------------------------------
HMQtSslAsyncConnection::HMQtSslAsyncConnection(std::unique_ptr<QTcpSocket>&& inSocket, const ConCallbacks& inCallbacks) :
    HMQtAbstractAsyncConnection(std::move(inSocket), inCallbacks)
{
    errors::error_code ConnectError = connectionSigSlotConnect(); // Линкуем сигналы\сокеты
    assert(!ConnectError); // Сигналы должны слинковаться успешно
    ConnectError.clear();
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
    QSslSocket* CurrentSocket = getSslSocket(); // Получаем указатель на текущий сокет

    disconnect(); // Принудительный разрыв соединения

    if (!CurrentSocket) // Сокет не инициализирован
        Error = make_error_code(errors::eNetError::neSocketNotInit);
    else // Если сокет успешно сформирован
    {
        if (!QSslSocket::supportsSsl()) // Проверяем поддержку SSL
            Error = make_error_code(errors::eNetError::neNoSslSupport);
        else
        {   // Пытаемся подключиться
            CurrentSocket->setPeerVerifyMode(QSslSocket::QueryPeer);

            CurrentSocket->connectToHostEncrypted(QString::fromStdString(getHost()), static_cast<quint16>(getPort()),
                                   QIODevice::ReadWrite, QAbstractSocket::AnyIPProtocol);

            //if (!sslSocket()->waitForEncrypted(inWaitTime.count())) // По хорошему требуется, но блокирует поток в тестах...
            if (!CurrentSocket->waitForConnected(inWaitTime.count()))
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
QSslSocket* HMQtSslAsyncConnection::getSslSocket()
{
    /*
     * БЕЗОПАСНО, т.к. вернёт перекастованый указатель,
     * владельцем которого является предок класса, являющийся
     * его единоличным владельцем (хранит в приватной секции).
     * А значит никуда не денется, пока не будет разрушен
     * текущий потомок.
    */
    return dynamic_cast<QSslSocket*>(getSocket().get());
}
//-----------------------------------------------------------------------------
errors::error_code HMQtSslAsyncConnection::connectionSigSlotConnect()
{
    errors::error_code Error = make_error_code(errors::eNetError::neSuccess); // Изначально метим как успех
    const QSslSocket* CurrentSocket = getSslSocket(); // Получаем указатель на текущий сокет

    if (!CurrentSocket)
        Error = make_error_code(errors::eNetError::neSocketNotInit);
    else
    {
        QObject::connect(CurrentSocket, &QSslSocket::readyRead, this, &HMQtSslAsyncConnection::slot_onReadyRead); // Линкуем событие "К чтению готов"
        QObject::connect(CurrentSocket, &QSslSocket::errorOccurred, this, &HMQtSslAsyncConnection::slot_onErrorOccurred); // Линкуем событие "Произошла ошибка"
        QObject::connect(CurrentSocket, &QSslSocket::disconnected, this, &HMQtSslAsyncConnection::slot_onDisconnected); // Линкуем событие "разрыв соеденения"

        QObject::connect(CurrentSocket, &QSslSocket::encrypted, this, &HMQtSslAsyncConnection::slot_onEncrypted); // Линкуем событие "Установлено защищённое соединение"
        QObject::connect(CurrentSocket, &QSslSocket::peerVerifyError, this, &HMQtSslAsyncConnection::slot_onPeerVerifyError); // Линкуем событие "Ошибка идентификации надежно однорангового узла"
        QObject::connect(CurrentSocket, &QSslSocket::sslErrors, this, &HMQtSslAsyncConnection::slot_onSslErrors); // Линкуем событие "Ошибка SSL"

        QObject::connect(CurrentSocket, &QSslSocket::encryptedBytesWritten, this, &HMQtSslAsyncConnection::slot_onEncryptedBytesWritten); // Линкуем событие "Зашифрованных байт записано"
        QObject::connect(CurrentSocket, &QSslSocket::handshakeInterruptedOnError, this, &HMQtSslAsyncConnection::slot_onHandshakeInterruptedOnError); // Линкуем событие "Прерывание рукопожатия по ошибке"
    }

    return Error;
}
//-----------------------------------------------------------------------------
