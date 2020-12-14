#include "abstractcahcedatastorage.h"

#include "systemerrorex.h"

using namespace hmservcommon::datastorage;

//-----------------------------------------------------------------------------
std::vector<QUuid> HMAbstractCahceDataStorage::getUserContactsIDList(const QUuid& inUserUUID,  std::error_code& outErrorCode) const
{
    Q_UNUSED(inUserUUID);
    outErrorCode = make_error_code(hmcommon::eSystemErrorEx::seOperationNotSupported); // Кеширующмй контейнер не поддерживает данную операцию
    return {};
}
//-----------------------------------------------------------------------------
std::vector<QUuid> HMAbstractCahceDataStorage::getGroupUserIDList(const QUuid& inGroupUUID,  std::error_code& outErrorCode) const
{
    Q_UNUSED(inGroupUUID);
    outErrorCode = make_error_code(hmcommon::eSystemErrorEx::seOperationNotSupported); // Кеширующмй контейнер не поддерживает данную операцию
    return {};
}
//-----------------------------------------------------------------------------
