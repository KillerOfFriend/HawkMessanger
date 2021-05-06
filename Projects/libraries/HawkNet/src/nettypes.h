#ifndef NETTYPES_H
#define NETTYPES_H

#include <sstream>

namespace net
{
//-----------------------------------------------------------------------------
/**
 * @brief The eConnectionStatus enum - Перечисление статусов соединения
 */
enum class eConnectionStatus
{
    csUnknown = 0,      ///< Соединение не определено
    csDisconnected,     ///< Соединение отсутствует
    csDisconnecting,    ///< Соединение разрывается
    csConnecting,       ///< Соединени устанавливается
    csConnected,        ///< Соединение установлено

    csCount             ///< Счётчик
};
//-----------------------------------------------------------------------------
typedef std::basic_istringstream<char> iByteStream; ///< Буфер получаемых данных
typedef std::basic_ostringstream<char> oByteStream; ///< Буфер отправляемых данных
//-----------------------------------------------------------------------------
constexpr char C_DATA_WRAP_END =    0x1D;   ///< Разделитель "обёртки" и потока даннх
constexpr char C_DATA_SEPARATOR =   0x1E;   ///< Символ, разделяющий "сообщения" в потоке данных (RECORD SEPARATOR (RS) UP ARROW)
constexpr char C_DATA_REPLACER =    0x1F;   ///< Символ заменитель разделяющего символа в потоке данных (UNIT SEPARATOR (US) DOWN ARROW)
//-----------------------------------------------------------------------------
static_assert(C_DATA_SEPARATOR != C_DATA_REPLACER, "Символ разделителя потока и символ заменитель не должны совпадать!");
static_assert(C_DATA_SEPARATOR != C_DATA_WRAP_END, "Символ разделителя потока и символ разделитель \"обёртки\" не должны совпадать!");
static_assert(C_DATA_REPLACER != C_DATA_WRAP_END, "Символ заменитель и символ разделитель \"обёртки\" не должны совпадать!");
//-----------------------------------------------------------------------------
static const std::string C_WRAP_HEAD =      "WRAP_INFO";        ///< Заголовок информации об обёртке
static const std::string C_WRAP_SEQUENCE =  "replace_sequence"; ///< Подменяющая последоваетльность
static const std::string C_WRAP_COUNT =     "replace_count";    ///< Количество произведённых замен
//-----------------------------------------------------------------------------
} // namespace net

#endif // NETTYPES_H
