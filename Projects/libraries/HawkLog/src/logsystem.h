#ifndef LOGSYSTEM_H
#define LOGSYSTEM_H

#include <list>
#include <mutex>
#include <memory>

#include <QFile>
#include <QString>

#include "logmessage.h"

namespace hmlog
{
//-----------------------------------------------------------------------------
/**
 * @brief The eLogConfig enum - Перечисление параметров настройки системы логирования
 *
 * @author Alekseev_s
 * @date 13.11.2020
 */
enum eLogConfig : uint8_t
{
    lcNone          = 0, ///< Бездействие
    lcShowStdOut    = 1, ///< Вывод логов в стандартный поток
    lcWriteFile     = 2, ///< Запись логов в файл
    lcContainMsg    = 4  ///< Хранение сообщений внутри системы
};
//-----------------------------------------------------------------------------
/**
 * @brief The HMLogSystem class - Класс, описывающий систему ведения логов
 *
 * @author Alekseev_s
 * @date 13.11.2020
 */
class HMLogSystem
{
private:

    uint8_t m_config;                               ///< Настройки логирования

    std::list<HMLogMessage> m_messages;             ///< Список логов
    std::mutex m_listDefender;                      ///< Мьютекс, защищающий список особщений
    std::size_t m_maxContainMessages = 100;         ///< Максимальное количество хранимых сообщений

    std::unique_ptr<QFile> m_logFile = nullptr;     ///< Хендл файла логов
    std::mutex m_fileDefender;                      ///< Мьютекс, защищающий файл
    std::atomic_bool m_workFinish;                  ///< Флаг, сообщающий о завершении работы

    /**
     * @brief HMLogSystem - Конструктор по умолчанию
     */
    HMLogSystem();

    /**
     * @brief ~HMLogSystem - Деструктор
     */
    ~HMLogSystem();

    /**
     * @brief processMsg - Метод обработки сообщений
     * @param inMsg - Полученное сообщение
     */
    void processMsg(const HMLogMessage& inMsg);

    /**
     * @brief printMsg - Метод печати сообщений
     * @param inMsg - Печатуемое сообщение
     */
    void printMsg(const HMLogMessage& inMsg) const;

    /**
     * @brief fileOpen - Метод откроет файл на запись
     * @return Вернёт признак успешности операции
     */
    bool fileOpen();

    /**
     * @brief writeMsgToFile - Метод записи в файл
     * @param inMsg - Записываемое сообщение
     */
    void writeMsgToFile(const HMLogMessage& inMsg);

    /**
     * @brief fileClose - Метод закроет файл
     */
    void fileClose();

public:

    // Копирование запрещено
    HMLogSystem(const HMLogSystem& inOther) = delete;
    HMLogSystem& operator=(HMLogSystem& inOther) = delete;

    /**
     * @brief getInstance - Метод вернёт единственный экземпляр системы логирования
     * @return Вернёт ссылку на систему логирования
     */
    static HMLogSystem& getInstance();

    /**
     * @brief setConfig - Метод задаст настройки системы логирования
     * @param inConfigMask - Маска параметров системы логирования
     */
    void setConfig(const uint8_t inConfigMask);

    /**
     * @brief getConfig - Метод вернёт настройки системы логирования
     * @return Вернёт настройки системы логирования
     */
    uint8_t getConfig() const;

    /**
     * @brief logText - Метод занесёт в лог текстовое сообщение
     * @param inMessage - Тест сообщения
     */
    void logText(const QString& inMessage);

    /**
     * @brief logDebug - Метод занесёт в лог отладочное сообщение
     * @param inMessage - Тест сообщения
     */
    void logDebug(const QString& inMessage);

    /**
     * @brief logInfo - Метод занесёт в лог информационное сообщение
     * @param inMessage - Тест сообщения
     */
    void logInfo(const QString& inMessage);

    /**
     * @brief logWarning - Метод занесёт в лог сообщение с предупреждением
     * @param inMessage - Тест сообщения
     */
    void logWarning(const QString& inMessage);

    /**
     * @brief logError - Метод занесёт в лог сообщение об ошибке
     * @param inMessage - Тест сообщения
     */
    void logError(const QString& inMessage);

};
//-----------------------------------------------------------------------------
} // namespace hmlog

// Макросы вывода сообщений
#define LOG_SYSTEM ( hmlog::HMLogSystem::getInstance() )
#define SENDER_LINE ( " Line: " + QString(__LINE__) )
#define SENDER_DATA(inClass) ( QString(typeid(inClass).name()) + "::" + __FUNCTION__ + SENDER_LINE )

#if defined(__GNUC__)

#elif defined(_MSC_VER)

#endif

// Простой текст
#define LOG_TEXT(inMessage) ( LOG_SYSTEM.logText(inMessage) )
// Информационное сообщение
#define LOG_INFO(inMessage) ( LOG_SYSTEM.logInfo(inMessage) )
// Предупреждения
#define LOG_WARNING(inMessage)              ( LOG_SYSTEM.logWarning(inMessage) )
#define LOG_WARNING_EX(inMessage, inClass)  ( LOG_WARNING(inMessage + " " + SENDER_DATA(inClass)) )
// Ошибки
#define LOG_ERROR(inMessage)                ( LOG_SYSTEM.logError(inMessage) )
#define LOG_ERROR_EX(inMessage, inClass)    ( LOG_ERROR(inMessage + " " + SENDER_DATA(inClass)) )
// Дебаг
#ifndef NDEBUG
    #define LOG_DEBUG(inMessage)                ( LOG_SYSTEM.logDebug(inMessage) )
    #define LOG_DEBUG_EX(inMessage, inClass)    ( LOG_DEBUG(inMessage + " " + SENDER_DATA(inClass)) )
#else
    #define LOG_DEBUG(inMessage)
    #define LOG_DEBUG_EX(inMessage, inClass)
#endif // NDEBUG

#endif // LOGSYSTEM_H
