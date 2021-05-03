#ifndef PROTOTYPESERVER_H
#define PROTOTYPESERVER_H

#include <mutex>
#include <memory>

#include <HawkNet.h>

//-----------------------------------------------------------------------------
class PrototypeServer
{
public:

    /**
     * @brief PrototypeServer - Конструктор по умолчанию
     */
    PrototypeServer() = default;

    /**
     * @brief ~PrototypeServer - Деструктор
     */
    ~PrototypeServer();

    /**
     * @brief start - Метод запустит сервер
     * @param inPort - Рабочий порт
     * @return Вернёт признак ошибки
     */
    errors::error_code start(const std::int16_t inPort);

    /**
     * @brief stop - Метод остановит сервер
     */
    void stop();

private:

    std::unique_ptr<net::HMServer> m_server = nullptr; ///< Сервер

    /**
     * @brief makeCallBacks - Метод сформирует калбэки
     * @return Вернёт сформированные калбэки
     */
    net::ServCallbacks makeCallBacks();

    /**
     * @brief make_server - Метод сформирует сервер
     * @param inPort - Рабочий порт
     * @return Вернёт указатель на интерфейс сервера
     */
    std::unique_ptr<net::HMServer> make_server(const std::int16_t inPort);

    // Call Backs

    /**
     * @brief onNewConnection - Произошло новое подключение
     * @param inConnectionID - Идентификатор соединения
     */
    void onNewConnection(const std::size_t inConnectionID);

    /**
     * @brief onReceiveData - Получены данные
     * @param inData - Полученные данные
     * @param inSenderID - Идентификатор соединения
     */
    void onReceiveData(net::iByteStream&& inData, const std::size_t inSenderID);

    /**
     * @brief onClientDisconnect - Соединение разорвано
     * @param inSenderID - Идентификатор соединения
     */
    void onClientDisconnect(const std::size_t inSenderID);

    /**
     * @brief onClientError - Ошибка на клиенте
     * @param inError - Метка ошибки
     * @param inSenderID - Идентификатор соединения
     */
    void onClientError(const errors::error_code inError, const std::size_t inSenderID);

    /**
     * @brief onServerError - Ошибка на сервере
     * @param inError - Метка ошибки
     */
    void onServerError(const errors::error_code inError);

private:

    std::mutex m_dataReceiveDefender; ///< Сокет, защищающий получение данных

};
//-----------------------------------------------------------------------------

#endif // PROTOTYPESERVER_H
