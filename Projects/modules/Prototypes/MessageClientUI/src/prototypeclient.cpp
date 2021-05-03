#include "prototypeclient.h"

#include <cassert>

#include <HawkLog.h>
#include <HawkCommon.h>
#include <neterrorcategory.h>

//-----------------------------------------------------------------------------
PrototypeClient::~PrototypeClient()
{
    disconnect();
}
//-----------------------------------------------------------------------------
errors::error_code PrototypeClient::connect(const std::string& inHost, const std::uint16_t inPort)
{
    disconnect();

    m_client = makeClient(inHost, inPort);
    assert(m_client != nullptr);

    return m_client->connect();
}
//-----------------------------------------------------------------------------
void PrototypeClient::disconnect()
{
    if (m_client && m_client->isConnected())
    {
        m_client->disconnect();
        m_client = nullptr;
    }
}
//-----------------------------------------------------------------------------
errors::error_code PrototypeClient::send(net::oByteStream&& inData)
{
    if (!m_client)
        return make_error_code(errors::eNetError::neSocketNotInit);

    return m_client->send(net::wrap(std::move(inData)));
}
//-----------------------------------------------------------------------------
net::ConCallbacks PrototypeClient::makeCallBacks()
{
    net::ConCallbacks CallBacks;

    CallBacks.m_DataCallBack =        std::bind(&PrototypeClient::onReceiveData, this, std::placeholders::_1, std::placeholders::_2);
    CallBacks.m_DisconnectCallBack =  std::bind(&PrototypeClient::onDisconnect, this, std::placeholders::_1);
    CallBacks.m_ErrorCallBack =       std::bind(&PrototypeClient::onError, this, std::placeholders::_1, std::placeholders::_2);

    return CallBacks;
}
//-----------------------------------------------------------------------------
std::unique_ptr<net::HMConnection> PrototypeClient::makeClient(const std::string& inHost, const std::uint16_t inPort)
{
#if (NET_IMPLEMENTATION==NET_IMPLEMENTATION_QT_SIMPLE)  // Сетевая реализация QtSimple
    LOG_INFO("QtSimple implementation client create");
    return std::make_unique<net::HMQtSimpleAsyncConnection>(inHost, inPort, makeCallBacks());
#elif (NET_IMPLEMENTATION==NET_IMPLEMENTATION_QT_SSL)   // Сетевая реализация QtSsl
    LOG_INFO("QtSsl implementation client create");
    return std::make_unique<net::HMQtSslAsyncConnection>(inHost, inPort, makeCallBacks());
#else
    return nullptr; // АХТУНГ ТОВАРИЩИ!
#endif
}
//-----------------------------------------------------------------------------
void PrototypeClient::onReceiveData(net::iByteStream&& inData, const std::size_t inSenderID)
{
    HM_UNUSED(inSenderID);
    inData = net::unwrap(std::move(inData));
    QString Text = QString::fromStdString(inData.str());

    LOG_TEXT(Text);
    sig_textReceived(Text);
}
//-----------------------------------------------------------------------------
void PrototypeClient::onDisconnect(const std::size_t inSenderID)
{
    HM_UNUSED(inSenderID);
    QString Text = "Disconnect";

    LOG_INFO(Text);
    sig_textReceived(Text);
}
//-----------------------------------------------------------------------------
void PrototypeClient::onError(const errors::error_code inError, const std::size_t inSenderID)
{
    HM_UNUSED(inSenderID);
    QString Text = "ERROR: "+ inError.message_qstr();

    LOG_ERROR(Text);
    sig_textReceived(Text);
}
//-----------------------------------------------------------------------------
