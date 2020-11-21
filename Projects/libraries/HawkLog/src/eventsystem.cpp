#include "eventsystem.h"

#include "logsystem.h"

using namespace hmlog;

//-----------------------------------------------------------------------------
/**
 * @brief The eEventResult enum - Перечень результатов обработки события
 */
enum class eEventResult
{
    erIgnore,               ///< Игнорировать
    erTerminateAsFail,      ///< Завершиться с ошибкой
    erTerminateAsSucess,    ///< Завершиться штатно
    erContinue,             ///< Продолжить работу
    erError                 ///< Ошибка
};
//-----------------------------------------------------------------------------
#if defined(Q_OS_LINUX) // Определения для LINUX
    //-----------------------------------------------------------------------------
    #include <signal.h>
    //-----------------------------------------------------------------------------
    typedef std::uint32_t SystemSigCode;    ///< Код сигнала LINUX
    //-----------------------------------------------------------------------------
#elif defined(Q_OS_WIN) // Определения для WINDIWS
//-----------------------------------------------------------------------------
    #include <windows.h>
    //-----------------------------------------------------------------------------
    #define CALL_FIRST 1
    #define CALL_LAST 0
    //-----------------------------------------------------------------------------
    typedef DWORD SystemSigCode;            ///< Код сигнала WINDOWS
    static void* EVENT_HANDLE = nullptr;    ///< Указатель на обработчик событий Windows
    //-----------------------------------------------------------------------------
#endif


static const std::unordered_map<SystemSigCode, eCapturedEvents> C_SYSTEM_EVENTS = { ///< Перечень контролируемых обработчиков

#if defined(Q_OS_LINUX) // Сигналы для LINUX
    { SIGINT,   eCapturedEvents::ceInteractiveAttention },
    { SIGILL,   eCapturedEvents::ceIllegalInstruction },
    { SIGABRT,  eCapturedEvents::ceAbnormalTermination },
    { SIGFPE,   eCapturedEvents::ceErroneousArithmetic },
    { SIGSEGV,  eCapturedEvents::ceInvalidAccess },
    { SIGTERM,  eCapturedEvents::ceTerminationRequest },
    { SIGKILL,  eCapturedEvents::ceKilled }
#elif defined(Q_OS_WIN) // Сигналы для WINDOWS
    { EXCEPTION_ACCESS_VIOLATION,           eCapturedEvents::ceInteractiveAttention },
    { EXCEPTION_ILLEGAL_INSTRUCTION,        eCapturedEvents::ceIllegalInstruction },
    { EXCEPTION_NONCONTINUABLE_EXCEPTION,   eCapturedEvents::ceAbnormalTermination },
    { EXCEPTION_INVALID_DISPOSITION,        eCapturedEvents::ceInvalidAccess },
    { EXCEPTION_FLT_DENORMAL_OPERAND,       eCapturedEvents::ceErroneousArithmetic },
    { EXCEPTION_FLT_DIVIDE_BY_ZERO,         eCapturedEvents::ceErroneousArithmetic },
    { EXCEPTION_FLT_INEXACT_RESULT,         eCapturedEvents::ceErroneousArithmetic },
    { EXCEPTION_FLT_INVALID_OPERATION,      eCapturedEvents::ceErroneousArithmetic },
    { EXCEPTION_FLT_UNDERFLOW,              eCapturedEvents::ceErroneousArithmetic },
    { EXCEPTION_INT_DIVIDE_BY_ZERO,         eCapturedEvents::ceErroneousArithmetic },
    { EXCEPTION_INT_OVERFLOW,               eCapturedEvents::ceErroneousArithmetic },
#endif

};

//-----------------------------------------------------------------------------
/**
 * @brief SystemCodeToEvent - Функция преобразует системный сигнал в обрабатываемое событие
 * @param inSystemCode - Системный код события
 * @return Вернёт переопределение системного сигнала
 */
eCapturedEvents SystemCodeToEvent(const SystemSigCode& inSystemCode)
{
    const auto It = C_SYSTEM_EVENTS.find(inSystemCode); // Ищим переопределение системного сигнала

    if (It == C_SYSTEM_EVENTS.end()) // Если не наёдено
        return  eCapturedEvents::ceSilence;
    else // Если найдено
        return It->second;
}
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
HMEventSystem::HMEventSystem()
{
    setConfig(eCapturedEvents::ceAll); // При инициализации вулючаем обработку всех событий
}
//-----------------------------------------------------------------------------
HMEventSystem::~HMEventSystem()
{
    clear(); // При завершение отключаем обработчики
}
//-----------------------------------------------------------------------------
HMEventSystem& HMEventSystem::getInstance()
{
    static HMEventSystem EventSystem;
    return EventSystem;
}
//-----------------------------------------------------------------------------
void HMEventSystem::setConfig(const uint8_t inConfigMask)
{
    clear();
    m_config = inConfigMask;
    acceptConfig();
}
//-----------------------------------------------------------------------------
uint8_t HMEventSystem::getConfig() const
{ return  m_config; }
//-----------------------------------------------------------------------------
void HMEventSystem::setHandler(const eCapturedEvents& inEvent, UserHadle inHadle)
{
    if (inHadle == nullptr)
        return;

    std::lock_guard lg(m_userHandlesDefender);
    auto HandlesList = m_userHandles.find(inEvent); // Ищим список обработчиков указанного события

    if (HandlesList == m_userHandles.end()) // Если список обработчиков не обнаружен
        HandlesList = m_userHandles.insert(std::make_pair(inEvent, UserHandles())).first; // Добавляем новый

    auto Duplicate = std::find_if(HandlesList->second.begin(), HandlesList->second.end(), [&inHadle](UserHadle& Handle) // Проверяем что этот хендл ещё не обслуживается
    { return inHadle.target<void(*)()>() == Handle.target<void(*)()>(); }); // Сравниваем указатели на функции

    if (Duplicate == HandlesList->second.cend()) // Если хендл ещё не обслуживается
        HandlesList->second.push_back(inHadle); // Добавляем его на обслуживание
}
//-----------------------------------------------------------------------------
void HMEventSystem::dropHandel(const eCapturedEvents& inEvent, UserHadle inHadle)
{
    if (inHadle == nullptr)
        return;

    std::lock_guard lg(m_userHandlesDefender);
    auto HandlesList = m_userHandles.find(inEvent); // Ищим список обработчиков указанного события

    if (HandlesList != m_userHandles.end()) // Если список обработчиков обнаружен
    {
        auto ItHandle = std::find_if(HandlesList->second.begin(), HandlesList->second.end(), [&inHadle](UserHadle& Handle) // Проверяем что этот хендл ещё не обслуживается
        { return inHadle.target<void(*)()>() == Handle.target<void(*)()>(); }); // Сравниваем указатели на функции

        if (ItHandle != HandlesList->second.end()) // Если обработчик найден
            HandlesList->second.erase(ItHandle); // Удаляем обработчик

        if (HandlesList->second.empty()) // Если у события не осталось обработчиков
            m_userHandles.erase(HandlesList); // Удаляем список
    }
}
//-----------------------------------------------------------------------------
void HMEventSystem::processUserHadles(const eCapturedEvents& inEvent) const
{
    std::lock_guard lg(m_userHandlesDefender);
    auto HandlesList = m_userHandles.find(inEvent); // Ищим список обработчиков указанного события

    if (HandlesList != m_userHandles.end()) // Если список обработчиков обнаружен
        for(auto& Handle : HandlesList->second) // Перебираем обработчики пользователя
            Handle(); // Вызываем пользовательский обработчик
}
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
/**
 * @brief Handle - Финальный обработчик события
 * @param inEvent - Отловленное событие
 * @return Вернёт результат обработки
 */
eEventResult Handle(const eCapturedEvents inEvent)
{   // Все попавшие сюда события, гарантированно заданы в HMEventSystem::m_config
    eEventResult Result = eEventResult::erIgnore;
    HMEventSystem::getInstance().processUserHadles(inEvent); // Вызываем пользовательские обработчики

    switch (inEvent)
    {
        case ceInteractiveAttention:    { LOG_ERROR("SEGMENTATION FAULT!"); Result = eEventResult::erTerminateAsSucess; break; }

        case ceIllegalInstruction:      { LOG_ERROR("ILLEGAL INSTRUCTION!"); Result = eEventResult::erTerminateAsFail; break; }

        case ceAbnormalTermination:     { LOG_ERROR("ABNORMAL TERMINATION!"); Result = eEventResult::erTerminateAsFail; break; }

        case ceErroneousArithmetic:     { LOG_ERROR("ERRONEOUS ARITHMETIC!"); Result = eEventResult::erTerminateAsFail; break; }

        case ceInvalidAccess:           { LOG_ERROR("INVALID ACCESS!"); Result = eEventResult::erTerminateAsFail; break; }

#if defined(Q_OS_LINUX)

        case ceTerminationRequest:      { LOG_ERROR("TERMINATION REQUEST!"); Result = eEventResult::erTerminateAsSucess; break; }

        case ceKilled:                  { LOG_ERROR("KILL!"); Result = eEventResult::erTerminateAsSucess; break; }

#endif
        default:                        { LOG_WARNING("UNKNOWN EVENT NOT PROCESSED."); Result = eEventResult::erIgnore; }
    }

    return Result;
}
//-----------------------------------------------------------------------------

#if defined(Q_OS_LINUX)

    //-----------------------------------------------------------------------------
    /**
     * @brief UnixHandler - Обработчик для LINUX (technology POSIX Signals)
     * @param inSignal - Перехваченый сигнал
     */
    void UnixHandler(int inSignal)
    {
        eEventResult EventResul = eEventResult::erIgnore;
        eCapturedEvents Event = SystemCodeToEvent(inSignal); // Преобразуем системный код в обрабатываемое событие
        // Проверяем что событие успешно преобразовано и обработка этого осбытия разрешена:
        bool isProcessed = (Event == eCapturedEvents::ceSilence) ? false : (HMEventSystem::getInstance().getConfig() & Event);

        if (isProcessed) // Если обработка разрешена
            EventResul = Handle(Event); // Передаём на обработку общему хендлу

        switch (EventResul) // Проверяем результат обработки
        {
            // Разрешено продолжить
            case eEventResult::erContinue:          { signal(inSignal, SIG_IGN); break; }
            // Требуется штатное заверешине программы
            case eEventResult::erTerminateAsSucess: { signal(inSignal, SIG_DFL); exit(EXIT_SUCCESS); break; }
            // Требуется не штатно завергение программы
            case eEventResult::erTerminateAsFail:   { signal(inSignal, SIG_DFL); exit(EXIT_FAILURE); break; }
            // Требуется сообщение об ошибке обработки
            case eEventResult::erError:             { signal(inSignal, SIG_ERR); break; }
            // Во всех остальных случаях вызвать дефолтный системный обработчик
            default:                                { signal(inSignal, SIG_DFL); }
        }
    }
    //-----------------------------------------------------------------------------
    void HMEventSystem::acceptConfig()
    {
        for (const auto& Signal : C_SYSTEM_EVENTS) // Перебираем все доступные к обработке системные сигналы
        {           
            if (m_config & Signal.second) // Если он требует обработки
            {   // Задаём хендл обработки для этого сигнала
                struct sigaction NewAction;

                NewAction.sa_handler = UnixHandler;
                sigemptyset(&NewAction.sa_mask);
                NewAction.sa_flags = 0;

                sigaction(Signal.first, &NewAction, NULL); // Устанавливаем новый обработчик события
            }
        }
    }
    //-----------------------------------------------------------------------------
    void HMEventSystem::clear()
    {
        for (const auto& Signal : C_SYSTEM_EVENTS) // Перебираем события
            sigaction(Signal.first, NULL, NULL); // Сбрасываем всем обработчики

        m_config = eCapturedEvents::ceSilence; // Сбрасываем фильтр
    }
    //-----------------------------------------------------------------------------

#elif defined(Q_OS_WIN)

    //-----------------------------------------------------------------------------
    /**
     * @brief WinHandler - Обработчик событий Windows (technology Vectored Exception Handling (VEH))
     * @param ExceptionInfo - Информация о событии
     * @return Вернёт результат обработки
     */
    LONG WINAPI WinHandler(struct _EXCEPTION_POINTERS* ExceptionInfo)
    {
        eEventResult EventResul = eEventResult::erIgnore;
        LONG Result = EXCEPTION_CONTINUE_SEARCH; // По умолчанию продолжать поиск

        if (ExceptionInfo && ExceptionInfo->ExceptionRecord) // Если поступили валдные данные
        {
            eCapturedEvents Event = SystemCodeToEvent(ExceptionInfo->ExceptionRecord->ExceptionCode); // Преобразуем системный код в обрабатываемое событие
            // Проверяем что событие успешно преобразовано и обработка этого осбытия разрешена:
            bool isProcessed = (Event == eCapturedEvents::ceSilence) ? false : (HMEventSystem::getInstance().getConfig() & Event);

            if (isProcessed) // Если обработка разрешена
                EventResul = Handle(Event); // Передаём на обработку общему обработчику

            switch (EventResul) // Проверяем результат обработки
            {
                // Разрешено продолжить
                case eEventResult::erContinue:          { Result = EXCEPTION_CONTINUE_EXECUTION; break; }
                // Требуется штатное заверешине программы
                case eEventResult::erTerminateAsSucess: { Result = EXCEPTION_CONTINUE_SEARCH; exit(EXIT_SUCCESS); break; }
                // Требуется не штатно завергение программы
                case eEventResult::erTerminateAsFail:   { Result = EXCEPTION_CONTINUE_SEARCH; exit(EXIT_FAILURE); break; }
                // Требуется сообщение об ошибке обработки
                case eEventResult::erError:             { Result = EXCEPTION_EXECUTE_HANDLER; break; }
                // Во всех остальных случаях вызвать дефолтный системный обработчик
                default:                                { Result = EXCEPTION_CONTINUE_SEARCH; }
            }
        }

        return Result;
    }
    //-----------------------------------------------------------------------------
    void HMEventSystem::acceptConfig()
    {
        if (!EVENT_HANDLE)
            EVENT_HANDLE = AddVectoredExceptionHandler(CALL_FIRST, WinHandler); // Подсовываем системе свой обработчик
    }
    //-----------------------------------------------------------------------------
    void HMEventSystem::clear()
    {
        if (EVENT_HANDLE) // Если задан обработчик
        {
            RemoveVectoredExceptionHandler(EVENT_HANDLE); // Отключаем его
            EVENT_HANDLE = nullptr;
        }

        m_config = eCapturedEvents::ceSilence; // Сбрасываем фильтр
    }
    //-----------------------------------------------------------------------------

#endif
//-----------------------------------------------------------------------------
