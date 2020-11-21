#include "customstreams.h"

#include <qsystemdetection.h>

//-----------------------------------------------------------------------------
QTextStream& hmlog::QStdOut()
{
    static QTextStream TextStream( stdout, QIODeviceBase::WriteOnly );
    return TextStream;
}
//-----------------------------------------------------------------------------
#if defined(Q_OS_LINUX)

    //-----------------------------------------------------------------------------
    QTextStream& hmlog::reset(QTextStream& inStream)
    {
        inStream << "\x1B[0m";
        return inStream;
    }
    //-----------------------------------------------------------------------------
    QTextStream& hmlog::green(QTextStream& inStream)
    {
        inStream << "\x1B[32m";
        return inStream;
    }
    //-----------------------------------------------------------------------------
    QTextStream& hmlog::blue(QTextStream& inStream)
    {
        inStream << "\x1B[34m";
        return inStream;
    }
    //-----------------------------------------------------------------------------
    QTextStream& hmlog::yellow(QTextStream& inStream)
    {
        inStream << "\x1B[33m";
        return inStream;
    }
    //-----------------------------------------------------------------------------
    QTextStream& hmlog::red(QTextStream& inStream)
    {
        inStream << "\x1B[31m";
        return inStream;
    }
    //-----------------------------------------------------------------------------

#elif defined(Q_OS_WIN)

    #include <windows.h>

    //-----------------------------------------------------------------------------
    QTextStream& hmlog::reset(QTextStream& inStream)
    {
        inStream.flush();
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        return inStream;
    }
    //-----------------------------------------------------------------------------
    QTextStream& hmlog::green(QTextStream& inStream)
    {
        inStream.flush();
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
        return inStream;
    }
    //-----------------------------------------------------------------------------
    QTextStream& hmlog::blue(QTextStream& inStream)
    {
        inStream.flush();
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN);
        return inStream;
    }
    //-----------------------------------------------------------------------------
    QTextStream& hmlog::yellow(QTextStream& inStream)
    {
        inStream.flush();
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_RED);
        return inStream;
    }
    //-----------------------------------------------------------------------------
    QTextStream& hmlog::red(QTextStream& inStream)
    {
        inStream.flush();
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
        return inStream;
    }
    //-----------------------------------------------------------------------------

#endif
//-----------------------------------------------------------------------------
