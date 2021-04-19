#ifndef SERVERCORE_H
#define SERVERCORE_H

#include <memory>

#include "datastorage/DataStorage.h"
#include "HawkNet.h"
#include "builder.h"

namespace hmservcommon
{
//-----------------------------------------------------------------------------
class HMServerCore
{

public:
    HMServerCore();
    ~HMServerCore();

private:

    std::shared_ptr<datastorage::HMDataStorage> m_dataStorage = nullptr;    ///< Хранилище данных
    std::unique_ptr<HMBuilder> m_builder = nullptr; ///< "Сборщик"

    std::unique_ptr<net::HMServer> m_server = nullptr; ///< Сервер

};
//-----------------------------------------------------------------------------
}

#endif // SERVERCORE_H
