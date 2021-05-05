#include <QCoreApplication>

#include <chrono>
#include <iostream>

#include <HawkLog.h>

#include <QThread>

#include "prototypeclient.h"
#include "qinteractive.h"

//-----------------------------------------------------------------------------
void initLogSystem()
{
    // Настравиваем вывод логов в терминал и в файл
    hmlog::HMLogSystem::getInstance().setConfig(hmlog::eLogConfig::lcShowStdOut | hmlog::eLogConfig::lcWriteFile);
    // Включаем обработку всех опасных событий
    hmlog::HMEventSystem::getInstance().setConfig(hmlog::eCapturedEvents::ceAll);
}
//-----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    std::string Host = "127.0.0.1";
    std::uint16_t Port = 32112;

    QStringList Args = a.arguments();

    if (Args.size() >= 2)
        Host = Args[1].toStdString();
    if (Args.size() >= 3)
        Port = Args[2].toUShort();

    initLogSystem();

    QThread Thread;
    QInteractive Interactive;

    Interactive.moveToThread(&Thread);

    QObject::connect(&Thread, SIGNAL(started()), &Interactive, SLOT(slot_startInteractive()));
    QObject::connect(&Thread, SIGNAL(finished()), &Interactive, SLOT(slot_stopInteractive()));

    PrototypeClient PClient;
    errors::error_code Error = PClient.connect(Host, Port);

    if (Error)
        LOG_ERROR(Error.message_qstr());
    else
    {
        LOG_INFO("Клиент успешно подключен к серверу.");

        QObject::connect(&Interactive, &QInteractive::sig_sendMessage, &PClient, &PrototypeClient::slot_send);
        Thread.start();
    }

    return a.exec();
}
//-----------------------------------------------------------------------------
