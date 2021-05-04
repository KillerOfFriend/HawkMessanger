#include "qinteractive.h"

#include <iostream>

#include <HawkLog.h>

QInteractive::QInteractive(QObject *parent) : QObject(parent)
{

}

QInteractive::~QInteractive()
{
    slot_stopInteractive();
}

void QInteractive::slot_startInteractive()
{
    m_continue = true;

    do
    {
        std::string Message;
        std::getline(std::cin, Message);

        if (!Message.empty())
        {
            if (Message == "/q")
                m_continue = false;
            else
            {
                errors::error_code Error = sig_sendMessage(Message);

                if (Error)
                    LOG_ERROR(Error.message_qstr());
            }
        }
    }
    while (m_continue);
}

void QInteractive::slot_stopInteractive()
{
    m_continue = false;
}
