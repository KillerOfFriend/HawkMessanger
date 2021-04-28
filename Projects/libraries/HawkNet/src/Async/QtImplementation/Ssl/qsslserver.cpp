#include "qsslserver.h"

#include <memory>

#include <QSslSocket>

//-----------------------------------------------------------------------------
QSslServer::QSslServer(QSslConfiguration &inSslConfig, QObject *parent) :
    QTcpServer(parent),
    m_sslConfig(inSslConfig)
{

}
//-----------------------------------------------------------------------------
void QSslServer::incomingConnection(qintptr handle)
{
    std::unique_ptr<QSslSocket> NewSslSocket = std::make_unique<QSslSocket>(this);

    if (NewSslSocket->setSocketDescriptor(handle)) {
        NewSslSocket->setSslConfiguration(m_sslConfig);
        auto* s = NewSslSocket.release();
        addPendingConnection(s); // Добавляем во внутренний список ожидающих подключений
        //connect(NewSslSocket, &QSslSocket::encrypted, this, &QSslServer::ready);
        s->startServerEncryption();
    }
}
//-----------------------------------------------------------------------------
