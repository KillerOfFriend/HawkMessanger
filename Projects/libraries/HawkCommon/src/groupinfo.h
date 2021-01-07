#ifndef GROUP_H
#define GROUP_H

/**
 * @file groupinfo.h
 * @brief Содержит описание основной информации группы\чата
 */

#include <vector>
#include <system_error>

#include <QUuid>
#include <QDateTime>

namespace hmcommon
{
//-----------------------------------------------------------------------------
/**
 * @brief The HMGroupInfo class - Класс, описывающий информацию группы пользователей системы
 *
 * @authors Alekseev_s
 * @date 08.11.2020
 */
class HMGroupInfo
{
private:

    QString m_name;                                 ///< Имя группы

public:

    const QUuid m_uuid;                 ///< UUID группы
    const QDateTime m_registrationDate; ///< Дата регистрации группы

    /**
     * @brief HMGroupInfo - Инициализирующий конструктор
     * @param inUuid - Uuid группы
     * @param inRegistrationDate - Дата регистрации группы
     */
    HMGroupInfo(const QUuid& inUuid, const QDateTime& inRegistrationDate = QDateTime::currentDateTime());

    /**
     * @brief ~HMGroupInfo - Деструктор по умолчанию
     */
    ~HMGroupInfo() = default;

    /**
     * @brief operator == - Оператор сравнения
     * @param inOther - Сравниваемый объект
     * @return - Вернёт оператор сравнения
     */
    bool operator== (const HMGroupInfo& inOther) const;

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

};
//-----------------------------------------------------------------------------
} // namespace hmcommon

#endif // GROUP_H
