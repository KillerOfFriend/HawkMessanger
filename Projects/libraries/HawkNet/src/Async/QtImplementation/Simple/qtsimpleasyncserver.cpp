#include "qtsimpleasyncserver.h"

#include <neterrorcategory.h>

#include "qtsimpleasyncconnection.h"

using namespace net;

//-----------------------------------------------------------------------------
HMQtSimpleAsyncServer::HMQtSimpleAsyncServer(const std::uint16_t inPort, const ServCallbacks& inCallbacks) :
    HMQtAbstractAsyncServer(inPort, inCallbacks)
{

}
//-----------------------------------------------------------------------------
HMQtSimpleAsyncServer::~HMQtSimpleAsyncServer()
{
    stop(); // При завершении обязательно останавливаем сервер
}
//-----------------------------------------------------------------------------
std::unique_ptr<QTcpServer> HMQtSimpleAsyncServer::makeServer(errors::error_code& outError)
{
    outError = make_error_code(errors::eNetError::neSuccess); // Изначально метим как успех
    return std::make_unique<QTcpServer>(nullptr); // Формируем простой сервер QTcp
}
//-----------------------------------------------------------------------------
std::shared_ptr<HMQtAbstractAsyncConnection> HMQtSimpleAsyncServer::makeConnection(QTcpSocketPtr&& inSocket)
{
    return std::make_shared<HMQtSimpleAsyncConnection>(std::move(inSocket), m_Callbacks.m_conCalbacks);
}
//-----------------------------------------------------------------------------
