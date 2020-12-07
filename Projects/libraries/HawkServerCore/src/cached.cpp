#include "cached.h"

using namespace hmservcommon;

//-----------------------------------------------------------------------------
//HMCachedUser
//-----------------------------------------------------------------------------
HMCachedUser::HMCachedUser(const std::shared_ptr<hmcommon::HMUser> inUser, const QTime& inCreateTime) :
    m_user(inUser),
    m_lastRequest(inCreateTime)
{
    assert(m_user != nullptr);
}
//-----------------------------------------------------------------------------
HMCachedUser::HMCachedUser(HMCachedUser&& inOther) :
    m_user(inOther.m_user),
    m_lastRequest(inOther.m_lastRequest)
{
    inOther.m_user = nullptr;
    inOther.m_lastRequest = QTime();
}
//-----------------------------------------------------------------------------
bool HMCachedUser::operator== (const HMCachedUser& inOther) const noexcept
{
    if (!inOther.m_user || !this->m_user)
        return false;
    else
        return inOther.m_user->m_uuid == this->m_user->m_uuid;
}
//-----------------------------------------------------------------------------
// HMCachedGroup
//-----------------------------------------------------------------------------
HMCachedGroup::HMCachedGroup(const std::shared_ptr<hmcommon::HMGroup> inGroup, const QTime& inCreateTime) :
    m_group(inGroup),
    m_lastRequest(inCreateTime)
{
    assert(m_group != nullptr);
}
//-----------------------------------------------------------------------------
HMCachedGroup::HMCachedGroup(HMCachedGroup&& inOther) :
    m_group(inOther.m_group),
    m_lastRequest(inOther.m_lastRequest)
{
    inOther.m_group = nullptr;
    inOther.m_lastRequest = QTime();
}
//-----------------------------------------------------------------------------
bool HMCachedGroup::operator== (const HMCachedGroup& inOther) const noexcept
{
    if (!inOther.m_group || !this->m_group)
        return false;
    else
        return inOther.m_group->m_uuid == this->m_group->m_uuid;
}
//-----------------------------------------------------------------------------
