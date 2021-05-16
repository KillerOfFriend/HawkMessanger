#ifndef HMABSTRACTCONNECTION_H
#define HMABSTRACTCONNECTION_H

/**
 * @file abstractconnection.h
 * @brief Содержит описание абстрактного соединения
 */

#include <atomic>

#include "Interface/connection.h"

namespace net
{
//-----------------------------------------------------------------------------
/**
 * @brief The HMAbstractConnection class - Абстракция, описывающая базовое соединение
 *
 * @authors Alekseev_s
 * @date 12.04.2021
 */
class HMAbstractConnection : public HMConnection
{
public:

    /**
     * @brief HMAbstractConnection - Конструктор по умолчанию
     */
    HMAbstractConnection();

    /**
     * @brief ~HMAbstractConnection - Деструктор по умолчанию
     */
    virtual ~HMAbstractConnection() override = default;

    /**
     * @brief setID - Метод задаст уникальный идентификатор
     * @param inID - Новый уникальный идентификатор
     */
    void setID(const std::size_t inID);

    /**
     * @brief getID - Метод вернут уникальный идентификатор соединения
     * @return - Вернут уникальный идентификатор
     */
    std::size_t getID() const;

private:

    std::atomic<std::size_t> m_id; ///< Уникальный идентификатор соединения

}; // namespace net
//-----------------------------------------------------------------------------
}
#endif // HMABSTRACTCONNECTION_H
