#ifndef HMABSTRACTCAHCEDATASTORAGE_H
#define HMABSTRACTCAHCEDATASTORAGE_H

/**
 * @file abstractcahcedatastorage.h
 * @brief Содержит описание абстрактного класса кеширующего хранилища данных
 */

#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>

#include "datastorage/interface/abstractdatastoragefunctional.h"

namespace hmservcommon::datastorage
{
//-----------------------------------------------------------------------------
/**
 * @brief The HMAbstractCahceDataStorage class - Класс, описывающий абстракцию кеширующего хранилища данных
 *
 * @authors Alekseev_s
 * @date 12.12.2020
 */
class HMAbstractCahceDataStorage : public HMAbstractDataStorageFunctional
{
private:

    std::chrono::milliseconds m_cacheLifeTime;  ///< Время жизни объектов кеша (в милисекундах)
    std::chrono::milliseconds m_sleep;          ///< Время ожидания потока контроля кеша (в милисекундах)

    std::mutex m_conditionDefender;             ///< Мьютекс, защищающий условную переменную
    std::condition_variable m_break;            ///< Условная переменная прерывания ожидания потока

    std::atomic_bool m_threadWork;              ///< Флаг работы потока
    std::thread m_watchdogThread;               ///< Поток контроля кеша

public:

    /**
     * @brief HMAbstractCahceDataStorage - Инициализирующий конструктор
     * @param inCacheLifeTime - Время жизни объектов кеша (в милисекундах)
     * @param inSleep - Время ожидания потока контроля кеша в (в милисекундах)
     */
    HMAbstractCahceDataStorage(const std::chrono::milliseconds inCacheLifeTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::minutes(15)),
                               const std::chrono::milliseconds inSleep = std::chrono::milliseconds(1000));

    /**
     * @brief ~HMAbstractCahceDataStorage - Виртуальный деструктор по умолчанию
     */
    virtual ~HMAbstractCahceDataStorage() override = default;

    // Хранилище

    /**
     * @brief open - Метод откроет хранилище данных
     * @return Вернёт признак ошибки
     */
    virtual errors::error_code open() override;

    /**
     * @brief is_open - Метод вернёт признак открытости хранилища данных
     * @return Вернёт признак открытости
     */
    virtual bool is_open() const override;

    /**
     * @brief close - Метод закроет хранилище данных
     */
    virtual void close() override;

    void setCacheLifeTime(const std::chrono::milliseconds inCacheLifeTime);

    std::chrono::milliseconds getCacheLifeTime() const;

    /**
     * @brief setThreadSleep - Метод задаст время ожидания потока контроля кеша
     * @param inSleep - Время ожидания потока контроля кеша в (в милисекундах)
     */
    void setThreadSleep(const std::chrono::milliseconds inSleep);

    /**
     * @brief getThreadSleep - Метод вернёт время ожидания потока контроля кеша
     * @return Вернёт время ожидания потока контроля кеша (в милисекундах)
     */
    std::chrono::milliseconds getThreadSleep() const;

protected:

    /**
     * @brief startCacheWatchdogThread - Метод запустит поток, котролирующий объекты кеша
     * @return Вернёт признак ошибки
     */
    virtual errors::error_code startCacheWatchdogThread();

    /**
     * @brief stopCacheWatchdogThread - Метод остановит поток, котролирующий объекты кеша
     */
    virtual void stopCacheWatchdogThread();

    /**
     * @brief processCacheInThread - Метод обработки кеша в потоке
     */
    virtual void processCacheInThread() = 0;

private:

    /**
     * @brief cacheWatchdogThreadFunc - Метод потока, обрабатывающего кеш
     */
    void cacheWatchdogThreadFunc();

};
//-----------------------------------------------------------------------------
} // namespace hmservcommon::datastorage

#endif // HMABSTRACTCAHCEDATASTORAGE_H
