#include "contactlist.h"

#include "user.h"
#include "systemerrorex.h"

using namespace hmcommon;

//-----------------------------------------------------------------------------
// ContactHashCompare
//-----------------------------------------------------------------------------
size_t ContactHashCompare::operator() (const std::shared_ptr<HMUser> &inUser) const
{
    return std::hash<std::string>()(inUser->m_uuid.toString().toStdString());
}
//-----------------------------------------------------------------------------
// HMContactList
//-----------------------------------------------------------------------------
bool HMContactList::isEmpty() const
{  return m_contacts.empty(); }
//-----------------------------------------------------------------------------
std::size_t HMContactList::contactsCount() const
{ return m_contacts.size(); }
//-----------------------------------------------------------------------------
std::error_code HMContactList::addContact(const std::shared_ptr<HMUser> inNewContact)
{
    std::error_code Error = make_error_code(eSystemErrorEx::seSuccess); // Изначально считаем что ошбки нет

    if (!inNewContact) // Проверяем валидность указателя
        Error = make_error_code(eSystemErrorEx::seInvalidPtr);
    else
    {
        if (!m_contacts.insert(inNewContact).second) // Добавляем контакт в контейнер
            Error = make_error_code(eSystemErrorEx::seAlredyInContainer);
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::shared_ptr<HMUser> HMContactList::getContact(const std::size_t inContactIndex, std::error_code& outErrorCode) const
{
    std::shared_ptr<HMUser> Result = nullptr;
    outErrorCode = make_error_code(eSystemErrorEx::seSuccess); // Изначально считаем что ошбки нет

    if (isEmpty())
        outErrorCode = make_error_code(eSystemErrorEx::seContainerEmpty);
    else
    {
        if (inContactIndex >= m_contacts.size())
            outErrorCode = make_error_code(eSystemErrorEx::seIndexOutOfContainerRange);
        else
        {
            auto It = m_contacts.begin();
            std::advance(It, inContactIndex);
            Result = *It;
        }
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::shared_ptr<HMUser> HMContactList::getContact(const QUuid inContactUuid, std::error_code& outErrorCode) const
{
    std::shared_ptr<HMUser> Result = nullptr;
    outErrorCode = make_error_code(eSystemErrorEx::seSuccess); // Изначально считаем что ошбки нет

    Result = std::make_shared<HMUser>(inContactUuid); // Формируем пользователя для поиска по UUID
    auto FindRes = m_contacts.find(Result);
    Result = nullptr; // Сбрасывам пользователя

    if (FindRes == m_contacts.end())
        outErrorCode = make_error_code(eSystemErrorEx::seNotInContainer);
    else
        Result = *FindRes;

    return Result;
}
//-----------------------------------------------------------------------------
std::error_code HMContactList::removeContact(const std::size_t inContactIndex)
{
    std::error_code Error = make_error_code(eSystemErrorEx::seSuccess); // Изначально считаем что ошбки нет

    if (inContactIndex >= m_contacts.size())
        Error = make_error_code(eSystemErrorEx::seIndexOutOfContainerRange);
    else
    {
        auto It = m_contacts.begin();
        std::advance(It, inContactIndex);
        m_contacts.erase(It);
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMContactList::removeContact(const QUuid inContactUuid)
{
    std::error_code Error = make_error_code(eSystemErrorEx::seSuccess); // Изначально считаем что ошбки нет

    std::shared_ptr<HMUser> TempFind = std::make_shared<HMUser>(inContactUuid); // Формируем пользователя для поиска по UUID
    auto FindRes = m_contacts.find(TempFind);

    if (FindRes == m_contacts.end())
        Error = make_error_code(eSystemErrorEx::seNotInContainer);
    else
        m_contacts.erase(FindRes);

    return Error;
}
//-----------------------------------------------------------------------------
