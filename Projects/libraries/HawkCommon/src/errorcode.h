#ifndef ERRORCODE_H
#define ERRORCODE_H

#include <system_error>

#include <QString>

namespace hmcommon
{
//-----------------------------------------------------------------------------
/**
 * @brief The error_code class - Класс, описывающий признак ошибки
 * С++17 [[nodiscard]] будет генерировать предупреждения
 * если разработчик не будет принимать возвращаемую ошибку.
 *
 * @authors Alekseev_s
 * @date 23.01.2021
 */
class [[nodiscard]] error_code : public std::error_code
{
public:
    /**
     * @brief error_code - Конструктор по умолчанию
     */
    error_code() = default;

    /**
     * @brief error_code - Инициализирующий конструктор
     * @param inCode - Код ошибки
     * @param inErrCat - Категория
     */
    error_code(int inCode, const std::error_category& inErrCat);

    /**
     * @brief ~error_code - Деструктор по умолчанию
     */
    ~error_code() = default;

    /**
     * @brief error_code - Конструктор копирования
     * @param inOtherEC - Копируемый объект std::error_code
     */
    error_code(const std::error_code& inOtherEC);

    /**
     * @brief error_code - Конструктор копирования
     * @param inOther - Копируемый объект hmcommon::error_code
     */
    error_code(const error_code& inOther);

    /**
     * @brief operator = - Оператор присваивания
     * @param inOther - Копируемый объект std::error_code
     * @return Вернёт копию объекта
     */
    error_code& operator= (const std::error_code& inOther);

    /**
     * @brief operator = - Оператор присваивания
     * @param inOther - Копируемый объект std::error_code
     * @return Вернёт копию объекта
     */
    error_code& operator= (const error_code& inOther);

    /**
     * @brief message_qstr - Метод вернёт описание ошибки в виде QString
     * @return Вернёт описание ошибки
     */
    QString message_qstr() const;
};
//-----------------------------------------------------------------------------
}

#endif // ERRORCODE_H
