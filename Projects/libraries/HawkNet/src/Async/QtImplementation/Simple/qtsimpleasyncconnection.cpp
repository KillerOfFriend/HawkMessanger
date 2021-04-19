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
HMQtSimpleAsyncConnection::HMQtSimpleAsyncConnection(QTcpSocketPtr &&inConnection, const ConCallbacks& inCallbacks) :
    HMQtAbstractAsyncConnection(std::move(inConnection), inCallbacks)
{

}
//-----------------------------------------------------------------------------
HMQtSimpleAsyncConnection::~HMQtSimpleAsyncConnection()
{
    disconnect();
}
//-----------------------------------------------------------------------------
QTcpSocketPtr HMQtSimpleAsyncConnection::makeSocket(errors::error_code& outError)
{
    outError = make_error_code(errors::eNetError::neSuccess); // Изначально метим как успех
    return std::make_unique<QTcpSocket>();
}
//-----------------------------------------------------------------------------
