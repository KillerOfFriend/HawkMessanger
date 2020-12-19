#ifndef HMABSTRACTCAHCEDATASTORAGE_H
#define HMABSTRACTCAHCEDATASTORAGE_H

/**
 * @file abstractcahcedatastorage.h
 * @brief Содержит описание абстрактного класса кеширующего хранилища данных
 */

#include <thread>
#include <atomic>

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

    std::atomic_bool m_threadWork;      ///< Флаг работы потока
    std::thread m_watchdogThread;       ///< Поток контроля кеша

public:

    /**
     * @brief HMAbstractCahceDataStorage - Конструктор по умолчанию
     */
    HMAbstractCahceDataStorage();

    /**
     * @brief ~HMAbstractCahceDataStorage - Виртуальный деструктор по умолчанию
     */
    virtual ~HMAbstractCahceDataStorage() override = default;

    // Хранилище

    /**
     * @brief open - Метод откроет хранилище данных
     * @return Вернёт признак ошибки
     */
    virtual std::error_code open() override;

    /**
     * @brief is_open - Метод вернёт признак открытости хранилища данных
     * @return Вернёт признак открытости
     */
    virtual bool is_open() const override;

    /**
     * @brief close - Метод закроет хранилище данных
     */
    virtual void close() override;

protected:

    /**
     * @brief startCacheWatchdogThread - Метод запустит поток, котролирующий объекты кеша
     * @return Вернёт признак ошибки
     */
    virtual std::error_code startCacheWatchdogThread();

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
