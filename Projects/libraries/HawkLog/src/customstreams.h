#ifndef CUSTOMSTREAMS_H
#define CUSTOMSTREAMS_H

#include <QTextStream>

namespace hmlog
{
    /**
     * @brief QStdOut - Функция вернёт текстовый поток на онове стандартного потока
     * @return Вернёт текстовый поток на онове стандартного потока
     */
    QTextStream& QStdOut();

    /**
     * @brief reset - Функция сбросит цветовой формат в потоке
     * @param inStream - Входной поток
     * @return Вернёт обработанный поток
     */
    QTextStream& reset(QTextStream& inStream);

    /**
     * @brief green - Функция установит тексту потока зелёный цвет
     * @param inStream - Входной поток
     * @return Вернёт обработанный поток
     */
    QTextStream& green(QTextStream& inStream);

    /**
     * @brief blue - Функция установит тексту потока синий цвет
     * @param inStream - Входной поток
     * @return Вернёт обработанный поток
     */
    QTextStream& blue(QTextStream& inStream);

    /**
     * @brief yellow - Функция установит тексту потока жёлтый цвет
     * @param inStream - Входной поток
     * @return Вернёт обработанный поток
     */
    QTextStream& yellow(QTextStream& inStream);

    /**
     * @brief red - Функция установит тексту потока красный цвет
     * @param inStream - Входной поток
     * @return Вернёт обработанный поток
     */
    QTextStream& red(QTextStream& inStream);

} // namespace hmlog

#endif // CUSTOMSTREAMS_H
