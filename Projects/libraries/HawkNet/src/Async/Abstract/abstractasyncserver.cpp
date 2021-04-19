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
void HMAbstractAsyncServer::onNewConnection(std::shared_ptr<HMAbstractConnection> inConnection)
{
    HMAbstractServer::onNewConnection(inConnection);
    // После добавления соединения вызываем обработчик если нужно
    if (m_Callbacks.m_NewConnectionCallBack)
        m_Callbacks.m_NewConnectionCallBack(inConnection->getID());
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
