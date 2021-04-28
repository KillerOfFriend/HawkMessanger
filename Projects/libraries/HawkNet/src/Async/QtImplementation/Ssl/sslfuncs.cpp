#include "sslfuncs.h"

#include <QFile>

#include <systemerrorex.h>
#include <neterrorcategory.h>

namespace net
{
//-----------------------------------------------------------------------------
QSslCertificate readCertificate(const std::filesystem::path& inCertificatePath, errors::error_code& outError)
{
    QSslCertificate Result;
    outError = make_error_code(errors::eNetError::neSuccess); // Изначально метим как успех

    if (!std::filesystem::exists(inCertificatePath, outError)) // Проверяем существование объекта
        outError = make_error_code(errors::eSystemErrorEx::seFileNotExists);
    else
    {
        if (!std::filesystem::is_regular_file(inCertificatePath, outError)) // Проверяем что это файл
            outError = make_error_code(errors::eSystemErrorEx::seObjectNotFile);
        else
        {
            QFile CertificateFile(inCertificatePath); // Инициализируем файл

            if (!CertificateFile.open(QIODevice::ReadOnly)) // Проверяем что файл успешно открылся на чтение
                outError = make_error_code(errors::eSystemErrorEx::seOpenFileFail);
            else
            {
                QList<QSslCertificate> CertificateList = QSslCertificate::fromDevice(&CertificateFile); // Читаем сертификаты из файла
                CertificateFile.close();

                if (CertificateList.isEmpty()) // Если не удалось считать ни одного сертификата
                    outError = make_error_code(errors::eSystemErrorEx::seReadFileFail);
                else
                    Result = CertificateList.first(); // Используем первый
            }
        }
    }

    return Result;
}
//-----------------------------------------------------------------------------
QSslKey readPrivateKey(const std::filesystem::path& inPrivateKeyPath, errors::error_code& outError)
{
    QSslKey Result;
    outError = make_error_code(errors::eNetError::neSuccess); // Изначально метим как успех

    if (!std::filesystem::exists(inPrivateKeyPath, outError)) // Проверяем существование объекта
        outError = make_error_code(errors::eSystemErrorEx::seFileNotExists);
    else
    {
        if (!std::filesystem::is_regular_file(inPrivateKeyPath, outError)) // Проверяем что это файл
            outError = make_error_code(errors::eSystemErrorEx::seObjectNotFile);
        else
        {
            QFile CertificateFile(inPrivateKeyPath); // Инициализируем файл

            if (!CertificateFile.open(QIODevice::ReadOnly)) // Проверяем что файл успешно открылся на чтение
                outError = make_error_code(errors::eSystemErrorEx::seOpenFileFail);
            else
            {
                Result = QSslKey(&CertificateFile, QSsl::KeyAlgorithm::Rsa, QSsl::Pem, QSsl::PrivateKey, "BaGet");
                CertificateFile.close();

                if (Result.isNull()) // Если не удалось считать ключ
                    outError = make_error_code(errors::eSystemErrorEx::seReadFileFail);
            }
        }
    }

    return Result;
}
//-----------------------------------------------------------------------------
QSslConfiguration makeSslConfiguration(const std::filesystem::path& inCertificatePath, const std::filesystem::path& inPrivateKeyPath, errors::error_code& outError)
{
    QSslConfiguration Result;
    outError = make_error_code(errors::eNetError::neSuccess); // Изначально метим как успех

    QSslCertificate Certificate = readCertificate(inCertificatePath, outError); // Читаем сертификат

    if (!outError) // Если сертификат успешно считан
    {
        QSslKey PrivateKeys = readPrivateKey(inPrivateKeyPath, outError); // Читаем приватный ключ

        if (!outError) // Если публичный ключ успешно считан
        {
            Result = QSslConfiguration::defaultConfiguration();
            // Формируем настройки Ssl
            Result.setProtocol(QSsl::AnyProtocol);
            //Result.setPeerVerifyMode(QSslSocket::QueryPeer);
            //Result.setSslOption(QSsl::SslOptionDisableServerNameIndication, true);
            Result.addCaCertificate(Certificate);
            Result.setLocalCertificate(Certificate);
            Result.setPrivateKey(PrivateKeys);
        }
    }

    return Result;
}
//-----------------------------------------------------------------------------
} // namespace net
