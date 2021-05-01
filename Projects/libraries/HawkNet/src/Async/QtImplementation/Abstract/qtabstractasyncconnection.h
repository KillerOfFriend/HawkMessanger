#ifndef HMQTABSTRACTASYNCCONNECTION_H
#define HMQTABSTRACTASYNCCONNECTION_H

#include <memory>

#include <QTcpSocket>

#include "Async/Abstract/abstractasyncconnection.h"

namespace net
{
//-----------------------------------------------------------------------------
/**
 * @brief The HMQtAbstractAsyncConnection class - Абстракция, реализующая базовый функционал асинхронного соединения Qt
 *
 * @authors Alekseev_s
 * @date 19.04.2021
 */
class HMQtAbstractAsyncConnection : public QObject, public HMAbstractAsyncConnection
{
    Q_OBJECT
public:

    /**
     * @brief HMQtAbstractAsyncConnection - Инициализирующий конструктор (Сторона клиента)
     * @param inHost - Адрес хоста
     * @param inPort - Рабочий порт хоста
     * @param inCallbacks - Перечень калбеков
     * @param inSocket - Сокет соединения со стороны клиента
     */
    HMQtAbstractAsyncConnection(const std::string& inHost, const uint16_t inPort, const ConCallbacks& inCallbacks, std::unique_ptr<QTcpSocket>&& inSocket);

    /**
     * @brief HMQtAbstractAsyncConnection - Инициализирующий конструктор (Сторона сервера)
     * @param inSocket - Сокет соединения со стороны сервера
     * @param inCallbacks - Перечень калбеков
     */
    HMQtAbstractAsyncConnection(std::unique_ptr<QTcpSocket>&& inSocket, const ConCallbacks& inCallbacks);

    /**
     * @brief ~HMQtSimpleAsyncConnection - Виртуальный деструктор по умолчанию
     */
    virtual ~HMQtAbstractAsyncConnection() override = default;

    /**
     * @brief connect - Метод произведёт подключение
     * @param inWaitTime - Время ожидания подключения
     * @return  Вернёт признак ошибки
     */
    virtual errors::error_code connect(const std::chrono::milliseconds inWaitTime = std::chrono::seconds(5)) override;

    /**
     * @brief disconnect - Метод разорвёт соединение
     */
    virtual void disconnect() override;

    /**
     * @brief status - Метод вернёт текущий статус соединения
     * @return Вернёт текущий статус соединения
     */
    virtual eConnectionStatus status() const override;

    /**
     * @brief getHost - Метод вернёт адрес хоста
     * @return Вернёт адрес хоста
     */
    std::string getHost() const;

    /**
     * @brief getPort - Метод вернёт рабочй порт
     * @return Вернёт рабочй порт
     */
    uint16_t getPort() const;

    /**
     * @brief convertingError - Метод преобразует ошибку QtSocket в стандартную
     * @param inQtSocketError - Признак ошибки QtSocket
     * @return Вернтёт стандартную ошибка
     */
    static errors::error_code convertingError(const QAbstractSocket::SocketError inQtSocketError);

protected:

    /**
     * @brief prepateNextData - Метод подготовит данные перед началом записи
     * @param inData - Данные, подготовленные к отправке
     */
    virtual void prepateNextData(oByteStream&& inData) override;

    /**
     * @brief write - Отправка данных
     */
    virtual void write() override;

    /**
     * @brief getSocket - Метод вернёт сокет соединения
     * @return Вернёт сокет соединения
     */
    std::unique_ptr<QTcpSocket>& getSocket();

private:


    std::string m_host = "";                        ///< Адрес хоста
    uint16_t m_port = 0;                            ///< Рабочий порт
    std::unique_ptr<QTcpSocket> m_socket = nullptr; ///< Простой сокет Qt

    QByteArray m_writeBuffer;           ///< Буфер, из которого происходит отправка
    QByteArray m_readBuffer;            ///< Буфер, в который происходит чтение

protected slots:

    /**
     * @brief slot_onBytesWritten - Слот, обрабатывающий результат записи
     * @param inBytesWritten - Количество записанных байт
     */
    void slot_onBytesWritten(qint64 inBytesWritten);

    /**
     * @brief slot_onReadyRead - Слот, выполняющий чтение
     */
    void slot_onReadyRead();

    /**
     * @brief slot_onDisconnected - Слот, обрабытывающий закрытие соединениея
     */
    void slot_onDisconnected();

    /**
     * @brief slot_errorOccurred - Слот, обрабатывающий ошибку
     * @param inQtSocketError - Признак ошибки QtSocket
     */
    void slot_onErrorOccurred(QAbstractSocket::SocketError inQtSocketError);
};
//-----------------------------------------------------------------------------
} // namespace net

#endif // HMQTABSTRACTASYNCCONNECTION_H
