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
     * @brief HMQtSslAsyncConnection - Инициализирующий конструктор (Сторона клиента)
     * @param inHost - Адрес хоста
     * @param inPort - Рабочий порт хоста
     * @param inCallbacks - Перечень калбеков
     */
    HMQtSslAsyncConnection(const std::string& inHost, const uint16_t inPort, const ConCallbacks& inCallbacks);

    /**
     * @brief HMQtSslAsyncConnection - Инициализирующий конструктор (Сторона сервера)
     * @param inSocket - Сокет соединения со стороны сервера
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

    /**
     * @brief convertingError - Метод преобразует ошибку QSslError в стандартную
     * @param inQSslError - Признак ошибки QSslError
     * @return Вернтёт стандартную ошибка
     */
    static errors::error_code convertingError(const QSslError& inQSslError);

private slots:

    /**
     * @brief slot_onEncrypted - Слот, обрабатывающий успешное зашифрованное соединение
     */
    void slot_onEncrypted();

    /**
     * @brief slot_onPeerVerifyError - Слот, обрабатывающий ошибку идентификации надежно однорангового узла.
     * @param inSslError - Признак ошибки QSslError
     */
    void slot_onPeerVerifyError(const QSslError &inSslError);

    /**
     * @brief slot_onSslErrors - Слот, обрабатывающий ошибки QSslError
     * @param inSslErrors - Признак ошибки QSslError
     */
    void slot_onSslErrors(const QList<QSslError> &inSslErrors);

    /**
     * @brief slot_onEncryptedBytesWritten - Слот, обрабатывающий результат записи
     * @param inBytesWritten - Количество записанных байт
     */
    void slot_onEncryptedBytesWritten(qint64 inBytesWritten);

    /**
     * @brief slot_onHandshakeInterruptedOnError - Слот, обрабатывающий прерывание рукопожатия из за ошибки
     * @param inSslError - Признак ошибки QSslError
     */
    void slot_onHandshakeInterruptedOnError(const QSslError &inSslError);

private:

    /**
     * @brief connectionSigSlotConnect - Метод выполнит линковку сигналов\слотов
     * @return Вернёт признак ошибки
     */
    errors::error_code connectionSigSlotConnect();

    /**
     * @brief getSslSocket - Метод вернёт сокет SSL соединения
     * @return Вернёт указатель на сокет SSL соединения
     */
    QSslSocket* getSslSocket();

};
//-----------------------------------------------------------------------------
}

#endif // HMQTSSLASYNCCONNECTION_H
