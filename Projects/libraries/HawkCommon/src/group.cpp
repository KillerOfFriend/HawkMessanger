#include "group.h"

#include "systemerrorex.h"

using namespace hmcommon;

//-----------------------------------------------------------------------------
HMGroup::HMGroup(const QUuid &inUuid, const QDateTime& inRegistrationDate) :
    m_uuid(inUuid),
    m_registrationDate(inRegistrationDate)
{

}
//-----------------------------------------------------------------------------
bool HMGroup::operator== (const HMGroup& inOther) const
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
void HMGroup::setName(const QString& inName)
{ m_name = inName; }
//-----------------------------------------------------------------------------
QString HMGroup::getName() const
{ return m_name; }
//-----------------------------------------------------------------------------
bool HMGroup::isUsersEmpty() const
{ return  m_users.empty(); }
//-----------------------------------------------------------------------------
std::size_t HMGroup::usersCount() const
{ return m_users.size(); }
//-----------------------------------------------------------------------------
std::error_code HMGroup::addUser(const QUuid& inUserUuid)
{
    std::error_code Result = make_error_code(eSystemErrorEx::seSuccess);

    if (contain(inUserUuid))
        Result = make_error_code(eSystemErrorEx::seAlredyInContainer);
    else
        m_users.push_back(inUserUuid);

    return Result;
}
//-----------------------------------------------------------------------------
std::error_code HMGroup::removeUser(const QUuid& inUserUuid)
{
    std::error_code Result = make_error_code(eSystemErrorEx::seSuccess);

    auto It = std::find(m_users.begin(), m_users.end(), inUserUuid);

    if (It == m_users.end())
        Result = make_error_code(eSystemErrorEx::seNotInContainer);
    else
        m_users.erase(It);

    return Result;
}
//-----------------------------------------------------------------------------
QUuid HMGroup::getUser(const std::size_t inIndex, std::error_code& outErrorCode)
{
    QUuid Result;
    outErrorCode = make_error_code(eSystemErrorEx::seSuccess);

    if (isUsersEmpty())
        outErrorCode = make_error_code(eSystemErrorEx::seContainerEmpty);
    else
    {
        if (inIndex >= m_users.size())
            outErrorCode = make_error_code(eSystemErrorEx::seIndexOutOfContainerRange);
        else
            Result = m_users[inIndex];
    }

    return Result;
}
//-----------------------------------------------------------------------------
bool HMGroup::contain(const QUuid& inUserUuid) const
{
    return std::find(m_users.begin(), m_users.end(), inUserUuid) != m_users.end();
}
//-----------------------------------------------------------------------------
