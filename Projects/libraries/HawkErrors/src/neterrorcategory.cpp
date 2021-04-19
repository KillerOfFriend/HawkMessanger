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
        case eNetError::neTimeOut:                          { Result = "Время ожидания истекло"; break; }
        case eNetError::neServerNotInit:                    { Result = "Сервер не инициализирован"; break; }
        case eNetError::neSocketNotInit:                    { Result = "Сокет не инициализирован"; break; }
        case eNetError::neNotConnected:                     { Result = "Соединение не установлено"; break; }
        case eNetError::neClientNotFound:                   { Result = "Указанный клиент не найден"; break; }
        case eNetError::neClientIdAlredyExists:             { Result = "Клиент с таким ID уже существует"; break; }
        case eNetError::neCreateConnectionFail:             { Result = "Не удалось установить соединение"; break; }

        // Qt Implementation

        case eNetError::neUnknownQtSocketError:             { Result = "Незвестная ошибка QtSocket"; break; }
        case eNetError::neStartListenFail:                  { Result = "Не удалось начать прослушивание порта"; break; }
        // QtSocket errors
        case eNetError::neConnectionRefusedError:           { Result = "ConnectionRefused"; break; }
        case eNetError::neRemoteHostClosedError:            { Result = "RemoteHostClosed"; break; }
        case eNetError::neHostNotFoundError:                { Result = "HostNotFound"; break; }
        case eNetError::neSocketAccessError:                { Result = "SocketAccessError"; break; }
        case eNetError::neSocketResourceError:              { Result = "SocketResource"; break; }
        case eNetError::neSocketTimeoutError:               { Result = "SocketTimeout"; break; }
        case eNetError::neDatagramTooLargeError:            { Result = "DatagramTooLarge"; break; }
        case eNetError::neNetworkError:                     { Result = "NetworkError"; break; }
        case eNetError::neAddressInUseError:                { Result = "AddressInUse"; break; }
        case eNetError::neSocketAddressNotAvailableError:   { Result = "SocketAddressNotAvailable"; break; }
        case eNetError::neUnsupportedSocketOperationError:  { Result = "UnsupportedSocketOperation"; break; }
        case eNetError::neUnfinishedSocketOperationError:   { Result = "UnfinishedSocketOperation"; break; }
        case eNetError::neProxyAuthenticationRequiredError: { Result = "ProxyAuthenticationRequired"; break; }
        case eNetError::neSslHandshakeFailedError:          { Result = "SslHandshakeFailed"; break; }
        case eNetError::neProxyConnectionRefusedError:      { Result = "ProxyConnectionRefused"; break; }
        case eNetError::neProxyConnectionClosedError:       { Result = "ProxyConnectionClosed"; break; }
        case eNetError::neProxyConnectionTimeoutError:      { Result = "ProxyConnectionTimeout"; break; }
        case eNetError::neProxyNotFoundError:               { Result = "ProxyNotFound"; break; }
        case eNetError::neProxyProtocolError:               { Result = "ProxyProtocol"; break; }
        case eNetError::neOperationError:                   { Result = "OperationError"; break; }
        case eNetError::neSslInternalError:                 { Result = "SslInternalError"; break; }
        case eNetError::neSslInvalidUserDataError:          { Result = "SslInvalidUserData"; break; }
        case eNetError::neTemporaryError:                   { Result = "TemporaryError"; break; }

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
