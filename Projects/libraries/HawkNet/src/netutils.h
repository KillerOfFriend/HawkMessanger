#ifndef HMNETUTILS_H
#define HMNETUTILS_H

#include <nlohmann/json.hpp>

#include "nettypes.h"

namespace net
{
//-----------------------------------------------------------------------------
///**
// * @brief strToBinaryJson - Функция преобразует строку в бинарную последовательность Json
// * @param inStr - Преобразуемая строка
// * @return Вернёт бинарную последовательность Json
// */
//[[nodiscard]] nlohmann::json strToBinaryJson(const std::string& inStr);
////-----------------------------------------------------------------------------
///**
// * @brief binaryJsonToStr - Функция преобразует бинарную последовательность Json в строку
// * @param inJson - Бинарная последовательность Json
// * @return Вернё строку
// */
//[[nodiscard]] std::string binaryJsonToStr(const nlohmann::json& inJson);
//-----------------------------------------------------------------------------
/**
 * @brief getWrapAndMessageData - Иетод из
 * @param inTextData - "Обёрнутые" данные в текстовом представлении
 * @param outWrapInfo - Информация об обёртке
 * @param outMessageData - "Обёрнутое" сообщение
 * @return Вернёт признак успеха операции
 */
[[nodiscard]] bool getWrapAndMessageData(const std::string& inTextData, nlohmann::json& outWrapInfo, std::string& outMessageData);
//-----------------------------------------------------------------------------
/**
 * @brief wrap - Функция "обернёт" отправляемые данные в безопастный для отправки вид
 * @param inData - Отправляемые данные
 * @return Вернёт подготовленные к отправке данные
 */
[[nodiscard]] oByteStream wrap(oByteStream&& inData);
//-----------------------------------------------------------------------------
/**
 * @brief unwrap - Функция "развернёт" принятые обёрнутые данные
 * @param inData - Принятые данные
 * @return Вернёт распакованные данные
 */
[[nodiscard]] iByteStream unwrap(iByteStream&& inData);
//-----------------------------------------------------------------------------
} // namespace net

#endif // HMNETUTILS_H
