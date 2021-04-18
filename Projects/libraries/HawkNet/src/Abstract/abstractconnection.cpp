#include "abstractconnection.h"

using namespace net;

//-----------------------------------------------------------------------------
HMAbstractConnection::HMAbstractConnection() : HMConnection()
{
    std::atomic_init(&m_id, 0);
    // При инициализации зададим ГАРАНТИРОВАННО уникальный идентификатор
    setID(reinterpret_cast<std::uintptr_t>(this));
}
//-----------------------------------------------------------------------------
void HMAbstractConnection::setID(const std::size_t inID)
{ m_id = inID; }
//-----------------------------------------------------------------------------
std::size_t HMAbstractConnection::getID() const
{ return m_id; }
//-----------------------------------------------------------------------------
