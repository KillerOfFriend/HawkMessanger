#ifndef NETERRORCATEGORY_H
#define NETERRORCATEGORY_H

/**
 * @file neterrorcategoty.h
 * @brief Содержит описание категории сетевых ошибок
 */

#include <string>

#include "errorcode.h"
#include "categoryconst.h"

namespace errors
{
//-----------------------------------------------------------------------------
/**
 * @brief The eSystemErrorEx enum - Перечень расширеных системных ошибок
 */
enum class eNetError
{
    neSuccess = C_SUCCESS,                          ///< 0 Не явялется ошибкой

    neTimeOut = C_CATEGORY_NET_START,               ///< Время ожидания истекло
    neNotConnected,                                 ///< Соединение не установлено
    neClientNotFound,                               ///< Указанный клиент не найден
    neClientIdAlredyExists,                         ///< Клиент с таким ID уже существует
    neCreateConnectionFail,                         ///< Не удалось установить соединение

    // Qt Implementation
    neUnknownQtSocketError,                         ///< Неизвестная ошибка QtSocket
    neStartListenFail,                              ///< Не удалось начать прослушивание порта
    // QtSocket errors
    neConnectionRefusedError,
    neRemoteHostClosedError,
    neHostNotFoundError,
    neSocketAccessError,
    neSocketResourceError,
    neSocketTimeoutError,
    neDatagramTooLargeError,
    neNetworkError,
    neAddressInUseError,
    neSocketAddressNotAvailableError,
    neUnsupportedSocketOperationError,
    neUnfinishedSocketOperationError,
    neProxyAuthenticationRequiredError,
    neSslHandshakeFailedError,
    neProxyConnectionRefusedError,
    neProxyConnectionClosedError,
    neProxyConnectionTimeoutError,
    neProxyNotFoundError,
    neProxyProtocolError,
    neOperationError,
    neSslInternalError,
    neSslInvalidUserDataError,
    neTemporaryError,

    neCount
};
//---------------------------------------------------------------------------
/**
 * @brief The NetErrorCategory class - Класс, сописывающий категорию сетевых ошибок
 *
 * @authors Alekseev_s
 * @date 27.01.2021
 */
class NetErrorCategory : public std::error_category
{
public:

    /**
     * @brief NetErrorCategory - Конструктор по умолчанию
     */
    NetErrorCategory();

    /**
     * @brief ~NetErrorCategory - Виртуальный деструктор по умолчанию
     */
    virtual ~NetErrorCategory() override = default;

    /**
     * @brief name - Метод вернёт имя категории
     * @return Вернёт имя категории
     */
    virtual const char *name() const noexcept override final;

    /**
     * @brief message - Метод вернёт сообщение ошибки
     * @param inCode - Код ошибки
     * @return Вернёт сообщение ошибки
     */
    virtual std::string message(int inCode) const override final;
};
//-----------------------------------------------------------------------------
/**
 * @brief ConversionNetError_category - Функцию, возвращающую статический экземпляр пользовательской категории
 * @return Вернёт статический экземпляр пользовательской категории
 */
extern inline const NetErrorCategory &ConversionNetError_category();
//-----------------------------------------------------------------------------
} // namespace errors
//-----------------------------------------------------------------------------
namespace std
{
// Сообщаем метапрограммированию C++ 11 STL, что enum
// зарегистрирован в стандартной системе кодов ошибок
template <> struct is_error_code_enum<errors::eNetError> : true_type
{};
}
//-----------------------------------------------------------------------------
/**
 * @brief make_error_code - Перегрузка глобальной функции make_error_code () нашем пользовательским перечислением.
 * @param inErrCode - Код ошибки
 * @return Вернёт сформированный экемпляр errors::error_code
 */
errors::error_code make_error_code(errors::eNetError inErrCode);
//-----------------------------------------------------------------------------

#endif // NETERRORCATEGORY_H
