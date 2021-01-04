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

    mutable std::mutex m_streamDefender;            ///< Мьютекс, защищающий выходной поток

    std::list<HMLogMessage> m_messages;             ///< Список логов
    mutable std::mutex m_listDefender;              ///< Мьютекс, защищающий список особщений
    std::size_t m_maxContainMessages = 100;         ///< Максимальное количество хранимых сообщений

    std::unique_ptr<QFile> m_logFile = nullptr;     ///< Хендл файла логов
    mutable std::mutex m_fileDefender;              ///< Мьютекс, защищающий файл
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


// Вспомогательные макросы
#define AS_MACRO_STR(inVal)             # inVal
#define TO_MACRO_STR(inVal)             AS_MACRO_STR(inVal)
// Для глубокого дебага выводим информацию об исходном файле и строке
#define SRC_FILE_DATA                   ( "Source file: " __FILE__ " on line " TO_MACRO_STR(__LINE__) )

#if defined(__GNUC__)
    // Вывод информации о классе:методе (расширение gcc)
    #define SENDER_CLASS(inClass)           ( QString("In class method: ") + __PRETTY_FUNCTION__ )
#elif defined(_MSC_VER)
    #define SENDER_CLASS(inClass)           ( QString("In class method: ") + QString(typeid(inClass).name()) + ":" + __FUNCTION__ )
#endif

// Вывод изормации о функции
#define SENDER_FUNCTION                 ( QString("In function: ") + __FUNCTION__ )

#ifndef NDEBUG // В режиме дебага выводим доп информацию об исходном файле
    // Вывод изормации о функции с доп. информацией о сиходном файле
    #define SENDER_FUNCTION_EX          ( SENDER_FUNCTION + "\n\t" + SRC_FILE_DATA )
    // Вывод информации о классе::методе с доп. информацией о сиходном файле
    #define SENDER_CLASS_EX(inClass)    ( SENDER_CLASS(inClass) + "\n\t" + SRC_FILE_DATA )
#else // В релизной версии выводим только место "вызова"
    // Вывод изормации о функции
    #define SENDER_FUNCTION_EX          ( SENDER_FUNCTION )
    // Вывод информации о классе::методе
    #define SENDER_CLASS_EX(inClass)    ( SENDER_CLASS(inClass) )
#endif

//-----

// Макросы вывода сообщений
#define LOG_SYSTEM                      ( hmlog::HMLogSystem::getInstance() )
// Простой текст
#define LOG_TEXT(inMessage)             ( LOG_SYSTEM.logText(inMessage) )
// Информационное сообщение
#define LOG_INFO(inMessage)             ( LOG_SYSTEM.logInfo(inMessage) )
// Предупреждения (В режиме дебага, выведет доп. информацию об исходном файле и строке)
#define LOG_WARNING(inMessage)          ( LOG_SYSTEM.logWarning(inMessage + QString(" " + SENDER_FUNCTION_EX)) )
#define LOG_WARNING_EX(inMessage)       ( LOG_SYSTEM.logWarning(inMessage + QString(" " + SENDER_CLASS_EX(this))) )
// Ошибки (В режиме дебага, выведет доп. информацию об исходном файле и строке)
#define LOG_ERROR(inMessage)            ( LOG_SYSTEM.logError(inMessage + QString(" " + SENDER_FUNCTION_EX)) )
#define LOG_ERROR_EX(inMessage)         ( LOG_SYSTEM.logError(inMessage + QString(" " + SENDER_CLASS_EX(this))) )
// Дебаг (Сообщение дебага не даст информации об исходном файле и строке)
#ifndef NDEBUG
    #define LOG_DEBUG(inMessage)        ( LOG_SYSTEM.logDebug(inMessage + QString(" " + SENDER_FUNCTION)) )
    #define LOG_DEBUG_EX(inMessage)     ( LOG_SYSTEM.logDebug(inMessage + QString(" " + SENDER_CLASS(this))) )
#else
    #define LOG_DEBUG(inMessage)
    #define LOG_DEBUG_EX(inMessage)
#endif // NDEBUG

#endif // LOGSYSTEM_H
