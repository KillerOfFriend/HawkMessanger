#include "qtsimpleasyncconnection.h"

#include <QHostAddress>

#include <neterrorcategory.h>

using namespace net;

//-----------------------------------------------------------------------------
HMQtSimpleAsyncConnection::HMQtSimpleAsyncConnection(const std::string& inHost, const uint16_t inPort, const ConCallbacks &inCallbacks) :
    HMQtAbstractAsyncConnection(inHost, inPort, inCallbacks)
{

}
//-----------------------------------------------------------------------------
HMQtSimpleAsyncConnection::HMQtSimpleAsyncConnection(std::unique_ptr<QTcpSocket>&& inSocket, const ConCallbacks& inCallbacks) :
    HMQtAbstractAsyncConnection(std::move(inSocket), inCallbacks)
{

}
//-----------------------------------------------------------------------------
HMQtSimpleAsyncConnection::~HMQtSimpleAsyncConnection()
{
    disconnect();
}
//-----------------------------------------------------------------------------
std::unique_ptr<QTcpSocket> HMQtSimpleAsyncConnection::makeSocket(errors::error_code& outError)
{
    outError = make_error_code(errors::eNetError::neSuccess); // Изначально метим как успех
    return std::make_unique<QTcpSocket>();
}
//-----------------------------------------------------------------------------
