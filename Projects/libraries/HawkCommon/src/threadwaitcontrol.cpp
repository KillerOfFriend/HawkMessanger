#include "threadwaitcontrol.h"

using namespace hmcommon;

//-----------------------------------------------------------------------------
HMThreadWaitControl::HMThreadWaitControl()
{
    std::atomic_init(&m_threadWork, true); // Изначально разрешаем работу потока
}
//-----------------------------------------------------------------------------
HMThreadWaitControl::~HMThreadWaitControl()
{
    stop();
}
//-----------------------------------------------------------------------------
void HMThreadWaitControl::start()
{
    if (!m_threadWork) // Если работа ещё не разрешена
        m_threadWork.store(true); // Разрешаем работу потока
}
//-----------------------------------------------------------------------------
void HMThreadWaitControl::stop()
{
    {
        std::lock_guard<std::mutex> lk(m_conditionDefender);
        m_threadWork.store(false); // Останавливаем поток
    }
    m_break.notify_all(); // Шлём сигнал прирывания потока
}
//-----------------------------------------------------------------------------
bool HMThreadWaitControl::doWork() const
{
    return m_threadWork;
}
//-----------------------------------------------------------------------------
