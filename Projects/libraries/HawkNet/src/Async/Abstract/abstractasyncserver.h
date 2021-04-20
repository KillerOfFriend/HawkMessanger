#ifndef HMABSTRACTASYNCSERVER_H
#define HMABSTRACTASYNCSERVER_H

#include "Abstract/abstractserver.h"
#include "abstractasyncconnection.h"

namespace net
{
//-----------------------------------------------------------------------------
/**
 * @brief ServerNewConnectionCallBackFn - Тип функции-обработки нового подключения
 * @param inError - Код ошибки
 */
typedef std::function<void(const std::size_t inConnectionID)> ServNewConnectionCallBackFn;
//-----------------------------------------------------------------------------
/**
 * @brief ErrorCallBackFn - Тип функции-обработчика произошедших ошибок сервера6
 * @param inError - Код ошибки
 */
typedef std::function<void(const errors::error_code inError)> ServErrorCallBackFn;
//-----------------------------------------------------------------------------


struct ServCallbacks
{
    ConCallbacks m_conCalbacks;

    ServNewConnectionCallBackFn     m_NewConnectionCallBack = nullptr;    ///< Обработчик новых подключений к серверу
    ServErrorCallBackFn             m_ErrorCallBack = nullptr;          ///< Обработчик произошедших ошибок сервера
};
//-----------------------------------------------------------------------------
/**
 * @brief The HMAbstractAsyncServer class - Абстракция, описывающая асинхронный сервер
 *
 * @authors Alekseev_s
 * @date 12.04.2021
 */
class HMAbstractAsyncServer : public HMAbstractServer
{
public:

    /**
     * @brief HMAbstractAsyncServer - Инициализирующий конструктор
     */
    HMAbstractAsyncServer(const ServCallbacks& inCallbacks);

    /**
     * @brief ~HMAbstractAsyncServer - Виртуальный деструктор по умолчанию
     */
    virtual ~HMAbstractAsyncServer() override = default;

protected:

    ServCallbacks m_Callbacks; ///< Набор сторонних обработчиков

    /**
     * @brief onNewConnection - Метод обработает подключение нового соединения
     * @param inConnection - Новое соединение
     * @return Вернёт идентификатор, присвоеный  соединению
     */
    virtual std::size_t onNewConnection(std::unique_ptr<HMAbstractConnection>&& inConnection) override;

    /**
     * @brief onDisconnect - Метод обработает отключение соединения
     * @param inConnectionID - ID соединения
     */
    virtual void onDisconnect(const std::size_t inConnectionID) override;

private:

    DisconnectCallBackFn m_DisconnectCallback; ///< Буфер для хранения стороннего калбека, обрабатывающего закрытие соединения

};
//-----------------------------------------------------------------------------
} // namespace net

#endif // HMABSTRACTASYNCSERVER_H
