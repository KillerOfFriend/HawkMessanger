#ifndef HMQTSIMPLEASYNCCONNECTION_H
#define HMQTSIMPLEASYNCCONNECTION_H

#include <memory>

#include <QTcpSocket>

#include "Async/QtImplementation/Abstract/qtabstractasyncconnection.h"

namespace net
{
//-----------------------------------------------------------------------------
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
     * @brief HMQtSimpleAsyncConnection - Инициализирующий конструктор (Сторона клиента)
     * @param inHost - Адрес хоста
     * @param inPort - Рабочий порт хоста
     * @param inCallbacks - Перечень калбеков
     */
    HMQtSimpleAsyncConnection(const std::string& inHost, const uint16_t inPort, const ConCallbacks& inCallbacks);

    /**
     * @brief HMQtSimpleAsyncConnection - Инициализирующий конструктор (Сторона сервера)
     * @param inSocket - Сокет соединения со стороны сервера
     * @param inCallbacks - Перечень калбеков
     */
    HMQtSimpleAsyncConnection(std::unique_ptr<QTcpSocket>&& inSocket, const ConCallbacks& inCallbacks);

    /**
     * @brief ~HMQtSimpleAsyncConnection - Виртуальный деструктор
     */
    virtual ~HMQtSimpleAsyncConnection() override;

private:

    /**
     * @brief connectionSigSlotConnect - Метод выполнит линковку сигналов\слотов
     * @return Вернёт признак ошибки
     */
    errors::error_code connectionSigSlotConnect();

};
//-----------------------------------------------------------------------------
} // namespace net

#endif // HMQTSIMPLEASYNCCONNECTION_H
