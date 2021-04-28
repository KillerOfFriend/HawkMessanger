#include "qtsslasyncserver.h"

#include <neterrorcategory.h>

#include "qtsslasyncconnection.h"
#include "sslfuncs.h"

using namespace net;

//-----------------------------------------------------------------------------
HMQtSslAsyncServer::HMQtSslAsyncServer(const std::uint16_t inPort, const CertificatePaths& inCerPaths, const ServCallbacks& inCallbacks) :
    HMQtAbstractAsyncServer(inPort, inCallbacks),
    m_certificatePaths(inCerPaths)
{

}
//-----------------------------------------------------------------------------
HMQtSslAsyncServer::~HMQtSslAsyncServer()
{
    stop();
}
//-----------------------------------------------------------------------------
errors::error_code HMQtSslAsyncServer::start()
{
    errors::error_code Error = make_error_code(errors::eNetError::neSuccess); // Изначально метим как успех

    stop();
    // Формируем настройки серитфикатоа
    m_sslConfig = makeSslConfiguration(m_certificatePaths.m_certificatePath, m_certificatePaths.m_privateKey, Error);

    if (!Error)
        Error = HMQtAbstractAsyncServer::start(); // Вызываем запуск сервера предка

    return Error;
}
//-----------------------------------------------------------------------------
std::unique_ptr<QTcpServer> HMQtSslAsyncServer::makeServer(errors::error_code& outError)
{
    outError = make_error_code(errors::eNetError::neSuccess); // Изначально метим как успех
    return std::make_unique<QSslServer>(m_sslConfig, nullptr); // Формируем защищённый сервер QTcpSSL
}
//-----------------------------------------------------------------------------
std::unique_ptr<HMQtAbstractAsyncConnection> HMQtSslAsyncServer::makeConnection(std::unique_ptr<QTcpSocket>&& inSocket)
{
    return std::make_unique<HMQtSslAsyncConnection>(std::move(inSocket), m_Callbacks.m_conCalbacks);
}
//-----------------------------------------------------------------------------

