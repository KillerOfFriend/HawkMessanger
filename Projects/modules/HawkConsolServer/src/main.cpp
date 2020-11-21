#include <QCoreApplication>

#include <HawkLog.h>

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

    LOG_TEXT("LOG_TEXT");
    LOG_INFO("LOG_INFO");
    LOG_WARNING("LOG_WARNING");
    LOG_ERROR("LOG_ERROR");
    LOG_DEBUG("LOG_DEBUG");

    hmlog::HMEventSystem::getInstance().setHandler(hmlog::eCapturedEvents::ceInteractiveAttention, []() -> void
    {
        LOG_INFO("User Handle Success!");
    });

    int* i = nullptr;
    *i = 5;

    return a.exec();
}
//-----------------------------------------------------------------------------
