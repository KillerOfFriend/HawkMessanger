#ifndef HMNETUTILS_H
#define HMNETUTILS_H

#include "nettypes.h"

namespace net
{
//-----------------------------------------------------------------------------
/**
 * @brief wrap - Функция "обернёт" отправляемые данные в безопастный для отправки вид
 * @param inData - Отправляемые данные
 * @return Вернёт подготовленные к отправке данные
 */
oByteStream wrap(oByteStream&& inData);
//-----------------------------------------------------------------------------
/**
 * @brief unwrap - Функция "развернёт" принятые обёрнутые данные
 * @param inData - Принятые данные
 * @return Вернёт распакованные данные
 */
iByteStream unwrap(iByteStream&& inData);
//-----------------------------------------------------------------------------
} // namespace net

#endif // HMNETUTILS_H
