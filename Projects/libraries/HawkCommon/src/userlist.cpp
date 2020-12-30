#include "userlist.h"

#include <cassert>

#include "systemerrorex.h"

using namespace hmcommon;

//-----------------------------------------------------------------------------
// ContactHashCompare
//-----------------------------------------------------------------------------
size_t ContactMakeHash::operator() (const std::shared_ptr<HMUser> &inContact) const noexcept
{
    assert(inContact != nullptr);
    return std::hash<std::string>()(inContact->m_uuid.toString().toStdString());
}
//-----------------------------------------------------------------------------
// ContactCheckEqual
//-----------------------------------------------------------------------------
bool ContactCheckEqual::operator()(const std::shared_ptr<HMUser>& inLeftContact, const std::shared_ptr<HMUser>& inRightContact) const noexcept
{
    assert((inLeftContact != nullptr) && (inRightContact != nullptr));
    return inLeftContact->m_uuid == inRightContact->m_uuid;
}
//-----------------------------------------------------------------------------
// HMUserList
//-----------------------------------------------------------------------------
bool HMUserList::isEmpty() const
{  return m_contacts.empty(); }
//-----------------------------------------------------------------------------
std::size_t HMUserList::count() const
{ return m_contacts.size(); }
//-----------------------------------------------------------------------------
bool HMUserList::contain(const QUuid& inUserUUID) const
{
    return m_contacts.find(std::make_shared<HMUser>(inUserUUID)) != m_contacts.end();;
}
//-----------------------------------------------------------------------------
bool HMUserList::contain(const std::shared_ptr<HMUser> inUser) const
{
    if (!inUser)
        return false;
    else
        return m_contacts.find(inUser) != m_contacts.end();
}
//-----------------------------------------------------------------------------
std::error_code HMUserList::add(const std::shared_ptr<HMUser> inNewUser)
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
std::shared_ptr<HMUser> HMUserList::get(const std::size_t inIndex, std::error_code& outErrorCode) const
{
    std::shared_ptr<HMUser> Result = nullptr;
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
std::shared_ptr<HMUser> HMUserList::get(const QUuid inUserUuid, std::error_code& outErrorCode) const
{
    std::shared_ptr<HMUser> Result = nullptr;
    outErrorCode = make_error_code(eSystemErrorEx::seSuccess); // Изначально считаем что ошбки нет

    auto FindRes = m_contacts.find(std::make_shared<HMUser>(inUserUuid));

    if (FindRes == m_contacts.end())
        outErrorCode = make_error_code(eSystemErrorEx::seNotInContainer);
    else
        Result = *FindRes;

    return Result;
}
//-----------------------------------------------------------------------------
std::error_code HMUserList::remove(const std::size_t inIndex)
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
std::error_code HMUserList::remove(const QUuid inUserUuid)
{
    std::error_code Error = make_error_code(eSystemErrorEx::seSuccess); // Изначально считаем что ошбки нет

    std::shared_ptr<HMUser> TempFind = std::make_shared<HMUser>(inUserUuid); // Формируем пользователя для поиска по UUID
    auto FindRes = m_contacts.find(TempFind);

    if (FindRes == m_contacts.end())
        Error = make_error_code(eSystemErrorEx::seNotInContainer);
    else
        m_contacts.erase(FindRes);

    return Error;
}
//-----------------------------------------------------------------------------
