#ifndef HMQTSSLASYNCSERVER_H
#define HMQTSSLASYNCSERVER_H

#include <QObject>
#include <QSslConfiguration>

#include "qsslserver.h"
#include "Async/QtImplementation/Abstract/qtabstractasyncserver.h"
// filesystem подключаем после Qt иначе возникнет конфликт (официальный баг)
#include <filesystem>

namespace net
{
//-----------------------------------------------------------------------------
/**
 * @brief The CertificatePaths struct - Структура, хранящая пути к файлам сертификата
 */
struct CertificatePaths
{
    std::filesystem::path m_certificatePath;    ///< Путь к файлу сертификату
    std::filesystem::path m_privateKey;         ///< Путь к приватному ключу
};
//-----------------------------------------------------------------------------
/**
 * @brief The HMQtSslAsyncServer class - Абстракция, описывающая асинхронный TCP сервер, использующий SSL
 *
 * @authors Alekseev_s
 * @date 19.04.2021
 */
class HMQtSslAsyncServer : public HMQtAbstractAsyncServer
{
    Q_OBJECT
public:

    /**
     * @brief HMQtSslAsyncServer - Инициализирующий конструктор
     */
    HMQtSslAsyncServer(const std::uint16_t inPort, const CertificatePaths& inCerPaths, const ServCallbacks& inCallbacks);

    /**
     * @brief ~HMQtSslAsyncServer - Виртуальный деструктор по умолчанию
     */
    virtual ~HMQtSslAsyncServer() override;

    /**
     * @brief start - Метод запустит сервер
     * @return Вернёт признак ошибки
     */
    virtual errors::error_code start() override;

protected:

    /**
     * @brief makeServer - Метод, формирующий экземпляр Qt сервера
     * @param outError - Признак ошибки
     * @return Венёт указатель на экземпляр сервера или nullptr
     */
    virtual std::unique_ptr<QTcpServer> makeServer(errors::error_code& outError) override;

    /**
     * @brief makeConnection - Метод сформирует соединение
     * @param inSocket - Сокет соединения
     * @return Вернёт указатель на новое соединение или nullptr
     */
    virtual std::unique_ptr<HMQtAbstractAsyncConnection> makeConnection(std::unique_ptr<QTcpSocket>&& inSocket) override;

private:

    CertificatePaths m_certificatePaths;    ///< Пути к файлам сертификата
    QSslConfiguration m_sslConfig;          ///< Настройки SSL

};
//-----------------------------------------------------------------------------
} // namespace net

#endif // HMQTSSLASYNCSERVER_H
