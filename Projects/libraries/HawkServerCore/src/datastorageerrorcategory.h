#ifndef DATASTORAGEERRORCATEGORY_H
#define DATASTORAGEERRORCATEGORY_H

/**
 * @file systemerrorex.h
 * @brief Содержит описание категории ошибок хранилища данных
 */

#include <string>
#include <system_error>

#include <QObject>

namespace hmservcommon
{
//-----------------------------------------------------------------------------
static const std::int32_t C_STORAG_ERROR_START = 300; ///< Начальное значение ошибок хранилища данных
//-----------------------------------------------------------------------------
/**
 * @brief The eDataStoragError enum - Перечень ошибок хранилища данных
 */
enum class eDataStoragError
{
    dsSuccess = 0,                      ///< 0 Не явялется ошибкой

    dsNotOpen = C_STORAG_ERROR_START,   ///< Хранилище не открыто

    dsUserNotExists,                    ///< Пользователь не существует
    dsUserAlreadyExists,                ///< Пользователь уже существует
    dsUserUUIDCorrupted,                ///< UUID пользователя повреждён
    dsUserRegistrationDateCorrupted,    ///< Время регистрации пользователя поврежено
    dsUserLoginCorrupted,               ///< Логин пользователя повреждён
    dsUserPasswordHashCorrupted,        ///< Хеш пароля пользователя повреждён
    dsUserNameCorrupted,                ///< Имя пользователя повреждено
    dsUserSexCorrupted,                 ///< Пол пользователя поврежден
    dsUserBirthday,                     ///< День рождения пользователя повреждён

    dsGroupNotExists,                   ///< Группа не существует
    dsGroupAlreadyExists,               ///< Группа уже существует
    dsGroupUUIDCorrupted,               ///< UUID группы повреждён
    dsGroupRegistrationDateCorrupted,   ///< Время группы пользователя поврежено
    dsGroupNameCorrupted,               ///< Имя группы повреждено
    dsGroupUsersCorrupted,              ///< Пользователи группы повреждены

    dsCount
};
//-----------------------------------------------------------------------------
/**
 * @brief The DataStorageErrorCategory class - Класс, определяющий категорию ошибко хранилища данных
 *
 * @authors Alekseev_s
 * @date 21.11.2020
 */
class DataStorageErrorCategory : public QObject,  public std::error_category
{
    Q_OBJECT
public:

    /**
     * @brief DataStorageErrorCategory - Конструктор по умолчанию
     */
    DataStorageErrorCategory();

    /**
     * @brief ~DataStorageErrorCategory - Виртуальный деструктор по умолчанию
     */
    virtual ~DataStorageErrorCategory() override = default;

    /**
     * @brief name - Метод вернёт имя категории
     * @return Вернёт имя категории
     */
    virtual const char *name() const noexcept override final;

    /**
     * @brief message - Метод вернёт сообщение ошибки
     * @param inCode - Код ошибки
     * @return Вернёт сообщение ошибки
     */
    virtual std::string message(int inCode) const override final;
};
//-----------------------------------------------------------------------------
// Define the linkage for this function to be used by external code.
// This would be the usual __declspec(dllexport) or __declspec(dllimport)
// if we were in a Windows DLL etc. But for this example use a global
// instance but with inline linkage so multiple definitions do not collide.
#define DATASTORAGE_API_DECL extern inline
//-----------------------------------------------------------------------------
/**
 * @brief ConversionErrc_category - Функцию, возвращающую статический экземпляр пользовательской категории
 * @return Вернёт статический экземпляр пользовательской категории
 */
//DATASTORAGE_API_DECL
const DataStorageErrorCategory &ConversionErrc_category();
//-----------------------------------------------------------------------------
} // namespace hmservcommon
//-----------------------------------------------------------------------------
namespace std
{
// Сообщаем метапрограммированию C++ 11 STL, что enum
// зарегистрирован в стандартной системе кодов ошибок
template <> struct is_error_code_enum<hmservcommon::eDataStoragError> : true_type
{};
}
//-----------------------------------------------------------------------------
/**
 * @brief make_error_code - Перегрузка глобальной функции make_error_code () нашем пользовательским перечислением.
 * @param inErrCode - Код ошибки
 * @return Вернёт сформированный экемпляр std::error_code
 */
std::error_code make_error_code(hmservcommon::eDataStoragError inErrCode);
//-----------------------------------------------------------------------------

#endif // DATASTORAGEERRORCATEGORY_H
