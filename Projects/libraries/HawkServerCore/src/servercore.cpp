#include "servercore.h"

#include <HawkLog.h>

using namespace hmservcommon;

//-----------------------------------------------------------------------------
HMServerCore::HMServerCore() :
    m_dataStorage(std::make_unique<datastorage::HMJsonDataStorage>(std::filesystem::path("Storage.JSON")))
{
    if (m_dataStorage)
    {
        std::error_code Error = m_dataStorage->open();

        if (Error)
            LOG_ERROR(QString::fromStdString(Error.message()));
    }
}
//-----------------------------------------------------------------------------
HMServerCore::~HMServerCore()
{
    if (m_dataStorage && m_dataStorage->is_open())
        m_dataStorage->close();
}
//-----------------------------------------------------------------------------
