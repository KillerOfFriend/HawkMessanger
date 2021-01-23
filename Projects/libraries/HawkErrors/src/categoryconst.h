#ifndef CATEGORYCONST_H
#define CATEGORYCONST_H

#include <cstdint>
#include <string>

namespace errors
{
//-----------------------------------------------------------------------------
static const std::string C_ERROR_SUCESS_TEXT                = "Операция выполнена успешно";     ///< Сообщение об успешном завершении операции
static const std::string C_ERROR_UNKNOWN_TEXT               = "Не известная ошибка с кодом: ";  ///< Сообщение о неизвестном коде ошибки
//-----------------------------------------------------------------------------
static const std::int32_t C_SUCCESS                         = 0x0;          ///< Код успеха. Общий для всех категорий
static const std::int32_t C_CATEGORY_SYSTEMERROREX_START    = 0x00001000;   ///< Начальное значение расширеных ошибок
static const std::int32_t C_CATEGORY_DATASTORAGE_START      = 0x00002000;   ///< Начальное значение ошибок хранилища данных
//-----------------------------------------------------------------------------
static const std::string C_CATEGORY_SYSTEMERROREX_NAME      = "Расширеные системные ошибки";
static const std::string C_CATEGORY_DATASTORAGE_NAME        = "Ошибки хранилища данных";
//-----------------------------------------------------------------------------
} // namespace errors

#endif // CATEGORYCONST_H
