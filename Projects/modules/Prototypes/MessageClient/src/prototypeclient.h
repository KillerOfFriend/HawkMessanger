#ifndef PROTOTYPECLIENT_H
#define PROTOTYPECLIENT_H

#include <memory>

#include <QObject>

#include <HawkNet.h>

//-----------------------------------------------------------------------------
class PrototypeClient : public QObject
{
    Q_OBJECT
public:

    /**
     * @brief PrototypeServer - Конструктор по умолчанию
     */
    PrototypeClient() = default;

    /**
     * @brief ~PrototypeServer - Деструктор
     */
    ~PrototypeClient();

    /**
     * @brief connect - Метод установит соединение с сервером
     * @param inHost - Адрес хоста
     * @param inPort - Рабочий порт
     * @return Вернёт признак ошибки
     */
    errors::error_code connect(const std::string& inHost, const std::uint16_t inPort);

    /**
     * @brief disconnect - Метод разорвёт соединение
     */
    void disconnect();

    /**
     * @brief send - Метод отправит данные на сервер
     * @param inData - Отправляемые данные
     * @return Вернёт признак ошибки
     */
    errors::error_code send(net::oByteStream&& inData);


private:

    std::unique_ptr<net::HMConnection> m_client = nullptr; ///< Клиент

    /**
     * @brief makeCallBacks - Метод сформирует калбэки
     * @return Вернёт сформированные калбэки
     */
    net::ConCallbacks makeCallBacks();

    /**
     * @brief makeClient - Метод сформирует клиент
     * @param inHost - Адрес хоста
     * @param inPort - Рабочий порт
     * @return Вернёт указатель на сформерованный клиент
     */
    std::unique_ptr<net::HMConnection> makeClient(const std::string& inHost, const std::uint16_t inPort);

    // Call Backs

    /**
     * @brief onReceiveData - Получены данные
     * @param inData - Полученные данные
     * @param inSenderID - Идентификатор соединения
     */
    void onReceiveData(net::iByteStream&& inData, const std::size_t inSenderID);

    /**
     * @brief onDisconnect - Соединение разорвано
     * @param inSenderID - Идентификатор соединения
     */
    void onDisconnect(const std::size_t inSenderID);

    /**
     * @brief onError - Ошибка на клиенте
     * @param inError - Метка ошибки
     * @param inSenderID - Идентификатор соединения
     */
    void onError(const errors::error_code inError, const std::size_t inSenderID);

public slots:

    void slot_doWork();

};
//-----------------------------------------------------------------------------

#endif // PROTOTYPECLIENT_H
