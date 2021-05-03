#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <limits>

#include <HawkLog.h>

//-----------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->portSpinBox->setMinimum(5000);
    ui->portSpinBox->setMaximum(std::numeric_limits<std::uint16_t>::max());

    QObject::connect(ui->ChatListWidget->model(), SIGNAL(rowsInserted(QModelIndex,int,int)), ui->ChatListWidget, SLOT(scrollToBottom()));
    QObject::connect(&m_client, &PrototypeClient::sig_textReceived, this, &MainWindow::on_Text);
}
//-----------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    m_client.disconnect();
    delete ui;
}
//-----------------------------------------------------------------------------
void MainWindow::on_Text(QString& inText)
{
    ui->ChatListWidget->addItem(inText);
}
//-----------------------------------------------------------------------------
void MainWindow::on_btnConnect_clicked()
{
    auto Host = ui->hostLineEdit->text().toStdString();
    errors::error_code Error = m_client.connect(Host, ui->portSpinBox->value());

    QString Str = "";

    if (Error)
    {
        Str = Error.message_qstr();
        LOG_ERROR(Str);
    }
    else
    {
        Str = "Connected";
        LOG_INFO(Str);
    }

    on_Text(Str);
}
//-----------------------------------------------------------------------------
void MainWindow::on_btnDisconnect_clicked()
{
    m_client.disconnect();
}
//-----------------------------------------------------------------------------
void MainWindow::on_btnSend_clicked()
{
    std::string Text = ui->lineEdit->text().toStdString();

    if (Text.empty())
        return;

    errors::error_code Error = m_client.send(net::oByteStream(Text));

    if (Error)
    {
        QString ErrText = Error.message_qstr();
        LOG_ERROR(ErrText);
        on_Text(ErrText);
    }
    else
        ui->lineEdit->clear();
}
//-----------------------------------------------------------------------------
