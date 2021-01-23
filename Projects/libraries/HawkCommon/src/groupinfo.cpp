#include "groupinfo.h"

#include <systemerrorex.h>

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
