#ifndef HMQTSIMPLEASYNCSERVER_H
#define HMQTSIMPLEASYNCSERVER_H

#include <QTcpServer>

#include "Async/abstractasyncserver.h"

namespace net
{
//-----------------------------------------------------------------------------
class HMQtSimpleAsyncServer : public QObject, public HMAbstractAsyncServer
{
    Q_OBJECT
public:

    /**
     * @brief HMQtSimpleAsyncServer - Инициализирующий конструктор
     */
    HMQtSimpleAsyncServer(const std::uint16_t inPort, const ServCallbacks& inCallbacks);

    /**
     * @brief ~HMQtSimpleAsyncServer - Виртуальный деструктор по умолчанию
     */
    virtual ~HMQtSimpleAsyncServer() override;

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

private:

    QTcpServer m_server; ///< Простой сервер Qt
    std::uint16_t m_port = 0; ///< Рабочий порт

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

#endif // HMQTSIMPLEASYNCSERVER_H
