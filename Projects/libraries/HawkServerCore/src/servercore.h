#ifndef SERVERCORE_H
#define SERVERCORE_H

#include <memory>

#include "datastorage/DataStorage.h"

namespace hmservcommon
{
//-----------------------------------------------------------------------------
class HMServerCore
{

public:
    HMServerCore();
    ~HMServerCore();

private:

    std::unique_ptr<datastorage::HMDataStorage> m_dataStorage = nullptr; ///< Хранилище данных

};
//-----------------------------------------------------------------------------
}

#endif // SERVERCORE_H
