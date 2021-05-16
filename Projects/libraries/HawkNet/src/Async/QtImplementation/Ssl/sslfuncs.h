#ifndef HMSSLFUNCS_H
#define HMSSLFUNCS_H

/**
 * @file sslfuncs.h
 * @brief Содержит функции для работы с сертификатами и ключами SSL (Qt)
 */

#include <QSslKey>
#include <QSslCertificate>
#include <QSslConfiguration>

#include <errorcode.h>

// filesystem подключаем после Qt иначе возникнет конфликт (официальный баг)
#include <filesystem>

namespace net
{
//-----------------------------------------------------------------------------s
/**
 * @brief readCertificate - Функция считает сертификат из файла
 * @param inCertificatePath - Путь к файлу сертификата
 * @param outError - Признак ошибки
 * @return Вернёт считанный сертификат
 */
QSslCertificate readCertificate(const std::filesystem::path& inCertificatePath, errors::error_code& outError);
//-----------------------------------------------------------------------------
/**
 * @brief readPrivateKey - Функция считает приватный ключ из файла
 * @param inPrivateKeyPath - Путь к файлу приватного ключа
 * @param outError - Признак ошибки
 * @return Вернёт считанный приватный ключ
 */
QSslKey readPrivateKey(const std::filesystem::path& inPrivateKeyPath, errors::error_code& outError);
//-----------------------------------------------------------------------------
/**
 * @brief makeSslConfiguration - Функция считает и подготовит настройки SSL
 * @param inCertificatePath - Путь к файлу сертификата
 * @param inPrivateKeyPath - Путь к файлу приватного ключа
 * @param outError - Признак ошибки
 * @return Вернёт настройки SSL
 */
QSslConfiguration makeSslConfiguration(const std::filesystem::path& inCertificatePath, const std::filesystem::path& inPrivateKeyPath, errors::error_code& outError);
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
} // namespace net

#endif // HMSSLFUNCS_H
