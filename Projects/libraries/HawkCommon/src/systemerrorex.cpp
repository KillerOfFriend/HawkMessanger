#include "systemerrorex.h"

using namespace hmcommon;

//-----------------------------------------------------------------------------
SystemErrorExCategory::SystemErrorExCategory() :
    QObject(nullptr),
    std::error_category()
{

}
//-----------------------------------------------------------------------------
const char* SystemErrorExCategory::name() const noexcept
{
    static const std::string CategoryName = tr("Расширеные системные ошибки").toStdString();
    return CategoryName.c_str();
}
//-----------------------------------------------------------------------------
std::string SystemErrorExCategory::message(int inCode) const
{
    std::string Result;

    switch (static_cast<SystemErrorEx>(inCode))
    {
        case SystemErrorEx::seSuccess:          { Result = tr("Успех").toStdString(); break; }
        case SystemErrorEx::seFileNotExists:    { Result = tr("Файл не существует").toStdString(); break; }
        case SystemErrorEx::seDirNotExists:     { Result = tr("Директория не существует").toStdString(); break; }
        case SystemErrorEx::seObjectNotFile:    { Result = tr("Объект не является файлом").toStdString(); break; }
        case SystemErrorEx::seObjectNotDir:     { Result = tr("Объект не является директорией").toStdString(); break; }
        case SystemErrorEx::seFileNotOpen:      { Result = tr("Файл не открыт").toStdString(); break; }
        case SystemErrorEx::seOpenFileFail:     { Result = tr("Не удалость открыть файл").toStdString(); break; }
        case SystemErrorEx::seReadFileFail:     { Result = tr("При чтении файла произошла ошибка").toStdString(); break; }



        default: Result = ( tr("Не известная ошибка с кодом: ") + QString::number(inCode) ).toStdString();
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
SYSTEMERREX_API_DECL const hmcommon::SystemErrorExCategory &hmcommon::ConversionErrc_category()
{
  static hmcommon::SystemErrorExCategory category;
  return category;
}
//-----------------------------------------------------------------------------
std::error_code make_error_code(hmcommon::SystemErrorEx inErrCode)
{
  return {static_cast<int>(inErrCode), hmcommon::ConversionErrc_category()};
}
//-----------------------------------------------------------------------------
