#include "userinfo.h"

#include <QCryptographicHash>

using namespace hmcommon;

//-----------------------------------------------------------------------------
HMUserInfo::HMUserInfo(const QUuid &inUuid, const QDateTime& inRegistrationDate)
    : m_uuid(inUuid),
      m_registrationDate(inRegistrationDate)
{

}
//-----------------------------------------------------------------------------
bool HMUserInfo::operator== (const HMUserInfo& inOther) const
{
    if (&inOther == this) // Если это один экземпляр объекта
        return true;
    else
    {   // ПОЛНОЕ сравнение объекта
        return this->m_uuid == inOther.m_uuid &&
                this->m_registrationDate == inOther.m_registrationDate &&
                this->m_login == inOther.m_login &&
                this->m_passwordHash == inOther.m_passwordHash &&
                this->m_name == inOther.m_name &&
                this->m_sex == inOther.m_sex &&
                this->m_birthday == inOther.m_birthday;
    }
}
//-----------------------------------------------------------------------------
void HMUserInfo::setLogin(const QString& inLogin)
{ m_login = inLogin; }
//-----------------------------------------------------------------------------
QString HMUserInfo::getLogin() const
{ return m_login; }
//-----------------------------------------------------------------------------
bool HMUserInfo::setPassword(const QString& inPassword)
{
    bool Result = true;

    if (inPassword.isEmpty())
        Result = false;
    else
        m_passwordHash = QCryptographicHash::hash(inPassword.toLocal8Bit(), QCryptographicHash::Algorithm::Sha512);

    return Result;
}
//-----------------------------------------------------------------------------
void HMUserInfo::setPasswordHash(const QByteArray& inPasswordHash)
{ m_passwordHash = inPasswordHash; }
//-----------------------------------------------------------------------------
QByteArray HMUserInfo::getPasswordHash() const
{ return m_passwordHash; }
//-----------------------------------------------------------------------------
void HMUserInfo::setName(const QString& inName)
{ m_name = inName; }
//-----------------------------------------------------------------------------
QString HMUserInfo::getName() const
{ return m_name; }
//-----------------------------------------------------------------------------
void HMUserInfo::setSex(const eSex& inSex)
{ m_sex = inSex; }
//-----------------------------------------------------------------------------
eSex HMUserInfo::getSex() const
{ return m_sex; }
//-----------------------------------------------------------------------------
void HMUserInfo::setBirthday(const QDate& inBirthday)
{ m_birthday = inBirthday; }
//-----------------------------------------------------------------------------
QDate HMUserInfo::getBirthday() const
{ return m_birthday; }
//-----------------------------------------------------------------------------
