#include "cached.h"

using namespace hmservcommon;

//-----------------------------------------------------------------------------
//HMCachedUser
//-----------------------------------------------------------------------------
HMCachedUser::HMCachedUser(const std::shared_ptr<hmcommon::HMUserInfo> inUser) :
    m_user(inUser),
    m_lastRequest(std::chrono::system_clock::now())
{
    assert(m_user != nullptr);
}
//-----------------------------------------------------------------------------
HMCachedUser::HMCachedUser(HMCachedUser&& inOther) :
    m_user(inOther.m_user),
    m_lastRequest(std::move(inOther.m_lastRequest))
{
    inOther.m_user = nullptr;
    inOther.m_lastRequest = {};
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
// HMCachedUserContacts
//-----------------------------------------------------------------------------
HMCachedUserContacts::HMCachedUserContacts(const QUuid& inUserUUID, const std::shared_ptr<std::set<QUuid>> inContactList) :
    m_userUUID(inUserUUID),
    m_contactList(inContactList),
    m_lastRequest(std::chrono::system_clock::now())
{
    assert(m_contactList != nullptr);
}
//-----------------------------------------------------------------------------
HMCachedUserContacts::HMCachedUserContacts(HMCachedUserContacts&& inOther) :
    m_userUUID(inOther.m_userUUID),
    m_contactList(inOther.m_contactList),
    m_lastRequest(inOther.m_lastRequest)
{
    inOther.m_userUUID = QUuid();
    inOther.m_contactList = nullptr;
    inOther.m_lastRequest = {};
}
//-----------------------------------------------------------------------------
bool HMCachedUserContacts::operator == (const HMCachedUserContacts& inOther) const noexcept
{
    return this->m_userUUID == inOther.m_userUUID;
}
//-----------------------------------------------------------------------------
// HMCachedGroup
//-----------------------------------------------------------------------------
HMCachedGroup::HMCachedGroup(const std::shared_ptr<hmcommon::HMGroupInfo> inGroup) :
    m_group(inGroup),
    m_lastRequest(std::chrono::system_clock::now())
{
    assert(m_group != nullptr);
}
//-----------------------------------------------------------------------------
HMCachedGroup::HMCachedGroup(HMCachedGroup&& inOther) :
    m_group(inOther.m_group),
    m_lastRequest(inOther.m_lastRequest)
{
    inOther.m_group = nullptr;
    inOther.m_lastRequest = {};
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
// HMCachedGroupUsers
//-----------------------------------------------------------------------------
HMCachedGroupUsers::HMCachedGroupUsers(const QUuid& inGroupUUID, const std::shared_ptr<std::set<QUuid>> inGroupUsers) :
    m_group(inGroupUUID),
    m_groupUsers(inGroupUsers),
    m_lastRequest(std::chrono::system_clock::now())
{
    assert(m_groupUsers != nullptr);
}
//-----------------------------------------------------------------------------
HMCachedGroupUsers::HMCachedGroupUsers(HMCachedGroupUsers&& inOther) :
    m_group(inOther.m_group),
    m_groupUsers(inOther.m_groupUsers),
    m_lastRequest(inOther.m_lastRequest)
{
    inOther.m_group = QUuid();
    inOther.m_groupUsers = nullptr;
    inOther.m_lastRequest = {};
}
//-----------------------------------------------------------------------------
bool HMCachedGroupUsers::operator == (const HMCachedGroupUsers& inOther) const noexcept
{
    if (!inOther.m_groupUsers || !this->m_groupUsers)
        return false;
    else
        return inOther.m_group == this->m_group;
}
//-----------------------------------------------------------------------------
