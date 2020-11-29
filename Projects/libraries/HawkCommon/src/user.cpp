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
