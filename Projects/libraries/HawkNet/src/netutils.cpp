#include "netutils.h"

#include <cassert>
#include <algorithm>

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
nlohmann::json makeJsonWrap(const std::string& inReplaceSequence, const std::uint64_t inReplaceCount)
{
    nlohmann::json Json;  // Формируемая обёртка Json

    // Формируем заголовок
    Json[net::C_WRAP_HEAD][net::C_WRAP_SEQUENCE] = inReplaceSequence; // Заменяющая последовательность
    Json[net::C_WRAP_HEAD][net::C_WRAP_COUNT] = inReplaceCount; // Количество замен

    return Json;
}
//-----------------------------------------------------------------------------
bool isJsonWrap(const nlohmann::json& inWrap)
{
    return inWrap.find(net::C_WRAP_HEAD) != inWrap.end() ||
            !inWrap[net::C_WRAP_HEAD].is_null() ||
            inWrap[net::C_WRAP_HEAD].is_object() ||
            inWrap[net::C_WRAP_HEAD].find(net::C_WRAP_SEQUENCE) != inWrap[net::C_WRAP_HEAD].end() ||
            inWrap[net::C_WRAP_HEAD][net::C_WRAP_SEQUENCE].is_string() ||
            inWrap[net::C_WRAP_HEAD].find(net::C_WRAP_COUNT) != inWrap[net::C_WRAP_HEAD].end() ||
            inWrap[net::C_WRAP_HEAD][net::C_WRAP_COUNT].is_number_unsigned();
}
//-----------------------------------------------------------------------------
//nlohmann::json net::strToBinaryJson(const std::string& inStr)
//{
//    nlohmann::json Result;

//    if (!inStr.empty())
//    {
//        std::vector<std::uint8_t> BinarVector(inStr.size());
//        std::transform(inStr.cbegin(), inStr.cend(), BinarVector.begin(), [](const char& Char){ return static_cast<std::uint8_t>(Char); });
//        Result = BinarVector;
//    }

//    return Result;
//}
////-----------------------------------------------------------------------------
//std::string net::binaryJsonToStr(const nlohmann::json& inJson)
//{
//    std::string Result = "";

//    if (!inJson.empty() && inJson.is_array())
//    {
//        std::vector<std::uint8_t> BinarVector = inJson.get<std::vector<std::uint8_t>>();

//        if (!BinarVector.empty())
//        {
//            //Result.reserve(BinarVector.size());
//            std::transform(BinarVector.cbegin(), BinarVector.cend(),  std::back_inserter(Result), [](const std::uint8_t& Byte){ return static_cast<char>(Byte); });
//        }
//    }

//    return Result;
//}
//-----------------------------------------------------------------------------
bool net::getWrapAndMessageData(const std::string& inTextData, nlohmann::json& outWrapInfo, std::string& outMessageData)
{
    bool Result = true;

    std::string::size_type WrapEndFindRes = inTextData.find(net::C_DATA_WRAP_END);

    if (WrapEndFindRes == std::string::npos) // Разделитель обёртки не найден
        Result = false;
    else // Если найден символ разделиетель обёртки
    {
        outWrapInfo = nlohmann::json::parse(inTextData.substr(0, WrapEndFindRes), nullptr, false); // Пытаемся распарсить обёртку

        if (outWrapInfo.is_discarded() || !isJsonWrap(outWrapInfo)) // Ошибка парсинга или обёртка не соответствует
            Result = false;
        else // Если парсинг прошёл успешно и JSON является заголовком обёртки
            outMessageData = inTextData.substr(WrapEndFindRes + 1); // Возвращаем данные
    }

    return Result;
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

        // Формируем обёрнутую последовательность {JSON WRAP DATA}{WRAP SEPARATOR}{REPLACED STR DATA}
        inData = oByteStream(makeJsonWrap(ReplaceSequence, ReplaceCount).dump() + C_DATA_WRAP_END + StrData);
    }

    /*
     * Сценарии:
     * 1) Если в отправляемой последовательности присутствует запрещённый символ "разделитель потока"
     * то вернём "обёртку" в формате {JSON WRAP DATA}{WRAP SEPARATOR}{REPLACED STR DATA}
     * 2) Если запрещённый символ не найден, то вернём данные без изменений
     */
    return std::move(inData);
}
//-----------------------------------------------------------------------------
net::iByteStream net::unwrap(iByteStream&& inData)
{
    nlohmann::json JsonWrapData; // Данные обёртки
    std::string WrapedData = ""; // "Обёрнутые" данные сообщения

    if (getWrapAndMessageData(inData.str(), JsonWrapData, WrapedData)) // Если получены данные обёртки и "обёрнутые" данные
    {
        if (JsonWrapData[C_WRAP_HEAD][C_WRAP_COUNT] > 0) // Если были замещения
        {   // Нужно развернуть обратно
            std::uint64_t ReplaceCount = 0; // Количество произведённых обратных замен
            std::string ReplaceSequence = JsonWrapData[C_WRAP_HEAD][C_WRAP_SEQUENCE].get<std::string>(); // Замещающая последовательность
            std::string::size_type ReplacerFindResult = WrapedData.find(ReplaceSequence); // Результат поиска замещающей последовательности

            while (ReplacerFindResult != std::string::npos) // Восстанавливаем, пока замещающая последоваетльность не перестанет встречаться в потоке
            {
                ReplaceCount++; // Увеличиваем счётчик подбора
                // Обратно заменим замещающую последовательность на разделитель потока даннх
                WrapedData.replace(ReplacerFindResult, ReplaceSequence.size(), { C_DATA_SEPARATOR });

                ReplacerFindResult = WrapedData.find(ReplaceSequence); // Ищим замещающую последоваетльность
            }

            assert(ReplaceCount == JsonWrapData[C_WRAP_HEAD][C_WRAP_COUNT].get<std::uint64_t>()); // Количество замещений и востановлений должно совпасть
        }

        inData = iByteStream(WrapedData);
    }

    /*
     * Сценарии:
     * 1) Если удалось получить данные обёртки то производим разворачивание
     * и вернём восстановленны данные
     * 2) Если данныз обёртки не обнаружено, то вернём как есть
     */
    return std::move(inData);
}
//-----------------------------------------------------------------------------

