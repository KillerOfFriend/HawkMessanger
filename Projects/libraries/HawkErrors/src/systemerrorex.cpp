#include "systemerrorex.h"

using namespace errors;

//-----------------------------------------------------------------------------
SystemErrorExCategory::SystemErrorExCategory() : std::error_category()
{

}
//-----------------------------------------------------------------------------
const char* SystemErrorExCategory::name() const noexcept
{
    return C_CATEGORY_SYSTEMERROREX_NAME.c_str();
}
//-----------------------------------------------------------------------------
std::string SystemErrorExCategory::message(int inCode) const
{
    std::string Result;

    switch (static_cast<errors::eSystemErrorEx>(inCode))
    {
        case errors::eSystemErrorEx::seSuccess:                     { Result = C_ERROR_SUCESS_TEXT; break; }

        case errors::eSystemErrorEx::seInvalidPtr:                  { Result = "Не валидный указатель"; break; }
        case errors::eSystemErrorEx::seIncorrecVersion:             { Result = "Версия не соответствует"; break; }
        case errors::eSystemErrorEx::seIncorretData:                { Result = "Не корректные данные"; break; }
        case errors::eSystemErrorEx::seOperationNotSupported:       { Result = "Операция не поддерживается"; break; }

        case errors::eSystemErrorEx::seInputOperationFail:          { Result = "Во время операции чтения произошла ошибка"; break; }
        case errors::eSystemErrorEx::seOutputOperationFail:         { Result = "Во время операции записи произошла ошибка"; break; }

        case errors::eSystemErrorEx::seFileNotExists:               { Result = "Файл не существует"; break; }
        case errors::eSystemErrorEx::seDirNotExists:                { Result = "Директория не существует"; break; }
        case errors::eSystemErrorEx::seObjectNotFile:               { Result = "Объект не является файлом"; break; }
        case errors::eSystemErrorEx::seObjectNotDir:                { Result = "Объект не является директорией"; break; }
        case errors::eSystemErrorEx::seFileNotOpen:                 { Result = "Файл не открыт"; break; }
        case errors::eSystemErrorEx::seOpenFileFail:                { Result = "Не удалость открыть файл"; break; }
        case errors::eSystemErrorEx::seReadFileFail:                { Result = "При чтении файла произошла ошибка"; break; }

        case errors::eSystemErrorEx::seContainerEmpty:              { Result = "Контейнер пуст"; break; }
        case errors::eSystemErrorEx::seNotInContainer:              { Result = "Объект не в контейнере"; break; }
        case errors::eSystemErrorEx::seAlredyInContainer:           { Result = "Объект уже в контейнере"; break; }
        case errors::eSystemErrorEx::seIndexOutOfContainerRange:    { Result = "Индекс за пределами контейнера"; break; }

        default: Result = C_ERROR_UNKNOWN_TEXT + std::to_string(inCode);
    }

    return Result;
}
//-----------------------------------------------------------------------------
// OPTIONAL: Allow generic error conditions to be compared to me
//    std::error_condition SystemErrorExCategory::default_error_condition(int c) const noexcept
//    {
//        switch (static_cast<SubscriptionError>(c))
//        {
//        case SubscriptionError::EmptyString:
//            return make_error_condition(std::errc::invalid_argument);
//        case SubscriptionError::IllegalChar:
//            return make_error_condition(std::errc::invalid_argument);
//        case SubscriptionError::TooLong:
//            return make_error_condition(std::errc::result_out_of_range);
//        default:
//            // I have no mapping for this code
//            return std::error_condition(c, *this);
//        }
//    }
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
extern inline const errors::SystemErrorExCategory &errors::ConversionSystemErrorEx_category()
{
  static errors::SystemErrorExCategory category;
  return category;
}
//-----------------------------------------------------------------------------
errors::error_code make_error_code(errors::eSystemErrorEx inErrCode)
{
  return { static_cast<int>(inErrCode), errors::ConversionSystemErrorEx_category() };
}
//-----------------------------------------------------------------------------
