#include "abstractasyncconnection.h"

#include <neterrorcategory.h>

using namespace net;

//-----------------------------------------------------------------------------
HMAbstractAsyncConnection::HMAbstractAsyncConnection(const ConCallbacks &inCallbacks) :
    HMAbstractConnection(),
    m_Callbacks(inCallbacks)
{
    std::atomic_init(&m_isWrite, false);
}
//-----------------------------------------------------------------------------
void HMAbstractAsyncConnection::disconnect()
{
    std::lock_guard lg(m_dataDefender);
    // Очищаем очередь сообщений
    while (!m_dataQueue.empty())
        m_dataQueue.pop();
}
//-----------------------------------------------------------------------------
errors::error_code HMAbstractAsyncConnection::send(oByteStream &&inData)
{
    errors::error_code Error = make_error_code(errors::eNetError::neSuccess); // Изначально метим как успех

    if (!isConnected()) // Если нет соединения
        Error = make_error_code(errors::eNetError::neNotConnected);
    else // Соединение установлено
    {
        inData.seekp(0, inData.end); // Принудительно переместим "курсор записи" в самый конец
        if (inData.str().back() != C_DATA_SEPARATOR) // Если данные не завершаются разделителем
            inData << C_DATA_SEPARATOR; // Добавляем разделитель в самый конец

        {
           std::lock_guard lg(m_dataDefender);
           m_dataQueue.push(std::move(inData)); // Перемещаем данные в очередь
        }

        if (beforeWrite()) // Если разрешается начать запись
            afterWrite(); // Вызываем функцию, выполняющую запись следующей порции данных
    }

    return Error;
}
//-----------------------------------------------------------------------------
bool HMAbstractAsyncConnection::beforeWrite()
{
    if (!m_isWrite) // Запись не идёт
    {
        m_isWrite = true; // Взводим флаг что запись идёт
        return true; // Разрешаем запись
    }
    else
        return false; // Запрещаем начинать новую запись запись
}
//-----------------------------------------------------------------------------
void HMAbstractAsyncConnection::afterWrite()
{
    m_isWrite = writeNext(); // Пытаемся продолжить запись, если в очереди есть данные
}
//-----------------------------------------------------------------------------
bool HMAbstractAsyncConnection::getNextData(oByteStream& outData)
{
    bool Result = true;
    std::lock_guard lg(m_dataDefender);

    if (m_dataQueue.empty()) // Если очередь пуста
        Result = false; // Взводим флаг ошибки
    else // Если данные успешно получены
    {
        outData = std::move(m_dataQueue.front()); // Перемещаем данные в возвращаемые
        m_dataQueue.pop(); // Выкидываем опустевший набор данных из очереди
    }

    return Result;
}
//-----------------------------------------------------------------------------
bool HMAbstractAsyncConnection::writeNext()
{
    bool Result = true;
    oByteStream Data; // Запрашиваем данные из очереди для отправки

    if (!getNextData(Data)) // Если не удалось получить данные из очереди
        Result = false;
    else // Данные успешно получены
    {
        prepateNextData(std::move(Data)); // Отправляем данные на подготовку перед записью
        write(); // Вызываем запись
    }

    return Result;
}
//-----------------------------------------------------------------------------

// ===================
// Обработчики эвентов
// ===================

//-----------------------------------------------------------------------------
void HMAbstractAsyncConnection::onReadEnd(iByteStream&& inData) const
{
    if (m_Callbacks.m_DataCallBack)
        m_Callbacks.m_DataCallBack(std::move(inData), getID());
}
//-----------------------------------------------------------------------------
void HMAbstractAsyncConnection::onError(const errors::error_code inError) const
{
    if (m_Callbacks.m_ErrorCallBack)
        m_Callbacks.m_ErrorCallBack(inError, getID());
}
//-----------------------------------------------------------------------------
void HMAbstractAsyncConnection::onDisconnect() const
{
    if (m_Callbacks.m_DisconnectCallBack)
        m_Callbacks.m_DisconnectCallBack(getID());
}
//-----------------------------------------------------------------------------
