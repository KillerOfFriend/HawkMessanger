#ifndef HMQTABSTRACTASYNCCONNECTION_H
#define HMQTABSTRACTASYNCCONNECTION_H

#include <memory>

#include <QTcpSocket>

#include "Async/Abstract/abstractasyncconnection.h"

namespace net
{
//-----------------------------------------------------------------------------
typedef std::function<void(QTcpSocket*)> QTcpSocketDeleter;
//-----------------------------------------------------------------------------
typedef std::unique_ptr<QTcpSocket, QTcpSocketDeleter> QTcpSocketPtr;
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
     * @brief HMQtAbstractAsyncConnection - Инициализирующий конструктор
     * @param inCallbacks - Перечень калбеков
     */
    HMQtAbstractAsyncConnection(const std::string& inHost, const uint16_t inPort, const ConCallbacks& inCallbacks);

    /**
     * @brief HMQtAbstractAsyncConnection - Инициализирующий конструктор
     * @param inSoket - Сокет соединения
     * @param inCallbacks - Перечень калбеков
     */
    HMQtAbstractAsyncConnection(QTcpSocketPtr&& inSoket, const ConCallbacks& inCallbacks);

    /**
     * @brief ~HMQtSimpleAsyncConnection - Виртуальный деструктор
     */
    virtual ~HMQtAbstractAsyncConnection() override;

    /**
     * @brief connect - Метод произведёт подключение
     * @param inWaitTime - Время ожидания подключения
     * @return  Вернёт признак ошибки
     */
    virtual errors::error_code connect(const std::chrono::milliseconds inWaitTime = std::chrono::seconds(5)) override;

    /**
     * @brief isConnected - Метод вернёт состояние подключение
     * @return Вернёт состояние подключения
     */
    virtual bool isConnected() const override;

    /**
     * @brief disconnect - Метод разорвёт соединение
     */
    virtual void disconnect() override;

    /**
     * @brief convertingError - Метод преобразует ошибку QtSocket в стандартную
     * @param inQtSocketError - Признак ошибки QtSocket
     * @return Вернтё стандартную ошибка
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
     * @brief makeSocket - Метод, формирующий экземпляр Qt сокета
     * @param outError - Признак ошибки
     * @return Венёт указатель на экземпляр сервера или nullptr
     */
    virtual QTcpSocketPtr makeSocket(errors::error_code& outError) = 0;

    /**
     * @brief connectionSigSlotConnect - Метод выполнит линковку сигналов\слотов
     * @return Вернёт признак ошибки
     */
    virtual errors::error_code connectionSigSlotConnect();

private:

    std::string m_host = "";            ///< Адрес хоста
    uint16_t m_port = 0;                ///< Рабочий порт

    QTcpSocketPtr m_socket = nullptr;   ///< Простой сокет Qt

    QByteArray m_writeBuffer;           ///< Буфер, из которого происходит отправка
    QByteArray m_readBuffer;            ///< Буфер, в который происходит чтение

private slots:

    /**
     * @brief bytesWritten - Слот, обрабатывающий результат записи
     * @param slot_onBytesWritten - Количество записанных байт
     */
    void slot_onBytesWritten(qint64 inBytes);

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
