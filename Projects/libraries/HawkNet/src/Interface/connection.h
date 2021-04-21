#ifndef HMCONNECTION_H
#define HMCONNECTION_H

/**
 * @file connection.h
 * @brief Содержит описание интерфейса соединения
 */

#include <chrono>
#include <sstream>

#include <errorcode.h>

namespace net
{
//-----------------------------------------------------------------------------
/**
 * @brief The eConnectionStatus enum - Перечисление статусов соединения
 */
enum class eConnectionStatus
{
    csUnknown = 0,      ///< Соединение не определено
    csDisconnected,     ///< Соединение отсутствует
    csDisconnecting,    ///< Соединение разрывается
    csConnecting,       ///< Соединени устанавливается
    csConnected,        ///< Соединение установлено

    csCount             ///< Счётчик
};
//-----------------------------------------------------------------------------
typedef std::basic_istringstream<char> iByteStream; ///< Буфер получаемых данных
typedef std::basic_ostringstream<char> oByteStream; ///< Буфер отправляемых данных
//-----------------------------------------------------------------------------
/**
 * @brief The HMConnection class - Интерфейс, описывающий соединение
 *
 * @authors Alekseev_s
 * @date 24.01.2021
 */
class HMConnection
{
public:

    /**
     * @brief HMConnection - Конструктор по умолчанию
     */
    HMConnection() = default;

    /**
     * @brief ~HMConnection - Виртуальный деструктор по умолчанию
     */
    virtual ~HMConnection() = default;

    /**
     * @brief connect - Метод произведёт подключение
     * @param inWaitTime - Время ожидания подключения
     * @return  Вернёт признак ошибки
     */
    virtual errors::error_code connect(const std::chrono::milliseconds inWaitTime = std::chrono::seconds(5)) = 0;

    /**
     * @brief isConnected - Метод вернёт состояние подключение
     * @return Вернёт состояние подключения
     */
    virtual bool isConnected() const = 0;

    /**
     * @brief disconnect - Метод разорвёт соединение
     */
    virtual void disconnect() = 0;

    /**
     * @brief status - Метод вернёт текущий статус соединения
     * @return Вернёт текущий статус соединения
     */
    virtual eConnectionStatus status() const = 0;

    /**
     * @brief send - Метод отправит переданные данные
     * @param inData - Отправлемые данные
     * @return Вернёт признак ошибки
     */
    virtual errors::error_code send(oByteStream&& inData) = 0;

};
//-----------------------------------------------------------------------------
}

#endif // HMCONNECTION_H
