#ifndef QSSLSERVER_H
#define QSSLSERVER_H

// https://github.com/GuiTeK/Qt-SslServer
// https://github.com/Skycoder42/QSslServer

#include <QTcpServer>
#include <QSslConfiguration>

//-----------------------------------------------------------------------------
/**
 * @brief The QSslServer class - Класс, описывающий реализация SSL TCP сервера
 *
 * @authors Alekseev_s
 * @date 19.04.2021
 */
class QSslServer : public QTcpServer
{
    Q_OBJECT
public:

    /**
     * @brief QSslServer - Инициализирующий конструктор
     * @param inSslConfig - Конфигурации SSL
     * @param parent - Указатель на объект-предок
     */
    QSslServer(QSslConfiguration& inSslConfig, QObject *parent = nullptr);

    /**
     * @brief ~QSslServer - Виртуальный деструктор по умолчанию
     */
    virtual ~QSslServer() override = default;

protected:

    /**
     * @brief incomingConnection - Метод, инициализирующий сокет нового входящего соединения
     * @param handle - Дискриптер сокета
     */
    virtual void incomingConnection(qintptr handle) override;

private:

    QSslConfiguration m_sslConfig; ///< Конфигурации SSL

};
//-----------------------------------------------------------------------------

#endif // QSSLSERVER_H
