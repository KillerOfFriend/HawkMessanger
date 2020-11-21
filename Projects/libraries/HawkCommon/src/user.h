#ifndef USER_H
#define USER_H

/**
 * @file user.h
 * @brief Содержит описание пользователя системы
 */

#include <QUuid>
#include <QString>
#include <QDateTime>
#include <QByteArray>

//-----------------------------------------------------------------------------
/**
 * @brief The eSex enum - Перечисление полов
 */
enum class eSex
{
    sMale,      ///< Мужской
    sFemale     ///< Женский
};
//-----------------------------------------------------------------------------
/**
 * @brief The HMUser class - Класс, описывающий пользователя системы
 *
 * @authors Alekseev_s
 * @date 08.11.2020
 */
class HMUser
{
private:

    QString m_login;                ///< Логин пользователя
    QByteArray m_passwordHash;      ///< Хеш пароля пользователя

    QString m_name;                 ///< Реальное имя пользователя
    eSex m_sex;                     ///< Пол пользователя

    QDateTime m_birthday;           ///< Дата рождения пользователя
    QDateTime m_registrationDate;   ///< Дата регистрации пользователя

public:

    /**
     * @brief HMUser - Инициализирующий конструктор
     * @param inUuid - Uuid пользователя
     */
    HMUser(const QUuid& inUuid);

    /**
     * @brief ~HMUser - Деструктор по умолчанию
     */
    ~HMUser() = default;

    const QUuid m_uuid;     ///< UUID пользователя

};
//-----------------------------------------------------------------------------

#endif // USER_H
