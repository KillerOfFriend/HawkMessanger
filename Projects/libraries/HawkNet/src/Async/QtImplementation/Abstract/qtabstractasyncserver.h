#ifndef HMQTABSTRACTASYNCSERVER_H
#define HMQTABSTRACTASYNCSERVER_H

#include <memory>

#include <QObject>
#include <QTcpServer>

#include "Async/Abstract/abstractasyncserver.h"
#include "qtabstractasyncconnection.h"

namespace net
{
//-----------------------------------------------------------------------------
/**
 * @brief The HMQtAbstractAsyncServer class - Абстракция, реализующая базовый функционал асинхронного сервера Qt
 *
 * @authors Alekseev_s
 * @date 19.04.2021
 */
class HMQtAbstractAsyncServer : public QObject, public HMAbstractAsyncServer
{
public:

    /**
     * @brief HMQtAbstractAsyncServer - Инициализирующий конструктор
     */
    HMQtAbstractAsyncServer(const std::uint16_t inPort, const ServCallbacks& inCallbacks);

    /**
     * @brief ~HMQtSimpleAsyncServer - Виртуальный деструктор
     */
    virtual ~HMQtAbstractAsyncServer() override;

    /**
     * @brief start - Метод запустит сервер
     * @return Вернёт признак ошибки
     */
    virtual errors::error_code start() override;

    /**
     * @brief isStarted - Метод вернёт состояние сервера
     * @return Вернёт состояние сервера
     */
    virtual bool isStarted() const override;

    /**
     * @brief stop - Метод остановит сервер
     */
    virtual void stop() override;

protected:

    /**
     * @brief makeServer - Метод, формирующий экземпляр Qt сервера
     * @param outError - Признак ошибки
     * @return Венёт указатель на экземпляр сервера или nullptr
     */
    virtual std::unique_ptr<QTcpServer> makeServer(errors::error_code& outError) = 0;

    /**
     * @brief makeConnection - Метод сформирует соединение
     * @param inSocket - Сокет соединения
     * @return Вернёт указатель на новое соединение или nullptr
     */
    virtual std::unique_ptr<HMQtAbstractAsyncConnection> makeConnection(QTcpSocketPtr&& inSocket) = 0;

    /**
     * @brief serverSigSlotConnect - Метод выполнит линковку сигналов\слотов
     * @return Вернёт признак ошибки
     */
    virtual errors::error_code serverSigSlotConnect();

private:

    std::unique_ptr<QTcpServer> m_server = nullptr; ///< Простой сервер Qt
    std::uint16_t m_port = 0;                       ///< Рабочий порт

private slots:

    /**
     * @brief slot_newConnection - Слот обрабатывающий новое соединение
     */
    void slot_newConnection();

    /**
     * @brief slot_acceptError
     * @param socketError
     */
    void slot_acceptError(QAbstractSocket::SocketError socketError);
};
//-----------------------------------------------------------------------------
} // namespace net

#endif // HMQTABSTRACTASYNCSERVER_H
