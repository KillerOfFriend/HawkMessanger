#ifndef SYSTEMERROREX_H
#define SYSTEMERROREX_H

/**
 * @file systemerrorex.h
 * @brief Содержит описание категории расширеных системных ошибок
 */

#include <string>
#include <system_error>

#include <QObject>

namespace hmcommon
{
//-----------------------------------------------------------------------------
/**
 * @brief The SystemErrorEx enum - Перечень расширеных системных ошибок
 */
enum class SystemErrorEx
{
    seSuccess = 0,                      ///< 0 Не явялется ошибкой

    seInvalidPtr,                       ///< Не валидный указатель

    seFileNotExists,                    ///< Файл не существует
    seDirNotExists,                     ///< Директория не существует
    seObjectNotFile,                    ///< Объект не является файлом
    seObjectNotDir,                     ///< Объект не является директорией
    seFileNotOpen,                      ///< Файл не открыт
    seOpenFileFail,                     ///< Не удалось открыть файл
    seReadFileFail,                     ///< Ошибка при чтении файла

    seCount
};
//---------------------------------------------------------------------------
/**
 * @brief The SystemErrorExCategory class - Класс, определяющий категорию расширеных системных ошибки
 *
 * @authors Alekseev_s
 * @date 21.11.2020
 */
class SystemErrorExCategory : public QObject, public std::error_category
{
    Q_OBJECT
public:

    /**
     * @brief SystemErrorExCategory - Конструктор по умолчанию
     */
    SystemErrorExCategory();

    /**
     * @brief ~SystemErrorExCategory - Виртуальный деструктор по умолчанию
     */
    virtual ~SystemErrorExCategory() override = default;

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
// Define the linkage for this function to be used by external code.
// This would be the usual __declspec(dllexport) or __declspec(dllimport)
// if we were in a Windows DLL etc. But for this example use a global
// instance but with inline linkage so multiple definitions do not collide.
#define SYSTEMERREX_API_DECL extern inline
//-----------------------------------------------------------------------------
/**
 * @brief ConversionErrc_category - Функцию, возвращающую статический экземпляр пользовательской категории
 * @return Вернёт статический экземпляр пользовательской категории
 */
SYSTEMERREX_API_DECL const SystemErrorExCategory &ConversionErrc_category();
//-----------------------------------------------------------------------------
} // namespace hmcommon
//-----------------------------------------------------------------------------
namespace std
{
// Сообщаем метапрограммированию C++ 11 STL, что enum
// зарегистрирован в стандартной системе кодов ошибок
template <> struct is_error_code_enum<hmcommon::SystemErrorEx> : true_type
{};
}
//-----------------------------------------------------------------------------
/**
 * @brief make_error_code - Перегрузка глобальной функции make_error_code () нашем пользовательским перечислением.
 * @param inErrCode - Код ошибки
 * @return Вернёт сформированный экемпляр std::error_code
 */
std::error_code make_error_code(hmcommon::SystemErrorEx inErrCode);
//-----------------------------------------------------------------------------


#endif // SYSTEMERROREX_H
