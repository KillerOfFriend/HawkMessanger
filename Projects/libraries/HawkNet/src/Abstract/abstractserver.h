#ifndef HMABSTRACTSERVER_H
#define HMABSTRACTSERVER_H

#include <set>
#include <mutex>
#include <unordered_map>

#include "Interface/server.h"
#include "Abstract/abstractconnection.h"

namespace net
{
//-----------------------------------------------------------------------------
/**
 * @brief The HMAbstractServer class - Абстракция, описывающая базовый сервер
 *
 * @authors Alekseev_s
 * @date 12.04.2021
 */
class HMAbstractServer : public HMServer
{
public:

    /**
     * @brief HMAbstractServer - Конструктор по умолчанию
     */
    HMAbstractServer() = default;

    /**
     * @brief ~HMAbstractServer - Виртуальный деструктор по умолчанию
     */
    virtual ~HMAbstractServer() override = default;

    /**
     * @brief connected - Метод проверит наличие соединения
     * @param inID - Идентификтор клиента
     * @return Вернёт признак существования соедиениня
     */
    virtual bool connected(const std::size_t inID) const override;

    /**
     * @brief connectionCount - Метод вернёт количество активных подключений
     * @return Вернёт количество активных подключений
     */
    virtual std::size_t connectionCount() const override;

    /**
     * @brief changeConnectionID - Метод перерегистрирует соединение под новым идентификатором
     * @param inCurrentID - Актуальный идентификатор соединения
     * @param inNewID - Новый идентификатор соединения
     * @return Вернёт признак ошибки
     */
    virtual errors::error_code changeConnectionID(const std::size_t inCurrentID, const std::size_t inNewID) override;

    /**
     * @brief send - Метод отправит данные клиенту с указанным идентификатором
     * @param inID - Идентификатор клиента-получателя
     * @param inData - Отправляемые данные
     * @return Вернёт признак ошибки
     */
    virtual errors::error_code send(const size_t inID, oByteStream&& inData) override;

    /**
     * @brief sendToAll - Метод отправит данные всем подключённым клиентам
     * @param inData - Отправляемые данные
     * @return Вернёт контейнер, хранящий список ошибок (ключём является идентификатор соединения)
     */
    virtual std::map<std::size_t, errors::error_code> sendToAll(oByteStream&& inData) override;

    /**
     * @brief closeConnection - Метод разорвёт соединение
     * @param inID - Идентификатор соединения
     */
    virtual void closeConnection(const size_t inID) override;

    /**
     * @brief closeConnection - Метод разорвёт все соединения
     */
    virtual void closeAllConnections() override;

protected:

    /**
     * @brief onNewConnection - Метод обработает подключение нового соединения
     * @param inConnection - Новое соединение
     */
    virtual void onNewConnection(std::shared_ptr<HMAbstractConnection> inConnection);

    /**
     * @brief onDisconnect - Метод обработает отключение соединения
     * @param inConnectionID - ID соединения
     */
    virtual void onDisconnect(const std::size_t inConnectionID);

private:

    mutable std::recursive_mutex m_clientsDefender; ///< Мьютекс, защищающий контейнер авторизированных клиентов
    std::unordered_map<std::size_t, std::shared_ptr<HMAbstractConnection>> m_clients; ///< Контейнер, содержащий перечень авторизированных клиентов
};
//-----------------------------------------------------------------------------
} // namespace net

#endif // HMABSTRACTSERVER_H
