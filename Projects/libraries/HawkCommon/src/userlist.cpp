#include "userlist.h"

#include <cassert>

#include "systemerrorex.h"

using namespace hmcommon;

//-----------------------------------------------------------------------------
// ContactHashCompare
//-----------------------------------------------------------------------------
size_t ContactMakeHash::operator() (const std::shared_ptr<HMUserInfo> &inContact) const noexcept
{
    assert(inContact != nullptr);
    return std::hash<std::string>()(inContact->m_uuid.toString().toStdString());
}
//-----------------------------------------------------------------------------
// ContactCheckEqual
//-----------------------------------------------------------------------------
bool ContactCheckEqual::operator()(const std::shared_ptr<HMUserInfo>& inLeftContact, const std::shared_ptr<HMUserInfo>& inRightContact) const noexcept
{
    assert((inLeftContact != nullptr) && (inRightContact != nullptr));
    return inLeftContact->m_uuid == inRightContact->m_uuid;
}
//-----------------------------------------------------------------------------
// HMUserInfoList
//-----------------------------------------------------------------------------
bool HMUserInfoList::isEmpty() const
{  return m_contacts.empty(); }
//-----------------------------------------------------------------------------
std::size_t HMUserInfoList::count() const
{ return m_contacts.size(); }
//-----------------------------------------------------------------------------
bool HMUserInfoList::contain(const QUuid& inUserUUID) const
{
    return m_contacts.find(std::make_shared<HMUserInfo>(inUserUUID)) != m_contacts.end();;
}
//-----------------------------------------------------------------------------
bool HMUserInfoList::contain(const std::shared_ptr<HMUserInfo> inUser) const
{
    if (!inUser)
        return false;
    else
        return m_contacts.find(inUser) != m_contacts.end();
}
//-----------------------------------------------------------------------------
std::error_code HMUserInfoList::add(const std::shared_ptr<HMUserInfo> inNewUser)
{
    std::error_code Error = make_error_code(eSystemErrorEx::seSuccess); // Изначально считаем что ошбки нет

    if (!inNewUser) // Проверяем валидность указателя
        Error = make_error_code(eSystemErrorEx::seInvalidPtr);
    else
    {
        if (!m_contacts.insert(inNewUser).second) // Добавляем пользователя в контейнер
            Error = make_error_code(eSystemErrorEx::seAlredyInContainer);
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::shared_ptr<HMUserInfo> HMUserInfoList::get(const std::size_t inIndex, std::error_code& outErrorCode) const
{
    std::shared_ptr<HMUserInfo> Result = nullptr;
    outErrorCode = make_error_code(eSystemErrorEx::seSuccess); // Изначально считаем что ошбки нет

    if (isEmpty())
        outErrorCode = make_error_code(eSystemErrorEx::seContainerEmpty);
    else
    {
        if (inIndex >= m_contacts.size())
            outErrorCode = make_error_code(eSystemErrorEx::seIndexOutOfContainerRange);
        else
        {
            auto It = m_contacts.begin();
            std::advance(It, inIndex);
            Result = *It;
        }
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::shared_ptr<HMUserInfo> HMUserInfoList::get(const QUuid inUserUuid, std::error_code& outErrorCode) const
{
    std::shared_ptr<HMUserInfo> Result = nullptr;
    outErrorCode = make_error_code(eSystemErrorEx::seSuccess); // Изначально считаем что ошбки нет

    auto FindRes = m_contacts.find(std::make_shared<HMUserInfo>(inUserUuid));

    if (FindRes == m_contacts.end())
        outErrorCode = make_error_code(eSystemErrorEx::seNotInContainer);
    else
        Result = *FindRes;

    return Result;
}
//-----------------------------------------------------------------------------
std::error_code HMUserInfoList::remove(const std::size_t inIndex)
{
    std::error_code Error = make_error_code(eSystemErrorEx::seSuccess); // Изначально считаем что ошбки нет

    if (inIndex >= m_contacts.size())
        Error = make_error_code(eSystemErrorEx::seIndexOutOfContainerRange);
    else
    {
        auto It = m_contacts.begin();
        std::advance(It, inIndex);
        m_contacts.erase(It);
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMUserInfoList::remove(const QUuid inUserUuid)
{
    std::error_code Error = make_error_code(eSystemErrorEx::seSuccess); // Изначально считаем что ошбки нет

    std::shared_ptr<HMUserInfo> TempFind = std::make_shared<HMUserInfo>(inUserUuid); // Формируем пользователя для поиска по UUID
    auto FindRes = m_contacts.find(TempFind);

    if (FindRes == m_contacts.end())
        Error = make_error_code(eSystemErrorEx::seNotInContainer);
    else
        m_contacts.erase(FindRes);

    return Error;
}
//-----------------------------------------------------------------------------
