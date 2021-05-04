#ifndef QINTERACTIVE_H
#define QINTERACTIVE_H

#include <QObject>

#include <errorcode.h>

class QInteractive : public QObject
{
    Q_OBJECT
public:
    explicit QInteractive(QObject *parent = nullptr);
    virtual ~QInteractive() override;

public slots:

    void slot_startInteractive();

    void slot_stopInteractive();

signals:

    errors::error_code sig_sendMessage(std::string inStrData);

private:
    bool m_continue = true;
};

#endif // QINTERACTIVE_H
