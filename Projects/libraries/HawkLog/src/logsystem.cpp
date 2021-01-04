#include "logsystem.h"

#include <system_error>

#include <QDir>
#include <QDateTime>

#include "customstreams.h"

using namespace hmlog;

#define M_DEBUG     blue << "[DEBUG]:" << reset
#define M_INFO      green << "[INFO]:" << reset
#define M_WARNING   yellow << "[WARNING]:" << reset
#define M_ERROR     red << "[ERROR]:" << reset

//-----------------------------------------------------------------------------
HMLogSystem::HMLogSystem()
{
    setConfig(eLogConfig::lcShowStdOut | eLogConfig::lcWriteFile); // По дефолту разрешаем вывод в терминал и запись в файл
    std::atomic_init(&m_workFinish, false); // Разрешаем создание файла логов
}
//-----------------------------------------------------------------------------
HMLogSystem::~HMLogSystem()
{
    m_workFinish = true; // Взводим флаг заверешния работы
    fileClose(); // Закрываем файл
}
//-----------------------------------------------------------------------------
HMLogSystem& HMLogSystem::getInstance()
{
    static HMLogSystem LogSystem;
    return LogSystem;
}
//-----------------------------------------------------------------------------
void HMLogSystem::setConfig(const uint8_t inConfigMask)
{
    m_config = inConfigMask;
    fileClose();
}
//-----------------------------------------------------------------------------
uint8_t HMLogSystem::getConfig() const
{ return  m_config;}
//-----------------------------------------------------------------------------
void HMLogSystem::logText(const QString& inMessage)
{
    processMsg(HMLogMessage(eLogMessageType::lmtText, inMessage));
}
//-----------------------------------------------------------------------------
void HMLogSystem::logDebug(const QString& inMessage)
{
    processMsg(HMLogMessage(eLogMessageType::lmtDebug, inMessage));
}
//-----------------------------------------------------------------------------
void HMLogSystem::logInfo(const QString& inMessage)
{
    processMsg(HMLogMessage(eLogMessageType::lmtInfo, inMessage));
}
//-----------------------------------------------------------------------------
void HMLogSystem::logWarning(const QString& inMessage)
{
    processMsg(HMLogMessage(eLogMessageType::lmtWarning, inMessage));
}
//-----------------------------------------------------------------------------
void HMLogSystem::logError(const QString& inMessage)
{
    processMsg(HMLogMessage(eLogMessageType::lmtError, inMessage));
}
//-----------------------------------------------------------------------------
void HMLogSystem::processMsg(const HMLogMessage& inMsg)
{
    if (m_workFinish) // При завершении работы перестаём обрабатывать сообщения
        return;

    if (m_config & eLogConfig::lcShowStdOut) // Проверям разрешение на вывод в терминал
        printMsg(inMsg);

    if (m_config & eLogConfig::lcWriteFile) // Проверяем разрешение на запись в файл
        writeMsgToFile(inMsg);

    if (m_config & eLogConfig::lcContainMsg) // Проверяем разрешение на сохранение
    {
        std::lock_guard lg(m_listDefender);
        // Проверяем, что количество сообщений в контейнере не привысит максимально допустимого
        if (m_messages.size() + 1 >  m_maxContainMessages)
            m_messages.pop_back();

        m_messages.push_front(inMsg);
    }
}
//-----------------------------------------------------------------------------
void HMLogSystem::printMsg(const HMLogMessage& inMsg) const
{
    std::lock_guard lg(m_streamDefender);

    switch (inMsg.m_type)
    {
        case eLogMessageType::lmtInfo:      { QStdOut() << "<" + inMsg.m_time.toString() + "> " << M_INFO << " " << inMsg.m_message << Qt::endl; break; }
        case eLogMessageType::lmtDebug:     { QStdOut() << "<" + inMsg.m_time.toString() + "> " << M_DEBUG << " " << inMsg.m_message << Qt::endl; break; }
        case eLogMessageType::lmtWarning:   { QStdOut() << "<" + inMsg.m_time.toString() + "> " << M_WARNING << " " << inMsg.m_message << Qt::endl; break; }
        case eLogMessageType::lmtError:     { QStdOut() << "<" + inMsg.m_time.toString() + "> " << M_ERROR << " " << inMsg.m_message << Qt::endl; break; }

        default: { QStdOut() << " <" + inMsg.m_time.toString() + "> " << inMsg.m_message << Qt::endl; };
    }
}
//-----------------------------------------------------------------------------
#include <QCoreApplication>
bool HMLogSystem::fileOpen()
{
    std::error_code Error;

    if (m_workFinish) // Если завершается работа системы
        Error = std::make_error_code(std::errc::read_only_file_system);
    else // Система в рабочем режиме
    {
        std::lock_guard lg(m_fileDefender);

        if (!m_logFile || !m_logFile->isOpen()) // Файл не инициализирован или не открыт
        {
            QDir LogDir(QDir::currentPath() + "/LOG");

            if (!LogDir.exists()) // Если папки не существует
                LogDir.mkpath("."); // Создаём

            QString FilePath = LogDir.path() + ("/LOG_" + QDateTime::currentDateTime().toString("hh-mm-ss_dd.MM.yyyy") + ".txt");

            m_logFile = std::make_unique<QFile>(FilePath); // Инициализируем файл

            if (!m_logFile->open(QIODevice::WriteOnly | QIODevice::Text)) // Не удалось окрыть файл на запись
            {
                Error = std::make_error_code(std::errc::bad_file_descriptor);
                m_logFile = nullptr;
            }

        }
    }

    if (Error && (m_config & eLogConfig::lcShowStdOut)) // Если при открытии файлов произошла ошибка и разрешён вывод в терминал
        printMsg(HMLogMessage(eLogMessageType::lmtWarning, "Не удалось открыть/создать файл логов!"));

    return (!Error);
}
//-----------------------------------------------------------------------------
void HMLogSystem::writeMsgToFile(const HMLogMessage& inMsg)
{
    if (fileOpen() && m_logFile)
    {
        std::lock_guard lg(m_fileDefender);
        QTextStream LogStream(m_logFile.get());

        LogStream << "<" + inMsg.m_time.toString() + ">";

        switch (inMsg.m_type)
        {
            case eLogMessageType::lmtInfo:      { LogStream << " [INFO]: "; break; }
            case eLogMessageType::lmtDebug:     { LogStream << " [DEBUG]: "; break; }
            case eLogMessageType::lmtWarning:   { LogStream << " [WARNING]: "; break; }
            case eLogMessageType::lmtError:     { LogStream << " [ERROR]: "; break; }

            default: { LogStream << " "; }
        }

        LogStream << inMsg.m_message << Qt::endl;
        LogStream.flush();
    }
}
//-----------------------------------------------------------------------------
void HMLogSystem::fileClose()
{
    std::lock_guard lg(m_fileDefender);

    if (m_logFile)
    {
        if (m_logFile->isOpen())
        {
            m_logFile->flush();
            m_logFile->close();
        }

        m_logFile = nullptr;
    }
}
//-----------------------------------------------------------------------------
