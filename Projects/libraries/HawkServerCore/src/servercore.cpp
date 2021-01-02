#include "servercore.h"

#include <HawkLog.h>

using namespace hmservcommon;

//-----------------------------------------------------------------------------
HMServerCore::HMServerCore()
{
    m_dataStorage = std::make_shared<datastorage::HMCombinedDataStorage>( // Формируем хранилище данных
                std::make_shared<datastorage::HMJsonDataStorage>(std::filesystem::path("Storage.JSON")), // Из фисического (медленного) хранилища
                std::make_shared<datastorage::HMCachedMemoryDataStorage>()); // И кеширующего (быстрого) хранилища

    std::error_code Error = m_dataStorage->open(); // Пытаемся открыть хранилище

    if (Error)
        LOG_ERROR(QString::fromStdString(Error.message()));

    m_accountBuilder = std::make_unique<builders::HMAccountBuilder>(m_dataStorage); // Формируем билдер и передаём в него хранилище
}
//-----------------------------------------------------------------------------
HMServerCore::~HMServerCore()
{
    if (m_dataStorage && m_dataStorage->is_open())
        m_dataStorage->close();
}
//-----------------------------------------------------------------------------
