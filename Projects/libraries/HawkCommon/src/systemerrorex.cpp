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

    switch (static_cast<eSystemErrorEx>(inCode))
    {
        case eSystemErrorEx::seSuccess:                     { Result = tr("Успех").toStdString(); break; }

        case eSystemErrorEx::seInvalidPtr:                  { Result = tr("Не валидный указатель").toStdString(); break; }
        case eSystemErrorEx::seIncorrecVersion:             { Result = tr("Версия не соответствует").toStdString(); break; }
        case eSystemErrorEx::seIncorretData:                { Result = tr("Не корректные данные").toStdString(); break; }
        case eSystemErrorEx::seOperationNotSupported:       { Result = tr("Операция не поддерживается").toStdString(); break; }

        case eSystemErrorEx::seInputOperationFail:          { Result = tr("Во время операции чтения произошла ошибка").toStdString(); break; }
        case eSystemErrorEx::seOutputOperationFail:         { Result = tr("Во время операции записи произошла ошибка").toStdString(); break; }

        case eSystemErrorEx::seFileNotExists:               { Result = tr("Файл не существует").toStdString(); break; }
        case eSystemErrorEx::seDirNotExists:                { Result = tr("Директория не существует").toStdString(); break; }
        case eSystemErrorEx::seObjectNotFile:               { Result = tr("Объект не является файлом").toStdString(); break; }
        case eSystemErrorEx::seObjectNotDir:                { Result = tr("Объект не является директорией").toStdString(); break; }
        case eSystemErrorEx::seFileNotOpen:                 { Result = tr("Файл не открыт").toStdString(); break; }
        case eSystemErrorEx::seOpenFileFail:                { Result = tr("Не удалость открыть файл").toStdString(); break; }
        case eSystemErrorEx::seReadFileFail:                { Result = tr("При чтении файла произошла ошибка").toStdString(); break; }

        case eSystemErrorEx::seContainerEmpty:              { Result = tr("Контейнер пуст").toStdString(); break; }
        case eSystemErrorEx::seNotInContainer:              { Result = tr("Объект не в контейнере").toStdString(); break; }
        case eSystemErrorEx::seAlredyInContainer:           { Result = tr("Объект уже в контейнере").toStdString(); break; }
        case eSystemErrorEx::seIndexOutOfContainerRange:    { Result = tr("Индекс за пределами контейнера").toStdString(); break; }



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
hmcommon::error_code make_error_code(hmcommon::eSystemErrorEx inErrCode)
{
  return {static_cast<int>(inErrCode), hmcommon::ConversionErrc_category()};
}
//-----------------------------------------------------------------------------
