#ifndef NOTCOPYABLE_H
#define NOTCOPYABLE_H

/**
 * @file NotСopyable.h
 * @brief Содержит описание не копируемой базовой структуры
 */

namespace hmcommon
{
//-----------------------------------------------------------------------------
/**
 * @brief The HMNotCopyable struct - Структура с запретом копирования
 *
 * @authors Alekseev_s
 * @date 16.05.2021
 */
struct HMNotCopyable
{
    /**
     * @brief HMNotCopyable - Конструктор по умолчанию
     */
    HMNotCopyable() = default;

    /**
     * @brief ~HMNotCopyable - Виртуальный деструктор по умолчанию
     */
    virtual ~HMNotCopyable() = default;

    /**
     * @brief HMNotCopyable - Запрещённый конструктор копирования
     */
    HMNotCopyable(const HMNotCopyable&) = delete;

    /**
     * @brief operator = - Запрещённый оператор копирования
     * @return ---
     */
    HMNotCopyable& operator=(const HMNotCopyable&) = delete;

};
//-----------------------------------------------------------------------------
} // namespace hmcommon

#endif // NOTCOPYABLE_H
