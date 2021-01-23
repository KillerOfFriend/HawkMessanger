#include "abstractcahcedatastorage.h"

#include <cassert>

#include "HawkLog.h"
#include "systemerrorex.h"
#include "datastorage/datastorageerrorcategory.h"

using namespace hmservcommon::datastorage;

HMAbstractCahceDataStorage::HMAbstractCahceDataStorage(const std::chrono::milliseconds inCacheLifeTime, const std::chrono::milliseconds inSleep) :
    HMAbstractDataStorageFunctional(),
    m_cacheLifeTime(inCacheLifeTime),
    m_sleep(inSleep)
{
    assert(m_cacheLifeTime.count() != 0);
    assert(m_sleep.count() != 0);

    std::atomic_init(&m_threadWork, false); // По умолчанию не разрешаем работу потока
}
//-----------------------------------------------------------------------------
hmcommon::error_code HMAbstractCahceDataStorage::open()
{
    return startCacheWatchdogThread(); // Запускаем поток
}
//-----------------------------------------------------------------------------
bool HMAbstractCahceDataStorage::is_open() const
{
    return m_threadWork && m_watchdogThread.joinable();
}
//-----------------------------------------------------------------------------
void HMAbstractCahceDataStorage::close()
{
    stopCacheWatchdogThread(); // Останавливаем поток
}
//-----------------------------------------------------------------------------
void HMAbstractCahceDataStorage::setCacheLifeTime(const std::chrono::milliseconds inCacheLifeTime)
{ m_cacheLifeTime = inCacheLifeTime; }
//-----------------------------------------------------------------------------
std::chrono::milliseconds HMAbstractCahceDataStorage::getCacheLifeTime() const
{ return m_cacheLifeTime; }
//-----------------------------------------------------------------------------
void HMAbstractCahceDataStorage::setThreadSleep(const std::chrono::milliseconds inSleep)
{ m_sleep = inSleep; }
//-----------------------------------------------------------------------------
std::chrono::milliseconds HMAbstractCahceDataStorage::getThreadSleep() const
{ return m_sleep; }
//-----------------------------------------------------------------------------
hmcommon::error_code HMAbstractCahceDataStorage::startCacheWatchdogThread()
{
    stopCacheWatchdogThread(); // Убедимся, что поток стоит

    m_threadWork = true; // Разрешаем запуск потока
    m_watchdogThread = std::thread(std::bind(&HMAbstractCahceDataStorage::cacheWatchdogThreadFunc, this)); // Запускаем поток-надзиратель

    if (!m_watchdogThread.joinable())
    {
        stopCacheWatchdogThread();
        return make_error_code(hmcommon::eSystemErrorEx::seIncorretData);
    }
    else
        return make_error_code(eDataStorageError::dsSuccess);
}
//-----------------------------------------------------------------------------
void HMAbstractCahceDataStorage::stopCacheWatchdogThread()
{
    if (m_threadWork)
    {
        {
            std::lock_guard<std::mutex> lk(m_conditionDefender);
            m_threadWork.store(false); // Останавливаем поток
        }
        m_break.notify_one(); // Шлём сигнал прирывания потока

        if (m_watchdogThread.joinable())
            m_watchdogThread.join(); // Ожидаем завершения потока
    }
}
//-----------------------------------------------------------------------------
void HMAbstractCahceDataStorage::cacheWatchdogThreadFunc()
{
    LOG_DEBUG("cacheWatchdogThreadFunc Started");

    while (m_threadWork)
    {
        processCacheInThread(); // Обрабатываем кеш

        std::unique_lock ul(m_conditionDefender);
        m_break.wait_for(ul, m_sleep, [&] { return !m_threadWork; }); // Ожидаем либо срабатывания m_break (по !m_threadWork) либо таймаута m_sleep
    }

    LOG_DEBUG("cacheWatchdogThreadFunc Finished");
}
//-----------------------------------------------------------------------------
