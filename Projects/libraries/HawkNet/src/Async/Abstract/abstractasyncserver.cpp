#include "abstractasyncserver.h"

#include <neterrorcategory.h>

using namespace net;

//-----------------------------------------------------------------------------
HMAbstractAsyncServer::HMAbstractAsyncServer(const ServCallbacks& inCallbacks) :
    HMAbstractServer(),
    m_Callbacks(inCallbacks)
{
    m_DisconnectCallback = m_Callbacks.m_conCalbacks.m_DisconnectCallBack; // Передаём сторонний обработчик разырва соединения в отдельный буфер
    m_Callbacks.m_conCalbacks.m_DisconnectCallBack = std::bind(&HMAbstractAsyncServer::onDisconnect, this, std::placeholders::_1); // Помещаем на его место обработчик этого класса
}
//-----------------------------------------------------------------------------
std::size_t HMAbstractAsyncServer::onNewConnection(std::unique_ptr<HMAbstractConnection>&& inConnection)
{
    std::size_t NewConnectionID = HMAbstractServer::onNewConnection(std::move(inConnection));
    // После добавления соединения вызываем обработчик если нужно
    if (m_Callbacks.m_NewConnectionCallBack && NewConnectionID)
        m_Callbacks.m_NewConnectionCallBack(NewConnectionID);

    return NewConnectionID;
}
//-----------------------------------------------------------------------------
void HMAbstractAsyncServer::onDisconnect(const std::size_t inConnectionID)
{
    // Сначала вызываем обработчик если нужно
    if (m_DisconnectCallback)
        m_DisconnectCallback(inConnectionID);
    // В уже потом вызываем отключение
    HMAbstractServer::onDisconnect(inConnectionID); // Вызываем обработчик предка (соединение будет удалено)
}
//-----------------------------------------------------------------------------
