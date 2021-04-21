#ifndef HMQTSIMPLEASYNCSERVER_H
#define HMQTSIMPLEASYNCSERVER_H

#include <QTcpServer>

#include "Async/QtImplementation/Abstract/qtabstractasyncserver.h"

namespace net
{
//-----------------------------------------------------------------------------
/**
 * @brief The HMQtSimpleAsyncServer class - Класс, описывающий простой асинхронный TCP сервер
 *
 * @authors Alekseev_s
 * @date 12.04.2021
 */
class HMQtSimpleAsyncServer : public HMQtAbstractAsyncServer
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

protected:

    /**
     * @brief makeServer - Метод, формирующий экземпляр Qt сервера
     * @param outError - Признак ошибки
     * @return Венёт указатель на экземпляр сервера или nullptr
     */
    virtual std::unique_ptr<QTcpServer> makeServer(errors::error_code& outError) override;

    /**
     * @brief makeConnection - Метод сформирует соединение
     * @param inSocket - Сокет соединения
     * @return Вернёт указатель на новое соединение или nullptr
     */
    virtual std::unique_ptr<HMQtAbstractAsyncConnection> makeConnection(std::unique_ptr<QTcpSocket>&& inSocket) override;

};
//-----------------------------------------------------------------------------
} // namespace net

#endif // HMQTSIMPLEASYNCSERVER_H
