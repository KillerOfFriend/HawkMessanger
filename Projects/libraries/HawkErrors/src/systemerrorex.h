#ifndef SYSTEMERROREX_H
#define SYSTEMERROREX_H

/**
 * @file systemerrorex.h
 * @brief Содержит описание категории расширеных системных ошибок
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
enum class eSystemErrorEx
{
    seSuccess = C_SUCCESS,                          ///< 0 Не явялется ошибкой

    seInvalidPtr = C_CATEGORY_SYSTEMERROREX_START,  ///< Не валидный указатель
    seIncorrecVersion,                              ///< Не корректная версия
    seIncorretData,                                 ///< Не корректные данные
    seOperationNotSupported,                        ///< Операция не поддерживается

    seInputOperationFail,                           ///< Операция ввода прошла с ошибкой
    seOutputOperationFail,                          ///< Операция вывода прошла с ошибкой

    seFileNotExists,                                ///< Файл не существует
    seDirNotExists,                                 ///< Директория не существует
    seObjectNotFile,                                ///< Объект не является файлом
    seObjectNotDir,                                 ///< Объект не является директорией
    seFileNotOpen,                                  ///< Файл не открыт
    seOpenFileFail,                                 ///< Не удалось открыть файл
    seReadFileFail,                                 ///< Ошибка при чтении файла

    seContainerEmpty,                               ///< Контейнер пуст
    seNotInContainer,                               ///< Объект не в контейнере
    seAlredyInContainer,                            ///< Объект уже в контейнере
    seIndexOutOfContainerRange,                     ///< Индекс за пределами контейнера

    seCount                                         ///< Количество
};
//---------------------------------------------------------------------------
/**
 * @brief The SystemErrorExCategory class - Класс, определяющий категорию расширеных системных ошибки
 *
 * @authors Alekseev_s
 * @date 21.11.2020
 */
class SystemErrorExCategory : public std::error_category
{
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
/**
 * @brief ConversionSystemErrorEx_category - Функцию, возвращающую статический экземпляр пользовательской категории
 * @return Вернёт статический экземпляр пользовательской категории
 */
extern inline const SystemErrorExCategory &ConversionSystemErrorEx_category();
//-----------------------------------------------------------------------------
} // namespace hmcommon
//-----------------------------------------------------------------------------
namespace std
{
// Сообщаем метапрограммированию C++ 11 STL, что enum
// зарегистрирован в стандартной системе кодов ошибок
template <> struct is_error_code_enum<errors::eSystemErrorEx> : true_type
{};
}
//-----------------------------------------------------------------------------
/**
 * @brief make_error_code - Перегрузка глобальной функции make_error_code () нашем пользовательским перечислением.
 * @param inErrCode - Код ошибки
 * @return Вернёт сформированный экемпляр errors::error_code
 */
errors::error_code make_error_code(errors::eSystemErrorEx inErrCode);
//-----------------------------------------------------------------------------


#endif // SYSTEMERROREX_H
