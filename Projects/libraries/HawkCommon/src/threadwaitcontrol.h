#ifndef HMTHREADWAITCONTROL_H
#define HMTHREADWAITCONTROL_H

/**
 * @file threadwaitcontrol.h
 * @brief Содержит описание контралёра выполнения потока
 */

#include <mutex>
#include <atomic>
#include <chrono>
#include <condition_variable>

namespace hmcommon
{
//-----------------------------------------------------------------------------
/**
 * @brief The HMThreadWaitControl class - Класс-контролёр выполнения потока
 *
 * @authors Alekseev_s
 * @date 21.04.2021
 */
class HMThreadWaitControl
{
private:

    std::mutex m_conditionDefender;     ///< Мьютекс, защищающий условную переменную
    std::condition_variable m_break;    ///< Условная переменная прерывания ожидания потока

    std::atomic_bool m_threadWork;      ///< Флаг работы потока

public:

    /**
     * @brief HMThreadWaitControl - Конструктор по умолчанию
     */
    HMThreadWaitControl();

    /**
     * @brief ~HMThreadWaitControl - Деструктор по умолчанию
     */
    ~HMThreadWaitControl();

    /**
     * @brief start - Метод разрешит "выполнение" потока
     */
    void start();

    /**
     * @brief wait_for - Метод приостановит выполнение текущего потока на указанное время
     * @param inWaitTime - Время приостановки
     */
    template<typename _Rep, typename _Period>
    void wait_for(const std::chrono::duration<_Rep, _Period>& inWaitTime);

    /**
     * @brief stop - Метод остановит выполнение потока и прервёт ожидание
     */
    void stop();

    /**
     * @brief doWork Метод вернёт флаг "продолжать работу" для потока
     * @return Вернёт флаг, разрешающий продолжение работы потока
     */
    bool doWork() const;
};
//-----------------------------------------------------------------------------
template<typename _Rep, typename _Period>
void HMThreadWaitControl::wait_for(const std::chrono::duration<_Rep, _Period>& inWaitTime)
{
    if (!m_threadWork) // Если уже поступила команда на завершение
        return; // Игнорируем таймаут

    std::unique_lock ul(m_conditionDefender);
    // Ожидаем либо срабатывания m_break (по !m_threadWork) либо таймаута inWaitTime
    m_break.wait_for(ul, inWaitTime, [&] { return !m_threadWork; });
}
//-----------------------------------------------------------------------------
} // namespace hmcommon

#endif // HMTHREADWAITCONTROL_H
