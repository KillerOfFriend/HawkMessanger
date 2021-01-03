#ifndef GROUP_H
#define GROUP_H

/**
 * @file group.h
 * @brief Содержит описание группы\чата
 */

#include <vector>
#include <system_error>

#include "user.h"

namespace hmcommon
{
//-----------------------------------------------------------------------------
/**
 * @brief The HMGroup class - Класс, описывающий группу пользователей системы
 *
 * @authors Alekseev_s
 * @date 08.11.2020
 */
class HMGroup
{
private:

    QString m_name;                                 ///< Имя группы
//    std::vector<std::shared_ptr<HMUser>> m_users;   ///< Перечень пользователей группы

public:

    const QUuid m_uuid;                 ///< UUID группы
    const QDateTime m_registrationDate; ///< Дата регистрации группы

    /**
     * @brief HMGroup - Инициализирующий конструктор
     * @param inUuid - Uuid группы
     * @param inRegistrationDate - Дата регистрации группы
     */
    HMGroup(const QUuid& inUuid, const QDateTime& inRegistrationDate = QDateTime::currentDateTime());

    /**
     * @brief ~HMGroup - Деструктор по умолчанию
     */
    ~HMGroup() = default;

    /**
     * @brief operator == - Оператор сравнения
     * @param inOther - Сравниваемый объект
     * @return - Вернёт оператор сравнения
     */
    bool operator== (const HMGroup& inOther) const;

    /**
     * @brief setName - Метод задаст имя группы
     * @param inName - Новое имя группы
     */
    void setName(const QString& inName);

    /**
     * @brief getName - Метод вернёт имя группы
     * @return Вернёт имя группы
     */
    QString getName() const;

//    /**
//     * @brief isUsersEnpty - Метод вернёт признак того, что в группе нет пользователей
//     * @return Вернёт признак того, что в группе нет пользователей
//     */
//    bool isUsersEmpty() const;

//    /**
//     * @brief usersCount - Метод вернёт количество пользователей в группе
//     * @return Вернёт количество пользователей в группе
//     */
//    std::size_t usersCount() const;

//    /**
//     * @brief addUser - Метод добавит пользователя в группу
//     * @param inUser - UUID добавляемого пользователя
//     * @return Вернёт признак ошибки
//     */
//    std::error_code addUser(const std::shared_ptr<HMUser> inUser);

//    /**
//     * @brief removeUser - Метод удалит польоваетля из группы
//     * @param inUserUuid - UUID удаляемого пользователя
//     * @return Вернёт признак ошибки
//     */
//    std::error_code removeUser(const QUuid& inUserUuid);

//    /**
//     * @brief getUser - Метод вернёт пользователя по его порядковому номеру
//     * @param outErrorCode - Признак ошибки
//     * @return Вернёт пользователя или nullptr
//     */
//    std::shared_ptr<HMUser> getUser(const std::size_t inIndex, std::error_code& outErrorCode);

//    /**
//     * @brief contain - Метод проверит, содержит ли группа пользователя
//     * @param inUserUuid - UUID искомого пользователя
//     * @return Вернёт признак вхождения пользователя в группу
//     */
//    bool contain(const QUuid& inUserUuid) const;

};
//-----------------------------------------------------------------------------
} // namespace hmcommon

#endif // GROUP_H
