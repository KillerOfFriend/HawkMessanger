#include <QCoreApplication>

#include <HawkLog.h>

#include "prototypeserver.h"

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

    std::uint16_t Port = 32112;

    QStringList Args = a.arguments();

    if (Args.size() >= 2)
        Port = Args[1].toUShort();

    initLogSystem();

    PrototypeServer PServer;
    errors::error_code Error = PServer.start(Port);

    if (Error)
        LOG_ERROR(Error.message_qstr());
    else
        LOG_INFO("Сервер успешно запущен...");

    return a.exec();
}//-----------------------------------------------------------------------------
