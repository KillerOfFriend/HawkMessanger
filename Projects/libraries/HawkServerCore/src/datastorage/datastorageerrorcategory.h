#ifndef DATASTORAGEERRORCATEGORY_H
#define DATASTORAGEERRORCATEGORY_H

/**
 * @file systemerrorex.h
 * @brief Содержит описание категории ошибок хранилища данных
 */

#include <string>
#include <system_error>

#include <QObject>

namespace hmservcommon::datastorage
{
//-----------------------------------------------------------------------------
static const std::int32_t C_STORAG_ERROR_START = 300; ///< Начальное значение ошибок хранилища данных
//-----------------------------------------------------------------------------
/**
 * @brief The eDataStorageError enum - Перечень ошибок хранилища данных
 */
enum class eDataStorageError
{
    dsSuccess = 0,                      ///< 0 Не явялется ошибкой

    // Хранилище
    dsNotOpen = C_STORAG_ERROR_START,   ///< Хранилище не открыто

    // Пользователи
    dsUserUUIDAlreadyRegistered,        ///< В хранилище уже зарегистрирован пользователь с таким UUID
    dsUserLoginAlreadyRegistered,       ///< В хранилище уже зарегистрирован пользователь с таким логином
    dsUserPasswordIncorrect,            ///< Не корректный пароль пользователя
    dsUserNotExists,                    ///< Пользователь не существует
    dsUserAlreadyExists,                ///< Пользователь уже существует
    dsUserUUIDCorrupted,                ///< UUID пользователя повреждён
    dsUserRegistrationDateCorrupted,    ///< Время регистрации пользователя поврежено
    dsUserLoginCorrupted,               ///< Логин пользователя повреждён
    dsUserPasswordHashCorrupted,        ///< Хеш пароля пользователя повреждён
    dsUserNameCorrupted,                ///< Имя пользователя повреждено
    dsUserSexCorrupted,                 ///< Пол пользователя поврежден
    dsUserBirthdayCorrupted,            ///< День рождения пользователя повреждён
    dsUserContactsCorrupted,            ///< Контакты пользователя повреждены
    dsUserGroupsCorrupted,              ///< Группы пользователя повреждены
    dsUserContactRelationAlredyExists,  ///< Связь пользователь-контакт уже существует
    dsUserContactRelationNotExists,     ///< Связь пользователь-контакт не существует
    dsUserContactAlredyExists,          ///< Контакт пользователя уже существует
    dsUserContactNotExists,             ///< Контакт пользователя не существует
    dsUserGroupsRelationAlredyExists,   ///< Связь пользователь-группы уже существует
    dsUserGroupsRelationNotExists,      ///< Связь пользователь-группы не существует

    // Группы
    dsGroupUUIDAlreadyRegistered,       ///< В хранилище уже зарегистрирована группа с таким UUID
    dsGroupNotExists,                   ///< Группа не существует
    dsGroupAlreadyExists,               ///< Группа уже существует
    dsGroupUUIDCorrupted,               ///< UUID группы повреждён
    dsGroupRegistrationDateCorrupted,   ///< Время группы поврежено
    dsGroupNameCorrupted,               ///< Имя группы повреждено
    dsGroupUsersCorrupted,              ///< Участники группы повреждены
    dsGroupUserRelationAlredyExists,    ///< Связь группа-пользователь уже существует
    dsGroupUserRelationNotExists,       ///< Связь группа-пользователь не существует

    // Собщения
    dsMessageNotExists,                 ///< Сообщение не существует
    dsMessageAlreadyExists,             ///< Сообщение уже существует
    dsMessageUUIDCorrupted,             ///< UUID сообщения повреждён
    dsMessageGroupUUIDCorrupted,        ///< UUID группы сообщения повреждён
    dsMessageRegistrationDateCorrupted, ///< Время сообщения поврежено
    dsMessageTypeCorrupted,             ///< Тип сообщения повреждён
    dsMessageDataCorrupted,             ///< Данные сообщения повреждены

    // Связи
//    dsRelationsCorrupted,               ///< ВСЕ связи повреждены

    // Связи пользователь-контакт
//    dsRelationUCAlreadyExists,          ///< Связь пользвоатель-контакты уже существует
//    dsRelationUCNotExists,              ///< Связь пользвоатель-контакты не существует
//    dsRelationUCContactAlredyExists,    ///< Контакт уже существует в связи пользвоатель-контакты
//    dsRelationUCContactNotExists,       ///< Контакт не существует в связи пользвоатель-контакты
//    dsRelationUCCorrupted,              ///< Связи пользвоатель-контакты повреждены
//    dsRelationUCUserUUIDCorrupted,      ///< В связи пользователь-контакты повреждён UUID пользователя
//    dsRelationUCContactsCorrupted,      ///< В связи пользователь-контакты повреждён список контактов
//    dsRelationUCContactUUIDCorrupted,   ///< В связи пользователь-контакты повреждён UUID контакта

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
} // namespace hmservcommon::datastorage
//-----------------------------------------------------------------------------
namespace std
{
// Сообщаем метапрограммированию C++ 11 STL, что enum
// зарегистрирован в стандартной системе кодов ошибок
template <> struct is_error_code_enum<hmservcommon::datastorage::eDataStorageError> : true_type
{};
}
//-----------------------------------------------------------------------------
/**
 * @brief make_error_code - Перегрузка глобальной функции make_error_code () нашем пользовательским перечислением.
 * @param inErrCode - Код ошибки
 * @return Вернёт сформированный экемпляр std::error_code
 */
std::error_code make_error_code(hmservcommon::datastorage::eDataStorageError inErrCode);
//-----------------------------------------------------------------------------

#endif // DATASTORAGEERRORCATEGORY_H
