#include "abstractcahcedatastorage.h"

#include <cassert>

#include <HawkLog.h>
#include <systemerrorex.h>
#include <datastorageerrorcategory.h>

using namespace hmservcommon::datastorage;

HMAbstractCahceDataStorage::HMAbstractCahceDataStorage(const std::chrono::milliseconds inCacheLifeTime, const std::chrono::milliseconds inSleep) :
    HMAbstractDataStorageFunctional(),
    m_cacheLifeTime(inCacheLifeTime),
    m_sleep(inSleep)
{
    assert(m_cacheLifeTime.count() != 0);
    assert(m_sleep.count() != 0);
}
//-----------------------------------------------------------------------------
errors::error_code HMAbstractCahceDataStorage::open()
{
    return startCacheWatchdogThread(); // Запускаем поток
}
//-----------------------------------------------------------------------------
bool HMAbstractCahceDataStorage::is_open() const
{
    return m_hreadControl.doWork() && m_watchdogThread.joinable();
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
errors::error_code HMAbstractCahceDataStorage::startCacheWatchdogThread()
{
    stopCacheWatchdogThread(); // Убедимся, что поток стоит

    m_hreadControl.start(); // Разрешаем запуск потока
    m_watchdogThread = std::thread(std::bind(&HMAbstractCahceDataStorage::cacheWatchdogThreadFunc, this)); // Запускаем поток-надзиратель

    if (!m_watchdogThread.joinable())
    {
        stopCacheWatchdogThread();
        return make_error_code(errors::eSystemErrorEx::seIncorretData);
    }
    else
        return make_error_code(errors::eDataStorageError::dsSuccess);
}
//-----------------------------------------------------------------------------
void HMAbstractCahceDataStorage::stopCacheWatchdogThread()
{
    if (m_hreadControl.doWork())
    {
        m_hreadControl.stop();

        if (m_watchdogThread.joinable())
            m_watchdogThread.join(); // Ожидаем завершения потока
    }
}
//-----------------------------------------------------------------------------
void HMAbstractCahceDataStorage::cacheWatchdogThreadFunc()
{
    LOG_DEBUG("cacheWatchdogThreadFunc Started");

    while (m_hreadControl.doWork())
    {
        processCacheInThread(); // Обрабатываем кеш
        m_hreadControl.wait_for(m_sleep); // Ожидаем завершения или прирываания
    }

    LOG_DEBUG("cacheWatchdogThreadFunc Finished");
}
//-----------------------------------------------------------------------------
