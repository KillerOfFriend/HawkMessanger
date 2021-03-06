#include "groupmessage.h"

#include <systemerrorex.h>

using namespace hmcommon;

//-----------------------------------------------------------------------------
// MsgData
//-----------------------------------------------------------------------------
MsgData::MsgData(const eMsgType& inType, const QByteArray& inData) :
    m_type(inType),
    m_data(inData)
{

}
//-----------------------------------------------------------------------------
MsgData::MsgData(const MsgData& inOther) :
    m_type(inOther.m_type),
    m_data(inOther.m_data)
{

}
//-----------------------------------------------------------------------------
MsgData& MsgData::operator=(const MsgData& inOther)
{
    if (this != &inOther)
    {
        this->m_type = inOther.m_type;
        this->m_data = inOther.m_data;
    }

    return *this;
}
//-----------------------------------------------------------------------------
// MsgRange
//-----------------------------------------------------------------------------
MsgRange::MsgRange(const QDateTime& inFrom, const QDateTime& inTo) :
    m_from(inFrom),
    m_to(inTo)
{

}
//-----------------------------------------------------------------------------
// HMGroupInfoMessage
//-----------------------------------------------------------------------------
HMGroupInfoMessage::HMGroupInfoMessage(const QUuid& inUuid, const QUuid& inGroupUuid, const QDateTime& inCreateTime) :
    m_message(MsgData(eMsgType::mtEmpty, QByteArray())),
    m_uuid(inUuid),
    m_group(inGroupUuid),
    m_createTime(inCreateTime)
{

}
//-----------------------------------------------------------------------------
errors::error_code HMGroupInfoMessage::setMessage(const MsgData& inMessageData)
{
    errors::error_code Error = make_error_code(errors::eSystemErrorEx::seSuccess);


    if (inMessageData.m_type == eMsgType::mtEmpty)
        Error = make_error_code(errors::eSystemErrorEx::seIncorretData);
    else
        m_message = inMessageData;

    return Error;
}
//-----------------------------------------------------------------------------
MsgData HMGroupInfoMessage::getMesssage() const
{ return m_message; }
//-----------------------------------------------------------------------------
