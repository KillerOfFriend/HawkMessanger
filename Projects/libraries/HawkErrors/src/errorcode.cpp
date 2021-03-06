#include "errorcode.h"

using namespace errors;

//-----------------------------------------------------------------------------
error_code::error_code(int inCode, const std::error_category& inErrCat) : std::error_code(inCode, inErrCat)
{

}
//-----------------------------------------------------------------------------
error_code::error_code(const std::error_code& inOtherEC) : std::error_code(inOtherEC)
{

}
//-----------------------------------------------------------------------------
error_code::error_code(const error_code& inOther) : std::error_code(inOther)
{

}
//-----------------------------------------------------------------------------
error_code& error_code::operator= (const std::error_code& inOther)
{
    if (&inOther != this)
        std::error_code::operator=(inOther);

    return *this;
}
//-----------------------------------------------------------------------------
error_code& error_code::operator= (const error_code& inOther)
{
    if (&inOther != this)
        std::error_code::operator=(inOther);

    return *this;
}
//-----------------------------------------------------------------------------
QString error_code::message_qstr() const
{
    return QString::fromStdString(message());
}
//-----------------------------------------------------------------------------
