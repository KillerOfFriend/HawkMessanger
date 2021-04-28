#ifndef HMQTSSLASYNCCONNECTION_H
#define HMQTSSLASYNCCONNECTION_H

#include <QSslSocket>
#include <QSslPreSharedKeyAuthenticator>

#include "Async/QtImplementation/Abstract/qtabstractasyncconnection.h"

namespace net
{
//-----------------------------------------------------------------------------
/**
 * @brief The HMQtSslAsyncConnection class - Абстракция, описывающая асинхронное TCP соединение, использующее SSL
 *
 * @authors Alekseev_s
 * @date 19.04.2021
 */
class HMQtSslAsyncConnection : public HMQtAbstractAsyncConnection
{
    Q_OBJECT
public:

    /**
     * @brief HMQtSslAsyncConnection - Инициализирующий конструктор
     * @param inHost - Адрес хоста
     * @param inPort - Рабочий порт хоста
     * @param inCallbacks - Перечень калбеков
     */
    HMQtSslAsyncConnection(const std::string& inHost, const uint16_t inPort, const ConCallbacks& inCallbacks);

    /**
     * @brief HMQtSslAsyncConnection - Инициализирующий конструктор
     * @param inSocket - Сокет соединения
     * @param inCallbacks - Перечень калбеков
     */
    HMQtSslAsyncConnection(std::unique_ptr<QTcpSocket>&& inSocket, const ConCallbacks& inCallbacks);

    /**
     * @brief ~HMQtSslAsyncConnection - Виртуальный деструктор
     */
    virtual ~HMQtSslAsyncConnection() override;

    /**
     * @brief connect - Метод произведёт подключение
     * @param inWaitTime - Время ожидания подключения
     * @return  Вернёт признак ошибки
     */
    virtual errors::error_code connect(const std::chrono::milliseconds inWaitTime = std::chrono::seconds(5)) override;

protected:

    /**
     * @brief makeSocket - Метод, формирующий экземпляр Qt сокета
     * @param outError - Признак ошибки
     * @return Венёт указатель на экземпляр сервера или nullptr
     */
    virtual std::unique_ptr<QTcpSocket> makeSocket(errors::error_code& outError) override;

    /**
     * @brief connectionSigSlotConnect - Метод выполнит линковку сигналов\слотов
     * @return Вернёт признак ошибки
     */
    virtual errors::error_code connectionSigSlotConnect() override;

private slots:

    void slot_onEncrypted();
    void slot_onPeerVerifyError(const QSslError &error);
    void slot_onSslErrors(const QList<QSslError> &errors);
    void slot_onModeChanged(QSslSocket::SslMode newMode);
    void slot_onEncryptedBytesWritten(qint64 totalBytes);
    void slot_onPreSharedKeyAuthenticationRequired(QSslPreSharedKeyAuthenticator *authenticator);
    void slot_onNewSessionTicketReceived();
    void slot_onAlertSent(QSsl::AlertLevel level, QSsl::AlertType type, const QString &description);
    void slot_onAlertReceived(QSsl::AlertLevel level, QSsl::AlertType type, const QString &description);
    void slot_onHandshakeInterruptedOnError(const QSslError &error);

private:

    QSslSocket* sslSocket() const;

};
//-----------------------------------------------------------------------------
}

#endif // HMQTSSLASYNCCONNECTION_H
