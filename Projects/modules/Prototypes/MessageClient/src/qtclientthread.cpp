#include "qtclientthread.h"

#include <HawkLog.h>

//-----------------------------------------------------------------------------
void QtClientThread::run()
{
    if (initClient())
        clientProcess();
}
//-----------------------------------------------------------------------------
bool QtClientThread::initClient()
{
    errors::error_code Error = m_PClient.connect("127.0.0.1", 32112);

    if (Error)
        LOG_ERROR(Error.message_qstr());

    return (!Error);
}
//-----------------------------------------------------------------------------
void QtClientThread::clientProcess()
{
    bool Continue = true;

    do
    {
        std::cout << "Print your message: ";
        std::string Message;
        std::getline(std::cin, Message);

        if (!Message.empty())
        {
            if (Message == "/q")
                Continue = false;
            else
            {
                net::oByteStream Data(Message);
                errors::error_code Error = m_PClient.send(std::move(Data));

                if (Error)
                    LOG_ERROR(Error.message_qstr());
            }
        }
    }
    while (Continue);
}
//-----------------------------------------------------------------------------
