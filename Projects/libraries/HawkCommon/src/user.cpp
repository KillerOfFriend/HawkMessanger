#include "user.h"

#include <QCryptographicHash>

using namespace hmcommon;

//-----------------------------------------------------------------------------
HMUser::HMUser(const QUuid &inUuid, const QDateTime& inRegistrationDate)
    : m_uuid(inUuid),
      m_registrationDate(inRegistrationDate)
{

}
//-----------------------------------------------------------------------------
bool HMUser::operator== (const HMUser& inOther) const
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
void HMUser::setLogin(const QString& inLogin)
{ m_login = inLogin; }
//-----------------------------------------------------------------------------
QString HMUser::getLogin() const
{ return m_login; }
//-----------------------------------------------------------------------------
bool HMUser::setPassword(const QString& inPassword)
{
    bool Result = true;

    if (inPassword.isEmpty())
        Result = false;
    else
        m_passwordHash = QCryptographicHash::hash(inPassword.toLocal8Bit(), QCryptographicHash::Algorithm::Sha512);

    return Result;
}
//-----------------------------------------------------------------------------
void HMUser::setPasswordHash(const QByteArray& inPasswordHash)
{ m_passwordHash = inPasswordHash; }
//-----------------------------------------------------------------------------
QByteArray HMUser::getPasswordHash() const
{ return m_passwordHash; }
//-----------------------------------------------------------------------------
void HMUser::setName(const QString& inName)
{ m_name = inName; }
//-----------------------------------------------------------------------------
QString HMUser::getName() const
{ return m_name; }
//-----------------------------------------------------------------------------
void HMUser::setSex(const eSex& inSex)
{ m_sex = inSex; }
//-----------------------------------------------------------------------------
eSex HMUser::getSex() const
{ return m_sex; }
//-----------------------------------------------------------------------------
void HMUser::setBirthday(const QDate& inBirthday)
{ m_birthday = inBirthday; }
//-----------------------------------------------------------------------------
QDate HMUser::getBirthday() const
{ return m_birthday; }
//-----------------------------------------------------------------------------
