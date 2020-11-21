#include "jsondatastorage.h"

//-----------------------------------------------------------------------------
HMJsonDataStorage::HMJsonDataStorage(const std::filesystem::__cxx11::path &inJsonPath) :
    HMDataStorage(),
    m_jsonPath(inJsonPath)
{

}
//-----------------------------------------------------------------------------
HMJsonDataStorage::~HMJsonDataStorage()
{

}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::open()
{

}
//-----------------------------------------------------------------------------
bool HMJsonDataStorage::is_open() const
{

}
//-----------------------------------------------------------------------------
void HMJsonDataStorage::close()
{

}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::addUser(const std::shared_ptr<HMUser> inUser)
{

}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::updateUser(const std::shared_ptr<HMUser> inUser)
{

}
//-----------------------------------------------------------------------------
std::shared_ptr<HMUser> HMJsonDataStorage::findUserByUUID(const QUuid &inUserUUID, std::error_code &outErrorCode)
{

}
//-----------------------------------------------------------------------------
std::shared_ptr<HMUser> HMJsonDataStorage::findUserByAuthentication(const QString &inLogin, const QByteArray &inPasswordHash, std::error_code &outErrorCode)
{

}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::addGroup(const std::shared_ptr<HMGroup> inGroup)
{

}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::updateGroup(const std::shared_ptr<HMGroup> inGroup)
{

}
//-----------------------------------------------------------------------------
std::shared_ptr<HMUser> HMJsonDataStorage::findGroupByUUID(const QUuid &inGroupUUID, std::error_code &outErrorCode)
{

}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::write() const
{

}
//-----------------------------------------------------------------------------
