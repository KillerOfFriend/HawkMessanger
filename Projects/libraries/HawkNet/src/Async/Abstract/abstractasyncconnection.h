#ifndef HMABSTRACTASYNCCONNECTION_H
#define HMABSTRACTASYNCCONNECTION_H

#include <queue>
#include <mutex>
#include <future>
#include <atomic>

#include "Abstract/abstractconnection.h"

namespace net
{
//-----------------------------------------------------------------------------
/**
 * @brief DataCallBackFn - Тип функции-обработчика полученных данных
 * @param inData - Полученные данные
 * @param inSenderID - Идентификатор соединения
 */
typedef std::function<void(iByteStream&& inData, const std::size_t inSenderID)> DataCallBackFn;
//-----------------------------------------------------------------------------
/**
 * @brief ErrorCallBackFn - Тип функции-обработчика произошедших ошибок
 * @param inError - Код ошибки
 * @param inSenderID - Идентификатор соединения
 */
typedef std::function<void(const errors::error_code inError, const std::size_t inSenderID)> ErrorCallBackFn;
//-----------------------------------------------------------------------------
/**
 * @brief DisconnectCallBackFn - Тип функции-обработчика закрытия соединения
 * @param inSenderID - Идентификатор соединения
 */
typedef std::function<void(const std::size_t inSenderID)> DisconnectCallBackFn;
//-----------------------------------------------------------------------------
/**
 * @brief The Callbacks struct - Структура, хранящая сторонние обработчики
 */
struct ConCallbacks
{
    DataCallBackFn m_DataCallBack = nullptr;                ///< Обработчик полученных данных
    ErrorCallBackFn m_ErrorCallBack = nullptr;              ///< Обработчик произошедших ошибок соединения
    DisconnectCallBackFn m_DisconnectCallBack = nullptr;    ///< Обработчик закрытия соединения
};
//-----------------------------------------------------------------------------
/**
 * @brief The HMAbstractAsyncConnection class - Абстракция, описывающая асинхронное соединение
 *
 * @authors Alekseev_s
 * @date 12.04.2021
 */
class HMAbstractAsyncConnection : public HMAbstractConnection
{
public:

    /**
     * @brief HMAbstractAsyncConnection - Инициализирующий конструктор
     * @param inRecievCallback - Функция, принимающая полученные данные
     */
    HMAbstractAsyncConnection(const ConCallbacks& inCallbacks);

    /**
     * @brief ~HMAbstractAsyncConnection - Виртуальный деструктор по умолчанию
     */
    virtual ~HMAbstractAsyncConnection() override = default;

    /**
     * @brief isConnected - Метод вернёт состояние подключение
     * @return Вернёт состояние подключения
     */
    virtual bool isConnected() const override;

    /**
     * @brief disconnect - Метод разорвёт соединение
     */
    virtual void disconnect() override;

    /**
     * @brief send - Метод отправит переданные данные
     * @param inData - Отправлемые данные
     * @return Вернёт признак ошибки
     */
    virtual errors::error_code send(oByteStream &&inData) override;

protected:

    const ConCallbacks m_Callbacks; ///< Набор сторонних обработчиков

    /**
     * @brief beforeWrite - Метод проверит, можно ли выполнить запись
     * @return Вернёт признак готовности к новой записи
     */
    virtual bool beforeWrite();

    /**
     * @brief afterWrite - Метод выполняется по завершению записи
     */
    virtual void afterWrite();

    /**
     * @brief prepateNextData - Метод подготовит данные перед началом записи
     * @param inData - Данные, подготовленные к отправке
     */
    virtual void prepateNextData(oByteStream&& inData) = 0;

    /**
     * @brief write - Отправка данных
     */
    virtual void write() = 0;

    // ===================
    // Обработчики эвентов
    // ===================

    /**
     * @brief onReadEnd - Метод, принимающий прочитанные данные
     * @param inData - Прочитанные данные
     */
    void onReadEnd(iByteStream&& inData) const;

    /**
     * @brief onError - Метод, принимающий принак ошибки
     * @param inError - Признак ошибки
     */
    void onError(const errors::error_code inError) const;

    /**
     * @brief onDisconnect - Метод, обрабатывающий разрыв соединения
     */
    void onDisconnect() const;

private:

    std::mutex m_dataDefender; ///< Мьютекс, защищающий очередь данных
    std::queue<oByteStream> m_dataQueue; ///< Очередь данных на отправку

    std::atomic_bool m_isWrite; ///< Флаг "идёт запись"

    /**
     * @brief getNextData - Метод вернёт следующий набор данных для отправки или признак ошибки
     * @param outData - Принимающий набор данных
     * @return Вернёт признак получения данных из очереди
     */
    bool getNextData(oByteStream& outData);

    /**
     * @brief writeNext - Метод начнёт запись следующих данных из очереди
     * @return Вернёт признак успеха операции
     */
    bool writeNext();
};
//-----------------------------------------------------------------------------
} // namespace net

#endif // HMABSTRACTASYNCCONNECTION_H
