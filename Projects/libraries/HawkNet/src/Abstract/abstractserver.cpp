#include "abstractserver.h"

#include <neterrorcategory.h>

using namespace net;

//-----------------------------------------------------------------------------
bool HMAbstractServer::connected(const std::size_t inID) const
{
    std::lock_guard lg(m_clientsDefender);
    return m_clients.find(inID) != m_clients.end();
}
//-----------------------------------------------------------------------------
std::size_t HMAbstractServer::connectionCount() const
{
    std::lock_guard lg(m_clientsDefender);
    return m_clients.size();
}
//-----------------------------------------------------------------------------
errors::error_code HMAbstractServer::changeConnectionID(const std::size_t inCurrentID, const std::size_t inNewID)
{
    errors::error_code Error = make_error_code(errors::eNetError::neSuccess);

    if (inCurrentID != inNewID) // Нет смысла заменять если ID одинаковые
    {
        std::lock_guard lg(m_clientsDefender);
        auto Node = m_clients.extract(inCurrentID); // Извлекаем из контенера ноду { ключ, значение(соединение) }

        if (Node.empty()) // Если не удалось найти ноду по ключу
            Error = make_error_code(errors::eNetError::neClientNotFound);
        else // Нода успешно найдена
        {
            Node.key() = inNewID; // Заменяем ключ ноды
            auto InsertRes = m_clients.insert(std::move(Node)); // И пытаемся проталкнуть её обратно в контейнер

            if (!InsertRes.inserted) // Если при вставке произошло столкновение по ключам
            {
                Error = make_error_code(errors::eNetError::neClientIdAlredyExists); // Взводим признак ошибки
                InsertRes.node.key() = inCurrentID; // Возвращаем ноде прежний ключ
                m_clients.insert(std::move(InsertRes.node)); // Возвращаем её на прежнее место
            }
            else // Нода с новым ID успешно добавилась в контенер
                InsertRes.position->second->setID(inNewID); // Меняем идентификатор самого соединения
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
errors::error_code HMAbstractServer::send(const size_t inID, oByteStream&& inData)
{
    errors::error_code Error = make_error_code(errors::eNetError::neSuccess); // Изначально метим как успех
    std::lock_guard lg(m_clientsDefender);

    auto FindRes = m_clients.find(inID); // Ищим клиента

    if (FindRes == m_clients.end()) // Клиент не найден
        Error = make_error_code(errors::eNetError::neClientNotFound);
    else // Клиент найден
        Error = FindRes->second->send(std::move(inData)); // Отправляем данные

    return Error;
}
//-----------------------------------------------------------------------------
std::map<std::size_t, errors::error_code> HMAbstractServer::sendToAll(oByteStream&& inData)
{
    std::map<std::size_t, errors::error_code> Result;
    std::lock_guard lg(m_clientsDefender);

    for (auto& ConnectionPair : m_clients)
    {
        errors::error_code Error = ConnectionPair.second->send(oByteStream(inData.str())); // Отправляем копию данных клиенту

        if (Error) // Ошибку добавляем в перечень ошибок
            Result.insert(std::make_pair(ConnectionPair.second->getID(), Error));
    }

    return Result;
}
//-----------------------------------------------------------------------------
void HMAbstractServer::closeConnection(const size_t inID)
{
    /*
     * Есть 3 возможных сценария попадания в этот метод:
     * 1) Вызов из вне
     * 2) Вызов из closeAllConnections
     * 3) Вызов из onDisconnect
     *
     * При удалении соединения есть побочный эффект:
     *  - closeConnection удаляет соединение из контейнера
     *  - срабатывает деструктор соединения, вызывающий disconnect
     *  - происходит событие onDisconnect, который вызывает closeConnection
     * В итоге чего происходит дедлок при создании std::lock_guard.
     *
     * Решением данной проблемы стал рекурсивный мьютекс.
     * Он позволяет произойти повторному входу в closeConnection, во
     * время которого m_clients.find(inID) уже не сможет вернуть
     * валидный итератор и выполнение прекратится.
     *
     * НО! Рекурсивный мьютекс гарантирует повторный доступ к блокировке
     * для потока, который уже его заблокировал. В случай QtServer\QtSocket
     * имею подозрения в небезопасности такого решения в силу того, что
     * сигнал disconnected вызовет слот в главном потоке самого приложения,
     * что может привести к ситуации когда одновременно будет вызван closeConnection
     * в рамках одного потока, что может привести к краху на доступе к инвалидированным
     * итераторам контейнера соединений.
     * TODO: Проверить надёжность, возможно поменять решение.
     *
     * А может дело в том что уже первый час ночи и я реально верю что один поток
     * одновременно может быть в разных местах...
     */
    std::lock_guard lg(m_clientsDefender);
    auto FindRes = m_clients.find(inID); // Ищим соединение в контейнере

    if (FindRes != m_clients.end()) // Если оно найдено
        m_clients.erase(FindRes); // Удаляем соединение (disconnect будет вызван автоматически!)
}
//-----------------------------------------------------------------------------
void HMAbstractServer::closeAllConnections()
{
    if (m_clients.empty())
        return;

    std::vector<std::size_t> IDs(connectionCount()); // Список ключей

    {
        std::lock_guard lg(m_clientsDefender);
        std::transform(m_clients.begin(), m_clients.end(), IDs.begin(), [](auto inPair){ return inPair.first; }); // Получам список ключей
    }

    for (const std::size_t& ID : IDs) // Перебираем все соединения
        closeConnection(ID); // Закрываем каждое
}
//-----------------------------------------------------------------------------
void HMAbstractServer::onNewConnection(std::shared_ptr<HMAbstractConnection> inConnection)
{
    if (!inConnection)
        return;

    while(connected(inConnection->getID())) // Если идентификатор занят
        inConnection->setID(inConnection->getID() + 1); // Подменяем идентификатор

    std::lock_guard lg(m_clientsDefender);
    m_clients.insert(std::make_pair(inConnection->getID(), inConnection)); // Помещаем в контейнер нового анонимного клиента
}
//-----------------------------------------------------------------------------
void HMAbstractServer::onDisconnect(const size_t inConnectionID)
{
    closeConnection(inConnectionID);
}
//-----------------------------------------------------------------------------
