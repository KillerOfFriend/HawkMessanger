#ifndef QTCLIENTTHREAD_H
#define QTCLIENTTHREAD_H

#include <iostream>

#include <QThread>

#include "prototypeclient.h"

//-----------------------------------------------------------------------------
class QtClientThread : public QThread
{
    Q_OBJECT
public:

    QtClientThread() = default;

    virtual ~QtClientThread() override = default;

protected:

    void run() override;

private:

    PrototypeClient m_PClient;

    bool initClient();

    void clientProcess();
};

//-----------------------------------------------------------------------------


#endif // QTCLIENTTHREAD_H
