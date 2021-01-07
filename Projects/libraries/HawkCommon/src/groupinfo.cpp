#include "groupinfo.h"

#include "systemerrorex.h"

using namespace hmcommon;

//-----------------------------------------------------------------------------
HMGroupInfo::HMGroupInfo(const QUuid &inUuid, const QDateTime& inRegistrationDate) :
    m_uuid(inUuid),
    m_registrationDate(inRegistrationDate)
{

}
//-----------------------------------------------------------------------------
bool HMGroupInfo::operator== (const HMGroupInfo& inOther) const
{
    if (&inOther == this) // Если это один экземпляр объекта
        return true;
    else
    {   // ПОЛНОЕ сравнение объекта
        return this->m_uuid == inOther.m_uuid &&
                this->m_registrationDate == inOther.m_registrationDate &&
                this->m_name == inOther.m_name;
    }
}
//-----------------------------------------------------------------------------
void HMGroupInfo::setName(const QString& inName)
{ m_name = inName; }
//-----------------------------------------------------------------------------
QString HMGroupInfo::getName() const
{ return m_name; }
//-----------------------------------------------------------------------------
//bool HMGroupInfo::isUsersEmpty() const
//{ return  m_users.empty(); }
////-----------------------------------------------------------------------------
//std::size_t HMGroupInfo::usersCount() const
//{ return m_users.size(); }
////-----------------------------------------------------------------------------
//std::error_code HMGroupInfo::addUser(const std::shared_ptr<HMUserInfo> inUser)
//{
//    std::error_code Result = make_error_code(eSystemErrorEx::seSuccess);

//    if (!inUser)
//        Result = make_error_code(eSystemErrorEx::seInvalidPtr);
//    else
//    {
//        if (contain(inUser->m_uuid))
//            Result = make_error_code(eSystemErrorEx::seAlredyInContainer);
//        else
//            m_users.push_back(inUser);
//    }

//    return Result;
//}
////-----------------------------------------------------------------------------
//std::error_code HMGroupInfo::removeUser(const QUuid& inUserUuid)
//{
//    std::error_code Result = make_error_code(eSystemErrorEx::seSuccess);

//    auto It = std::find_if(m_users.begin(), m_users.end(), [&inUserUuid](const std::shared_ptr<HMUserInfo>& User)
//    { return User->m_uuid == inUserUuid; });

//    if (It == m_users.end())
//        Result = make_error_code(eSystemErrorEx::seNotInContainer);
//    else
//        m_users.erase(It);

//    return Result;
//}
////-----------------------------------------------------------------------------
//std::shared_ptr<HMUserInfo> HMGroupInfo::getUser(const std::size_t inIndex, std::error_code& outErrorCode)
//{
//    std::shared_ptr<HMUserInfo> Result = nullptr;
//    outErrorCode = make_error_code(eSystemErrorEx::seSuccess);

//    if (isUsersEmpty())
//        outErrorCode = make_error_code(eSystemErrorEx::seContainerEmpty);
//    else
//    {
//        if (inIndex >= m_users.size())
//            outErrorCode = make_error_code(eSystemErrorEx::seIndexOutOfContainerRange);
//        else
//            Result = m_users[inIndex];
//    }

//    return Result;
//}
////-----------------------------------------------------------------------------
//bool HMGroupInfo::contain(const QUuid& inUserUuid) const
//{
//    return m_users.end() != std::find_if(m_users.begin(), m_users.end(), [&inUserUuid](const std::shared_ptr<HMUserInfo>& User)
//    { return User->m_uuid == inUserUuid; });
//}
////-----------------------------------------------------------------------------
