#include "abstractcahcedatastorage.h"

#include "HawkLog.h"
#include "systemerrorex.h"
#include "datastorage/datastorageerrorcategory.h"

using namespace hmservcommon::datastorage;

HMAbstractCahceDataStorage::HMAbstractCahceDataStorage() : HMAbstractDataStorageFunctional()
{
    std::atomic_init(&m_threadWork, false); // По умолчанию не разрешаем работу потока
}
//-----------------------------------------------------------------------------
std::error_code HMAbstractCahceDataStorage::open()
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
std::error_code HMAbstractCahceDataStorage::startCacheWatchdogThread()
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
        m_threadWork.store(false); // Останавливаем поток

        if (m_watchdogThread.joinable())
            m_watchdogThread.join(); // Ожидаем завершения потока
    }
}
//-----------------------------------------------------------------------------
void HMAbstractCahceDataStorage::cacheWatchdogThreadFunc()
{
    LOG_DEBUG_EX("cacheWatchdogThreadFunc Started");

    while (m_threadWork)
    {
        processCacheInThread(); // Обрабатываем кеш
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Замораживаем поток
    }

    LOG_DEBUG_EX("cacheWatchdogThreadFunc Finished");
}
//-----------------------------------------------------------------------------
