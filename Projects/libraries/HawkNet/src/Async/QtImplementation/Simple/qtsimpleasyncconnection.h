#ifndef HMQTSIMPLEASYNCCONNECTION_H
#define HMQTSIMPLEASYNCCONNECTION_H

#include <memory>

#include <QTcpSocket>

#include "Async/QtImplementation/Abstract/qtabstractasyncconnection.h"

namespace net
{
/**
 * @brief The HMQtSslAsyncServer class - Абстракция, описывающая простое асинхронное соединение Qt
 *
 * @authors Alekseev_s
 * @date 12.04.2021
 */
class HMQtSimpleAsyncConnection : public HMQtAbstractAsyncConnection
{
    Q_OBJECT
public:

    /**
     * @brief HMQtSimpleAsyncConnection - Инициализирующий конструктор
     * @param inCallbacks - Перечень калбеков
     */
    HMQtSimpleAsyncConnection(const std::string& inHost, const uint16_t inPort, const ConCallbacks& inCallbacks);

    /**
     * @brief HMQtSimpleAsyncConnection - Инициализирующий конструктор
     * @param inSoket - Сокет соединения
     * @param inCallbacks - Перечень калбеков
     */
    HMQtSimpleAsyncConnection(QTcpSocketPtr&& inSoket, const ConCallbacks& inCallbacks);

    /**
     * @brief ~HMQtSimpleAsyncConnection - Виртуальный деструктор
     */
    virtual ~HMQtSimpleAsyncConnection() override;

protected:

    /**
     * @brief makeSocket - Метод, формирующий экземпляр Qt сокета
     * @param outError - Признак ошибки
     * @return Венёт указатель на экземпляр сервера или nullptr
     */
    virtual QTcpSocketPtr makeSocket(errors::error_code& outError) override;

};
//-----------------------------------------------------------------------------
} // namespace net

#endif // HMQTSIMPLEASYNCCONNECTION_H
