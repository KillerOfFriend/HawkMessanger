#include "netutils.h"

#include <cassert>

#include <nlohmann/json.hpp>

//-----------------------------------------------------------------------------
/**
 * @brief makeMark - Функция сформирует замещающую последовательность
 * @param inRepSymbol - Главный символ замещающей последовательности
 * @param inRepeats - Количество повторов символа в замещающей последовательности
 * @return
 */
std::string makeMark(const char inRepSymbol, std::uint8_t inRepeats)
{
    assert(inRepSymbol != '\0'); // Символ не должен быть пустым
    assert(inRepeats != 0); // Количество повторов не должно быть рано 0

    std::string Result = "";

    do
    {  Result += inRepSymbol; } // Добавляем необходимое количество символов замены
    while (--inRepeats);

    return "{" + Result + "}"; // Формируем последовательность замещения
}
//-----------------------------------------------------------------------------
nlohmann::json makeJsonWrap(const std::string& inReplacedStrData, const std::string& inReplaceSequence, const std::uint64_t inReplaceCount)
{
    nlohmann::json Json;  // Формируемая обёртка Json

    // Формируем заголовок
    Json["HEAD"]["replace_sequence"] = inReplaceSequence; // Заменяющая последовательность
    Json["HEAD"]["replace_count"] = inReplaceCount; // Количество замен
    // Формируем данные
    Json["DATA"] = inReplacedStrData; // Данные

    return Json;
}
//-----------------------------------------------------------------------------
bool isJsonWrap(const nlohmann::json& inWrap)
{
    return inWrap.find("HEAD") == inWrap.end() ||
            inWrap["HEAD"].is_null() ||
            inWrap["HEAD"].type() != nlohmann::json::value_t::object ||
            inWrap["HEAD"].find("replace_sequence") == inWrap["HEAD"].end() ||
            inWrap["HEAD"]["replace_sequence"].type() != nlohmann::json::value_t::string ||
            inWrap["HEAD"].find("replace_count") == inWrap["HEAD"].end() ||
            inWrap["HEAD"]["replace_count"].type() != nlohmann::json::value_t::number_unsigned ||
            inWrap.find("DATA") == inWrap.end() ||
            inWrap["DATA"].is_null() ||
            inWrap["DATA"].type() != nlohmann::json::value_t::string;
}
//-----------------------------------------------------------------------------
net::oByteStream net::wrap(oByteStream&& inData)
{
    std::string StrData = inData.str();

    std::uint64_t ReplaceCount = 0; // Количество произведённых замен
    std::string ReplaceSequence = "";   // Заменяющая последовательность

    std::string::size_type SeparatorFindResult = StrData.find(C_DATA_SEPARATOR); // Ищим разделитель в потоке передаваемых данных

    if (SeparatorFindResult != std::string::npos) // Если резделитель присутствует в потоке
    {   // Подбираем замещаюшую последовательность
        std::uint64_t TryOfReplace = 0;     // Попытох подбора заменяющей последовательности

        std::string::size_type ReplacerFindResult = std::string::npos; // Результат поиска замещающей последовательности

        do
        {
            TryOfReplace++; // Увеличиваем счётчик подбора
            ReplaceSequence = makeMark(C_DATA_REPLACER, TryOfReplace); // Формируем замещающую последовательность
            ReplacerFindResult = StrData.find(ReplaceSequence); // Ищим замещающую последоваетльность
        }
        while (ReplacerFindResult != std::string::npos); // Подбираем, пока замещающая последоваетльность не перестанет встречаться в потоке

        // Когда последовательность подобрана

        do
        {
            ReplaceCount++;
            StrData.replace(SeparatorFindResult, sizeof(C_DATA_SEPARATOR), ReplaceSequence); // Заменяем найденый разделитель на подобранную последоваетльность
            SeparatorFindResult = StrData.find(C_DATA_SEPARATOR, SeparatorFindResult); // Снова разделитель в потоке передаваемых данных (начиная с позиции последней замены)
        }
        while (SeparatorFindResult != std::string::npos); // Пока найден разделитель в потоке передаваемых данных и это не последний символ
    }

    return oByteStream(makeJsonWrap(StrData, ReplaceSequence, ReplaceCount).dump());
}
//-----------------------------------------------------------------------------
net::iByteStream net::unwrap(iByteStream&& inData)
{
    iByteStream Result;
    // "Приёмник" обёртки
    nlohmann::json Json = nlohmann::json::parse(inData.str(), nullptr, false); // Пытаемся распарсить обёртку

    if (Json.is_discarded() || isJsonWrap(Json) ) // Если при парсинге произошла ошибка или данные не являются обёрткой
        Result = std::move(inData); // Вернём как есть
    else // Обёртка распаршена успешно
    {
        std::string StrData = Json["DATA"].get<std::string>(); // Получаем обёрнутые данные

        if (Json["HEAD"]["replace_count"] > 0) // Если были замещения
        {   // Нужно развернуть обратно
            std::uint64_t ReplaceCount = 0; // Количество произведённых обратных замен
            std::string ReplaceSequence = Json["HEAD"]["replace_sequence"].get<std::string>(); // Замещающая последовательность
            std::string::size_type ReplacerFindResult = StrData.find(ReplaceSequence); // Результат поиска замещающей последовательности

            while (ReplacerFindResult != std::string::npos) // Восстанавливаем, пока замещающая последоваетльность не перестанет встречаться в потоке
            {
                ReplaceCount++; // Увеличиваем счётчик подбора
                // Обратно заменим замещающую последовательность на разделитель потока даннх
                StrData.replace(ReplacerFindResult, ReplaceSequence.size(), { C_DATA_SEPARATOR });

                ReplacerFindResult = StrData.find(ReplaceSequence); // Ищим замещающую последоваетльность
            }

            assert(ReplaceCount == Json["HEAD"]["replace_count"].get<std::uint64_t>()); // Количество замещений и востановлений должно совпасть
        }

        Result = iByteStream(StrData);
    }

    return Result;
}
//-----------------------------------------------------------------------------
