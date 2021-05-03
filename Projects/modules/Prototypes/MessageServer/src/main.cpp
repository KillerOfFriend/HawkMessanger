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

    initLogSystem();

    PrototypeServer PServer;
    errors::error_code Error = PServer.start(32112);

    if (Error)
        LOG_ERROR(Error.message_qstr());
    else
        LOG_INFO("Сервер успешно запущен...");

    return a.exec();
}//-----------------------------------------------------------------------------
