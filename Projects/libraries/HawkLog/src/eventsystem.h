#ifndef HMEVENTSYSTEM_H
#define HMEVENTSYSTEM_H

#include <mutex>
#include <vector>
#include <atomic>
#include <functional>
#include <unordered_map>

#include <qsystemdetection.h>

namespace hmlog
{
//-----------------------------------------------------------------------------
/**
 * @brief The eCapturedEvents enum - Перечень перехватываемых событий
 *
 * @author Alekseev_s
 * @date 15.11.2020
 */
enum eCapturedEvents : std::uint8_t
{
    ceSilence =                     0,      ///< Нет логирования
    ceInteractiveAttention =        1,
    ceIllegalInstruction =          2,      ///< Не легальная инструкция
    ceAbnormalTermination =         4,      ///< Аномальное завершение работы
    ceErroneousArithmetic =         8,      ///< Ошибка в арифметике
    ceInvalidAccess =               16,     ///< Ошибка досупа к памяти (segfault)

#if defined(Q_OS_LINUX)
    ceTerminationRequest =          32,     ///< Поступил запрос на завершение
    ceKilled =                      64,     ///< Поступил запрос на уничтожение процесса
#endif

    ceAll =                                 ///< Все доступные события
            ceInteractiveAttention | ceIllegalInstruction | ceAbnormalTermination |
            ceErroneousArithmetic | ceInvalidAccess
#if defined(Q_OS_LINUX)
            | ceTerminationRequest | ceKilled
#endif

};
//-----------------------------------------------------------------------------
/**
 * @brief The HMEventSystem class - Класс описывает систему перехвата системных событий
 *
 * Все перехваченые системные события будут автомтически переданы в HMLogSystem и обработаны
 * согласно его настройкам.
 *
 * Передварительно будут вызваны все полученные от польователя обработчики в порядке их добавления.
 *
 * @author Alekseev_s
 * @date 15.11.2020
 */
class HMEventSystem
{

private:

    typedef std::function<void()> UserHadle;
    typedef std::vector<UserHadle> UserHandles;

    std::atomic<uint8_t> m_config;                                  ///< Настройки логирования
    mutable std::mutex m_userHandlesDefender;                       ///< Мьютекс, защищающий пользовательские обработчики
    std::unordered_map<eCapturedEvents, UserHandles> m_userHandles; ///< Перечень пользовательских обработчиков

    /**
     * @brief HMLogSystem - Конструктор по умолчанию
     */
    HMEventSystem();

    /**
     * @brief ~HMLogSystem - Деструктор
     */
    ~HMEventSystem();

    /**
     * @brief acceptConfig - Метод задаст обработчики событий согласно настройкам
     */
    void acceptConfig();

    /**
     * @brief clear - Метод сборсит обработчики событий
     */
    void clear();

public:

    // Копирование запрещено
    HMEventSystem(const HMEventSystem& inOther) = delete;
    HMEventSystem& operator=(HMEventSystem& inOther) = delete;

    /**
     * @brief getInstance - Метод вернёт единственный экземпляр системы перехвата системных событий
     * @return Вернёт ссылку на систему перехвата системных событий
     */
    static HMEventSystem& getInstance();

    /**
     * @brief setConfig - Метод задаст настройки системы перехвата системных событий
     * @param inConfigMask - Маска параметров системы перехвата системных событий
     */
    void setConfig(const uint8_t inConfigMask);

    /**
     * @brief getConfig - Метод вернёт настройки системы перехвата системных событий
     * @return Вернёт настройки системы перехвата системных событий
     */
    uint8_t getConfig() const;

    /**
     * @brief setHandler - Метод добавит пользовательский обработчик
     * @param inEvent - Обрабатываемое событие
     * @param inHadle - Обработчик
     */
    void setHandler(const eCapturedEvents& inEvent, UserHadle inHadle);

    /**
     * @brief dropHandel - Метод удалит пользовательский обработчик
     * @param inEvent - Обрабатываемое событие
     * @param inHadle - Обработчик
     */
    void dropHandel(const eCapturedEvents& inEvent, UserHadle inHadle);

    /**
     * @brief processUserHadles - Метод вызовет все пользовательские обработчики
     * @param inEvent - Возбуждённое событие
     */
    void processUserHadles(const eCapturedEvents& inEvent) const;

};
//-----------------------------------------------------------------------------
} // namespace hmlog


#endif // HMEVENTSYSTEM_H
