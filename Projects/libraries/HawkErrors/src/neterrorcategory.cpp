#include "neterrorcategory.h"

using namespace errors;

//-----------------------------------------------------------------------------
NetErrorCategory::NetErrorCategory() : std::error_category()
{

}
//-----------------------------------------------------------------------------
const char* NetErrorCategory::name() const noexcept
{
    return C_CATEGORY_NET_NAME.c_str();
}
//-----------------------------------------------------------------------------
std::string NetErrorCategory::message(int inCode) const
{
    std::string Result;

    switch (static_cast<errors::eNetError>(inCode))
    {
        case eNetError::neIgnored:                              { Result = "Ошибка проигнорирована системой"; break; }
        case eNetError::neTimeOut:                              { Result = "Время ожидания истекло"; break; }
        case eNetError::neServerNotInit:                        { Result = "Сервер не инициализирован"; break; }
        case eNetError::neSocketNotInit:                        { Result = "Сокет не инициализирован"; break; }
        case eNetError::neNotConnected:                         { Result = "Соединение не установлено"; break; }
        case eNetError::neClientNotFound:                       { Result = "Указанный клиент не найден"; break; }
        case eNetError::neClientIdAlredyExists:                 { Result = "Клиент с таким ID уже существует"; break; }
        case eNetError::neCreateConnectionFail:                 { Result = "Не удалось установить соединение"; break; }

        // Qt Implementation

        case eNetError::neUnknownQtSocketError:                 { Result = "Незвестная ошибка QtSocket"; break; }
        case eNetError::neStartListenFail:                      { Result = "Не удалось начать прослушивание порта"; break; }
        // QtSocket errors
        case eNetError::neConnectionRefusedError:               { Result = "ConnectionRefused"; break; }
        case eNetError::neRemoteHostClosedError:                { Result = "RemoteHostClosed"; break; }
        case eNetError::neHostNotFoundError:                    { Result = "HostNotFound"; break; }
        case eNetError::neSocketAccessError:                    { Result = "SocketAccessError"; break; }
        case eNetError::neSocketResourceError:                  { Result = "SocketResource"; break; }
        case eNetError::neSocketTimeoutError:                   { Result = "SocketTimeout"; break; }
        case eNetError::neDatagramTooLargeError:                { Result = "DatagramTooLarge"; break; }
        case eNetError::neNetworkError:                         { Result = "NetworkError"; break; }
        case eNetError::neAddressInUseError:                    { Result = "AddressInUse"; break; }
        case eNetError::neSocketAddressNotAvailableError:       { Result = "SocketAddressNotAvailable"; break; }
        case eNetError::neUnsupportedSocketOperationError:      { Result = "UnsupportedSocketOperation"; break; }
        case eNetError::neUnfinishedSocketOperationError:       { Result = "UnfinishedSocketOperation"; break; }
        case eNetError::neProxyAuthenticationRequiredError:     { Result = "ProxyAuthenticationRequired"; break; }
        case eNetError::neSslHandshakeFailedError:              { Result = "SslHandshakeFailed"; break; }
        case eNetError::neProxyConnectionRefusedError:          { Result = "ProxyConnectionRefused"; break; }
        case eNetError::neProxyConnectionClosedError:           { Result = "ProxyConnectionClosed"; break; }
        case eNetError::neProxyConnectionTimeoutError:          { Result = "ProxyConnectionTimeout"; break; }
        case eNetError::neProxyNotFoundError:                   { Result = "ProxyNotFound"; break; }
        case eNetError::neProxyProtocolError:                   { Result = "ProxyProtocol"; break; }
        case eNetError::neOperationError:                       { Result = "OperationError"; break; }
        case eNetError::neSslInternalError:                     { Result = "SslInternalError"; break; }
        case eNetError::neSslInvalidUserDataError:              { Result = "SslInvalidUserData"; break; }
        case eNetError::neTemporaryError:                       { Result = "TemporaryError"; break; }
        // QSslSoket errors
        case eNetError::neUnableToGetIssuerCertificate:         { Result = "The issuer certificate could not be found"; break; }
        case eNetError::neUnableToDecryptCertificateSignature:  { Result = "The certificate signature could not be decrypted"; break; }
        case eNetError::neUnableToDecodeIssuerPublicKey:        { Result = "The public key in the certificate could not be read"; break; }
        case eNetError::neCertificateSignatureFailed:           { Result = "The signature of the certificate is invalid"; break; }
        case eNetError::neCertificateNotYetValid:               { Result = "The certificate is not yet valid"; break; }
        case eNetError::neCertificateExpired:                   { Result = "The certificate has expired"; break; }
        case eNetError::neInvalidNotBeforeField:                { Result = "The certificate's notBefore field contains an invalid time"; break; }
        case eNetError::neInvalidNotAfterField:                 { Result = "The certificate's notAfter field contains an invalid time"; break; }
        case eNetError::neSelfSignedCertificate:                { Result = "The certificate is self-signed, and untrusted"; break; }
        case eNetError::neSelfSignedCertificateInChain:         { Result = "The root certificate of the certificate chain is self-signed, and untrusted"; break; }
        case eNetError::neUnableToGetLocalIssuerCertificate:    { Result = "The issuer certificate of a locally looked up certificate could not be found"; break; }
        case eNetError::neUnableToVerifyFirstCertificate:       { Result = "No certificates could be verified"; break; }
        case eNetError::neCertificateRevoked:                   { Result = "???"; break; }
        case eNetError::neInvalidCaCertificate:                 { Result = "One of the CA certificates is invalid"; break; }
        case eNetError::nePathLengthExceeded:                   { Result = "The basicConstraints path length parameter has been exceeded"; break; }
        case eNetError::neInvalidPurpose:                       { Result = "The root CA certificate is not trusted for this purpose"; break; }
        case eNetError::neCertificateUntrusted:                 { Result = ""; break; }
        case eNetError::neCertificateRejected:                  { Result = "The root CA certificate is marked to reject the specified purpose"; break; }
        case eNetError::neAuthorityIssuerSerialNumberMismatch:  { Result = "The current candidate issuer certificate was rejected because"
                                                                           " its issuer name and serial number was present and did not match the"
                                                                           " authority key identifier of the current certificate"; break; }
        case eNetError::neNoPeerCertificate:                    { Result = "The peer did not present any certificate"; break; }
        case eNetError::neHostNameMismatch:                     { Result = "The host name did not match any of the valid hosts for this certificate"; break; }
        case eNetError::neNoSslSupport:                         { Result = "No SSL support"; break; }
        case eNetError::neCertificateBlacklisted:               { Result = "The peer certificate is blacklisted"; break; }
        case eNetError::neCertificateStatusUnknown:             { Result = "???"; break; }
        case eNetError::neOcspNoResponseFound:                  { Result = "No OCSP status response found"; break; }
        case eNetError::neOcspMalformedRequest:                 { Result = "The OCSP status request had invalid syntax"; break; }
        case eNetError::neOcspMalformedResponse:                { Result = "OCSP response contains an unexpected number of SingleResponse structures"; break; }
        case eNetError::neOcspInternalError:                    { Result = "OCSP responder reached an inconsistent internal state"; break; }
        case eNetError::neOcspTryLater:                         { Result = "OCSP responder was unable to return a status for the requested certificate"; break; }
        case eNetError::neOcspSigRequred:                       { Result = "The server requires the client to sign the OCSP request in order to construct a response"; break; }
        case eNetError::neOcspUnauthorized:                     { Result = "The client is not authorized to request OCSP status from this server"; break; }
        case eNetError::neOcspResponseCannotBeTrusted:          { Result = "OCSP responder's identity cannot be verified"; break; }
        case eNetError::neOcspResponseCertIdUnknown:            { Result = "The identity of a certificate in an OCSP response cannot be established"; break; }
        case eNetError::neOcspResponseExpired:                  { Result = "The certificate status response has expired"; break; }
        case eNetError::neOcspStatusUnknown:                    { Result = "The certificate's status is unknown"; break; }

        default: Result = C_ERROR_UNKNOWN_TEXT + std::to_string(inCode);
    }

    return Result;
}
//-----------------------------------------------------------------------------
// OPTIONAL: Allow generic error conditions to be compared to me
//    std::error_condition NetErrorCategory::default_error_condition(int c) const noexcept
//    {
//        switch (static_cast<SubscriptionError>(c))
//        {
//        case SubscriptionError::EmptyString:
//            return make_error_condition(std::errc::invalid_argument);
//        case SubscriptionError::IllegalChar:
//            return make_error_condition(std::errc::invalid_argument);
//        case SubscriptionError::TooLong:
//            return make_error_condition(std::errc::result_out_of_range);
//        default:
//            // I have no mapping for this code
//            return std::error_condition(c, *this);
//        }
//    }
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
extern inline const errors::NetErrorCategory &errors::ConversionNetError_category()
{
  static errors::NetErrorCategory category;
  return category;
}
//-----------------------------------------------------------------------------
errors::error_code make_error_code(errors::eNetError inErrCode)
{
  return { static_cast<int>(inErrCode), errors::ConversionNetError_category() };
}
//-----------------------------------------------------------------------------
