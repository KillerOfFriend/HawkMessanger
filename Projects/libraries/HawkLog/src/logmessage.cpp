#include "logmessage.h"

using namespace hmlog;

//-----------------------------------------------------------------------------
HMLogMessage::HMLogMessage(const eLogMessageType inType, const QString& inMessage) :
    m_time(QDateTime::currentDateTime()),
    m_type(inType),
    m_message(inMessage)
{

}
//-----------------------------------------------------------------------------
