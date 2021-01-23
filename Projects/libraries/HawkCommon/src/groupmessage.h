#ifndef HMGroupInfoMESSAGE_H
#define HMGroupInfoMESSAGE_H

/**
 * @file groupmessage.h
 * @brief Содержит описание сообщения группы
 */

#include <QUuid>
#include <QString>
#include <QDateTime>
#include <QByteArray>

#include "errorcode.h"

namespace hmcommon
{
//-----------------------------------------------------------------------------
/**
 * @brief The eMsgType enum - Перечень типов сообщений чата
 */
enum class eMsgType
{
    mtEmpty,            ///< Данные не заданы
    mtText,             ///< Текстовое сообщение
    mtImage,            ///< Изображение

    mtCount
};
//-----------------------------------------------------------------------------
/**
 * @brief The MsgData struct - Структура, хранящая данные сообщения
 *
 * @authors Alekseev_s
 * @date 25.11.2020
 */
struct MsgData
{
    /**
     * @brief MsgData - Инициализирующий конструктор
     * @param inType - Тип данных
     * @param inData - Данные
     */
    MsgData(const eMsgType& inType, const QByteArray& inData);

    /**
     * @brief MsgData - Копирующий конструктор
     * @param inOther - Копируемый объект
     */
    MsgData(const MsgData& inOther);

    /**
     * @brief ~MsgData - Деструктор по умолчанию
     */
    ~MsgData() = default;

    /**
     * @brief operator = - Оператор копирования
     * @param inOther - Копируемый объект
     * @return Вернёт копию объекта
     */
    MsgData& operator=(const MsgData& inOther);

    eMsgType m_type;                ///< Тип данных
    QByteArray m_data;              ///< Хранилище данных
};
//-----------------------------------------------------------------------------
/**
 * @brief The MsgRange struct - Структура, описывающая временной промежуток сообщений
 *
 * @authors Alekseev_s
 * @date 25.11.2020
 */
struct MsgRange
{
    /**
     * @brief MsgRange - Инициализирующий конструктор
     * @param inFrom - Начало промежутка
     * @param inTo - Окончание промежутка
     */
    MsgRange(const QDateTime& inFrom, const QDateTime& inTo);

    /**
     * @brief ~MsgRange - Деструктор по умолчанию
     */
    ~MsgRange() = default;

    QDateTime m_from;   ///< Начало временного промежутка
    QDateTime m_to;     ///< Окончание временного промежутка
};
//-----------------------------------------------------------------------------
/**
 * @brief The HMGroupInfoMessage class - Класс, описывающий сообщение группы
 *
 * @authors Alekseev_s
 * @date 25.11.2020
 */
class HMGroupInfoMessage
{
private:

    MsgData m_message;              ///< Данные сообщения

public:

    const QUuid m_uuid;             ///< UUID сообщения
    const QUuid m_group;            ///< UUID группы, которой пренадлежит сообщение
    const QDateTime m_createTime;   ///< Время создания

    /**
     * @brief HMGroupInfoMessage - Инициализирующий конструктор
     * @param inUuid - UUID сообщения
     * @param inGroupUuid - UUID группы
     * @param inCreateTime - Время создания
     */
    HMGroupInfoMessage(const QUuid& inUuid, const QUuid& inGroupUuid, const QDateTime& inCreateTime = QDateTime::currentDateTime());

    /**
     * @brief ~HMGroupInfoMessage - Деструктор по умолчанию
     */
    ~HMGroupInfoMessage() = default;

    /**
     * @brief setMessage - Метод задаст данные сообщения
     * @param inMessageData - Новые данные сообщения
     * @return Вернёт признак ошибки
     */
    hmcommon::error_code setMessage(const MsgData& inMessageData);

    /**
     * @brief getMesssage - Метод вернёт данные сообщения
     * @return Вернёт данные сообщения
     */
    MsgData getMesssage() const;

};
//-----------------------------------------------------------------------------
}

#endif // HMGroupInfoMESSAGE_H
