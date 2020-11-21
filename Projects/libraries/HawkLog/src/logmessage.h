#ifndef LOGMESSAGE_H
#define LOGMESSAGE_H

#include <QString>
#include <QDateTime>

namespace hmlog
{
//-----------------------------------------------------------------------------
/**
 * @brief The eLogMessageType enum - Перечисление типов сообщений логов
 *
 * @author Alekseev_s
 * @date 13.11.2020
 */
enum class eLogMessageType : uint8_t
{
    lmtText     = 0, ///< Простой текст
    lmtDebug    = 1, ///< Отладочное сообщение
    lmtInfo     = 2, ///< Информация
    lmtWarning  = 3, ///< Предупреждение
    lmtError    = 4, ///< Ошибка

    lmtCount
};
//-----------------------------------------------------------------------------
/**
 * @brief The HMLogMessage struct - Структкра, описывающая сообщение логов
 *
 * @author Alekseev_s
 * @date 13.11.2020
 */
struct HMLogMessage
{
    /**
     * @brief HMLogMessage - Инициализирующий конструктор
     * @param inType - Тип сообщения
     * @param inMessage - Текст сообщения
     */
    HMLogMessage(const eLogMessageType inType, const QString& inMessage);

    /**
     * @brief ~HMLogMessage
     */
    ~HMLogMessage() = default;

    const QDateTime m_time;         ///< Время формирования сообщения
    const eLogMessageType m_type;   ///< Тип сообщения
    const QString m_message;        ///< Текст сообщения
};
//-----------------------------------------------------------------------------
} // namespace hmlog



#endif // LOGMESSAGE_H
