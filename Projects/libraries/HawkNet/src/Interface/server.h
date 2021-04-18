#ifndef HMSERVER_H
#define HMSERVER_H

/**
 * @file server.h
 * @brief Содержит описание интерфейса сервера
 */

#include <map>

#include <errorcode.h>

#include "connection.h"

namespace net
{
//-----------------------------------------------------------------------------
/**
 * @brief The HMServer class - Интерфейс, описывающий сервер
 *
 * @authors Alekseev_s
 * @date 24.01.2021
 */
class HMServer
{
public:

    /**
     * @brief HMServer - Конструктор по умолчанию
     */
    HMServer() = default;

    /**
     * @brief ~HMServer - Виртуальный деструктор по умолчанию
     */
    virtual ~HMServer() = default;

    /**
     * @brief start - Метод запустит сервер
     * @return Вернёт признак ошибки
     */
    virtual errors::error_code start() = 0;

    /**
     * @brief isStarted - Метод вернёт состояние сервера
     * @return Вернёт состояние сервера
     */
    virtual bool isStarted() const = 0;

    /**
     * @brief stop - Метод остановит сервер
     */
    virtual void stop() = 0;

    /**
     * @brief connected - Метод проверит наличие соединения
     * @param inID - Идентификтор клиента
     * @return Вернёт признак существования соедиениня
     */
    virtual bool connected(const std::size_t inID) const = 0;

    /**
     * @brief connectionCount - Метод вернёт количество активных подключений
     * @return Вернёт количество активных подключений
     */
    virtual std::size_t connectionCount() const = 0;

    /**
     * @brief changeConnectionID - Метод перерегистрирует соединение под новым идентификатором
     * @param inCurrentID - Актуальный идентификатор соединения
     * @param inNewID - Новый идентификатор соединения
     * @return Вернёт признак ошибки
     */
    virtual errors::error_code changeConnectionID(const std::size_t inCurrentID, const std::size_t inNewID) = 0;

    /**
     * @brief send - Метод отправит данные клиенту с указанным идентификатором
     * @param inID - Идентификатор клиента-получателя
     * @param inData - Отправляемые данные
     * @return Вернёт признак ошибки
     */
    virtual errors::error_code send(const size_t inID, oByteStream&& inData) = 0;

    /**
     * @brief sendToAll - Метод отправит данные всем подключённым клиентам
     * @param inData - Отправляемые данные
     * @return Вернёт контейнер, хранящий список ошибок (ключём является идентификатор соединения)
     */
    virtual std::map<std::size_t, errors::error_code> sendToAll(oByteStream&& inData) = 0;

    /**
     * @brief closeConnection - Метод разорвёт соединение
     * @param inID - Идентификатор соединения
     */
    virtual void closeConnection(const size_t inID) = 0;

    /**
     * @brief closeConnection - Метод разорвёт все соединения
     */
    virtual void closeAllConnections() = 0;

};
//-----------------------------------------------------------------------------
}

#endif // HMSERVER_H
