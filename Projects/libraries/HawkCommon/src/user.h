#ifndef USER_H
#define USER_H

/**
 * @file user.h
 * @brief Содержит описание пользователя системы
 */

#include <QUuid>
#include <QDate>
#include <QString>
#include <QDateTime>
#include <QByteArray>

namespace hmcommon
{
//-----------------------------------------------------------------------------
/**
 * @brief The eSex enum - Перечисление полов
 */
enum class eSex
{
    sNotSpecified,  ///< Пол не указан
    sMale,          ///< Мужской
    sFemale         ///< Женский
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

    QString m_login;                    ///< Логин пользователя
    QByteArray m_passwordHash;          ///< Хеш пароля пользователя

    QString m_name;                     ///< Реальное имя пользователя
    eSex m_sex = eSex::sNotSpecified;   ///< Пол пользователя

    QDate m_birthday;                   ///< Дата рождения пользователя

public:

    const QUuid m_uuid;                 ///< UUID пользователя
    const QDateTime m_registrationDate; ///< Дата регистрации пользователя

    /**
     * @brief HMUser - Инициализирующий конструктор
     * @param inUuid - Uuid пользователя
     * @param inRegistrationDate - Дата регистрации
     */
    HMUser(const QUuid& inUuid, const QDateTime& inRegistrationDate = QDateTime::currentDateTime());

    /**
     * @brief ~HMUser - Деструктор по умолчанию
     */
    ~HMUser() = default;

    /**
     * @brief operator == - Оператор сравнения
     * @param inOther - Сравниваемый объект
     * @return - Вернёт оператор сравнения
     */
    bool operator== (const HMUser& inOther) const;

    /**
     * @brief setLogin - Метод задаст логин
     * @param inLogin - Новое значение логина
     */
    void setLogin(const QString& inLogin);

    /**
     * @brief getLogin - Метод вернёт логин
     * @return Вернёт логин
     */
    QString getLogin() const;

    /**
     * @brief setPassword - Метод задаст пароль
     * @param inPassword - Пароль пользователя
     * @return Вернёт признак успешности операции
     */
    bool setPassword(const QString& inPassword);

    /**
     * @brief setPasswordHash - Метод задаст хеш пароля
     * @param inPasswordHash - Новое значение хеша пароля
     */
    void setPasswordHash(const QByteArray& inPasswordHash);

    /**
     * @brief getPasswordHash - Метод вернёт хеш пароля
     * @return Вернёт хеш пароля
     */
    QByteArray getPasswordHash() const;

    /**
     * @brief setName - Метод задаст имя
     * @param inName - Новое значение имени
     */
    void setName(const QString& inName);

    /**
     * @brief getName - Метод вернёт имя
     * @return Вернёт имя
     */
    QString getName() const;

    /**
     * @brief setSex - Метод задаст пол
     * @param inSex - Новое значение пола
     */
    void setSex(const eSex& inSex);

    /**
     * @brief getSex - Метод вернёт пол
     * @return Вернёт пол
     */
    eSex getSex() const;

    /**
     * @brief setBirthday - Метод задаст дату рожения
     * @param inBirthday - Новое значение даты рождения
     */
    void setBirthday(const QDate& inBirthday);

    /**
     * @brief getBirthday - Метод вернёт дату рождения
     * @return Вернёт дату рождения
     */
    QDate getBirthday() const;

};
//-----------------------------------------------------------------------------
} // namespace hmcommon

#endif // USER_H
