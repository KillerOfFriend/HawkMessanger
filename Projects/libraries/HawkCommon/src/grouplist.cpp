#include "grouplist.h"

#include <cassert>

#include "systemerrorex.h"

using namespace hmcommon;

//-----------------------------------------------------------------------------
// GroupMakeHash
//-----------------------------------------------------------------------------
size_t GroupMakeHash::operator() (const std::shared_ptr<HMGroup> &inGroup) const noexcept
{
    assert(inGroup != nullptr);
    return std::hash<std::string>()(inGroup->m_info->m_uuid.toString().toStdString());
}
//-----------------------------------------------------------------------------
// GroupsCheckEqual
//-----------------------------------------------------------------------------
bool GroupsCheckEqual::operator()(const std::shared_ptr<HMGroup>& inFirstGroup, const std::shared_ptr<HMGroup>& inSecondGroup) const noexcept
{
    assert((inFirstGroup != nullptr) && (inSecondGroup != nullptr));
    return inFirstGroup->m_info->m_uuid == inSecondGroup->m_info->m_uuid;
}
//-----------------------------------------------------------------------------
// HMGroupList
//-----------------------------------------------------------------------------
bool HMGroupList::isEmpty() const
{  return m_contacts.empty(); }
//-----------------------------------------------------------------------------
std::size_t HMGroupList::count() const
{ return m_contacts.size(); }
//-----------------------------------------------------------------------------
bool HMGroupList::contain(const QUuid& inGroupUUID) const
{
    return std::find_if(m_contacts.begin(), m_contacts.end(), [&inGroupUUID](const std::shared_ptr<HMGroup>& Group)
    {
        return Group->m_info->m_uuid == inGroupUUID;
    }) != m_contacts.end();
}
//-----------------------------------------------------------------------------
bool HMGroupList::contain(const std::shared_ptr<HMGroup> inGroup) const
{
    if (!inGroup)
        return false;
    else
        return contain(inGroup->m_info->m_uuid);
}
//-----------------------------------------------------------------------------
std::error_code HMGroupList::add(const std::shared_ptr<HMGroup> inNewGroup)
{
    std::error_code Error = make_error_code(eSystemErrorEx::seSuccess); // Изначально считаем что ошбки нет

    if (!inNewGroup) // Проверяем валидность указателя
        Error = make_error_code(eSystemErrorEx::seInvalidPtr);
    else
    {
        if (!m_contacts.insert(inNewGroup).second) // Добавляем пользователя в контейнер
            Error = make_error_code(eSystemErrorEx::seAlredyInContainer);
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::shared_ptr<HMGroup> HMGroupList::get(const std::size_t inIndex, std::error_code& outErrorCode) const
{
    std::shared_ptr<HMGroup> Result = nullptr;
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
std::shared_ptr<HMGroup> HMGroupList::get(const QUuid inGroupUUID, std::error_code& outErrorCode) const
{
    std::shared_ptr<HMGroup> Result = nullptr;
    outErrorCode = make_error_code(eSystemErrorEx::seSuccess); // Изначально считаем что ошбки нет

    auto FindRes = std::find_if(m_contacts.begin(), m_contacts.end(), [&inGroupUUID](const std::shared_ptr<HMGroup>& Group)
    {
        return Group->m_info->m_uuid == inGroupUUID;
    });

    if (FindRes == m_contacts.end())
        outErrorCode = make_error_code(eSystemErrorEx::seNotInContainer);
    else
        Result = *FindRes;

    return Result;
}
//-----------------------------------------------------------------------------
std::error_code HMGroupList::remove(const std::size_t inIndex)
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
std::error_code HMGroupList::remove(const QUuid inGroupUUID)
{
    std::error_code Error = make_error_code(eSystemErrorEx::seSuccess); // Изначально считаем что ошбки нет

    auto FindRes = std::find_if(m_contacts.begin(), m_contacts.end(), [&inGroupUUID](const std::shared_ptr<HMGroup>& Group)
    {
        return Group->m_info->m_uuid == inGroupUUID;
    });

    if (FindRes == m_contacts.end())
        Error = make_error_code(eSystemErrorEx::seNotInContainer);
    else
        m_contacts.erase(FindRes);

    return Error;
}
//-----------------------------------------------------------------------------
