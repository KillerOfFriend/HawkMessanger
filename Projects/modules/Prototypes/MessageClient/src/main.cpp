#include <QCoreApplication>

#include <chrono>
#include <iostream>

#include <HawkLog.h>

#include <QThread>

#include "prototypeclient.h"
#include "qinteractive.h"

//-----------------------------------------------------------------------------
void wait(const std::chrono::milliseconds inWaitTime)
{
    auto TimeOut = std::chrono::high_resolution_clock::now() + inWaitTime;

    do
    {
        QCoreApplication::processEvents();
    }
    while(std::chrono::high_resolution_clock::now() < TimeOut);

}
//-----------------------------------------------------------------------------
void clientProcess(PrototypeClient& inClient)
{
    bool Continue = true;

    do
    {
        std::cout << "Print your message: ";
        std::string Message;
        std::getline(std::cin, Message);

        if (!Message.empty())
        {
            if (Message == "/q")
                Continue = false;
            else
            {
                net::oByteStream Data(Message);
                errors::error_code Error = inClient.send(std::move(Data));

                if (Error)
                    LOG_ERROR(Error.message_qstr());
                else
                    wait(std::chrono::milliseconds(300));
            }
        }
    }
    while (Continue);
}
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

    initLogSystem();

    QThread Thread;
    QInteractive Interactive;

    Interactive.moveToThread(&Thread);

    QObject::connect(&Thread, SIGNAL(started()), &Interactive, SLOT(slot_startInteractive()));
    QObject::connect(&Thread, SIGNAL(finished()), &Interactive, SLOT(slot_stopInteractive()));

    PrototypeClient PClient;
    errors::error_code Error = PClient.connect("127.0.0.1", 32112);

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
