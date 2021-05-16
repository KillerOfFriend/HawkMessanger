#ifndef HMVERSION_@PROJECT_NAME@_H
#define HMVERSION_@PROJECT_NAME@_H

#include <string>
#include <cstdint>

namespace @PROJECT_NAME@
{
//-----------------------------------------------------------------------------
/**
 * @brief getVersion - Функция вернёт текстовое представление версии модуля\библиотеки
 * @return Вернёт текстовое представление версии проекта
 */
std::string getVersion();
//-----------------------------------------------------------------------------
/**
 * @brief getVerMajor - Функция вернёт номер мажора версии модуля\библиотеки
 * @return Вернёт номер мажора версии проекта
 */
std::uint8_t getVerMajor();
//-----------------------------------------------------------------------------
/**
 * @brief getVerMinor - Функция вернёт номер минора версии модуля\библиотеки
 * @return Вернёт номер минора версии проекта
 */
std::uint8_t getVerMinor();
//-----------------------------------------------------------------------------
/**
 * @brief getVerPatch - Функция вернёт номер патча версии модуля\библиотеки
 * @return Вернёт номер патча версии проекта
 */
std::uint8_t getVerPatch();
//-----------------------------------------------------------------------------
/**
 * @brief getVerTweak - Функция вернёт номер твика версии модуля\библиотеки
 * @return Вернёт номер твика версии проекта
 */
std::uint8_t getVerTweak();
//-----------------------------------------------------------------------------
/**
 * @brief getModuleName - Функция вернёт имя модуля\библиотеки
 * @return Вернёт имя модуля
 */
std::string getModuleName();
//-----------------------------------------------------------------------------
} // namespace @PROJECT_NAME@

#endif // HMVERSION_@PROJECT_NAME@_H
