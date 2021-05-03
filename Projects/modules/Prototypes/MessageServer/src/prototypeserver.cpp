#include "prototypeserver.h"

#include <cassert>
#include <functional>

#include <HawkLog.h>

//-----------------------------------------------------------------------------
PrototypeServer::~PrototypeServer()
{
    stop();
}
//-----------------------------------------------------------------------------
errors::error_code PrototypeServer::start(const std::int16_t inPort)
{
    stop();

    m_server = make_server(inPort);
    assert(m_server != nullptr);

    return m_server->start();
}
//-----------------------------------------------------------------------------
void PrototypeServer::stop()
{
    if (m_server && m_server->isStarted())
    {
        m_server->stop();
        m_server = nullptr;
    }
}
//-----------------------------------------------------------------------------
net::ServCallbacks PrototypeServer::makeCallBacks()
{
    net::ServCallbacks CallBacks;

    CallBacks.m_NewConnectionCallBack =             std::bind(&PrototypeServer::onNewConnection, this, std::placeholders::_1);
    CallBacks.m_ErrorCallBack =                     std::bind(&PrototypeServer::onServerError, this, std::placeholders::_1);

    CallBacks.m_conCalbacks.m_DataCallBack =        std::bind(&PrototypeServer::onReceiveData, this, std::placeholders::_1, std::placeholders::_2);
    CallBacks.m_conCalbacks.m_DisconnectCallBack =  std::bind(&PrototypeServer::onClientDisconnect, this, std::placeholders::_1);
    CallBacks.m_conCalbacks.m_ErrorCallBack =       std::bind(&PrototypeServer::onClientError, this, std::placeholders::_1, std::placeholders::_2);

    return CallBacks;
}
//-----------------------------------------------------------------------------
std::unique_ptr<net::HMServer> PrototypeServer::make_server(const std::int16_t inPort)
{
#if (NET_IMPLEMENTATION==NET_IMPLEMENTATION_QT_SIMPLE)  // Сетевая реализация QtSimple
    LOG_INFO("QtSimple implementation server create");
    return std::make_unique<net::HMQtSimpleAsyncServer>(inPort, makeCallBacks());
#elif (NET_IMPLEMENTATION==NET_IMPLEMENTATION_QT_SSL)   // Сетевая реализация QtSsl
    net::CertificatePaths certificatePaths;

    certificatePaths.m_certificatePath = std::filesystem::current_path() / "certificate.crt";
    certificatePaths.m_privateKey = std::filesystem::current_path() / "privateKey.key";

    LOG_INFO("QtSsl implementation server create");
    return std::make_unique<net::HMQtSslAsyncServer>(inPort, certificatePaths, makeCallBacks());
#else
    return nullptr; // АХТУНГ ТОВАРИЩИ!
#endif
}
//-----------------------------------------------------------------------------
void PrototypeServer::onNewConnection(const std::size_t inConnectionID)
{
    LOG_INFO("Client connected: [" + QString::number(inConnectionID) + "]");
}
//-----------------------------------------------------------------------------
void PrototypeServer::onReceiveData(net::iByteStream&& inData, const std::size_t inSenderID)
{
    if (!m_server)
        return;

    {
        std::lock_guard lg(m_dataReceiveDefender);
        LOG_TEXT("[" + QString::number(inSenderID) + "]: " + QString::fromStdString(inData.str()));
    }

    net::oByteStream Data(inData.str());
    auto SendResults = m_server->sendToAll(std::move(Data));

    for (const auto& SendRes : SendResults)
        LOG_ERROR("Send to [" + QString::number(SendRes.first) + "] ERROR: " + SendRes.second.message_qstr());
}
//-----------------------------------------------------------------------------
void PrototypeServer::onClientDisconnect(const std::size_t inSenderID)
{
    LOG_INFO("Client disconnected: [" + QString::number(inSenderID) + "]");
}
//-----------------------------------------------------------------------------
void PrototypeServer::onClientError(const errors::error_code inError, const std::size_t inSenderID)
{
    LOG_ERROR("Client [" + QString::number(inSenderID) + "] ERROR: " + inError.message_qstr());
}
//-----------------------------------------------------------------------------
void PrototypeServer::onServerError(const errors::error_code inError)
{
    LOG_ERROR("Server ERROR: " + inError.message_qstr());
}
//-----------------------------------------------------------------------------
