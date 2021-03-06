#ifndef DATASTORAGEERRORCATEGORY_H
#define DATASTORAGEERRORCATEGORY_H

/**
 * @file systemerrorex.h
 * @brief Содержит описание категории ошибок хранилища данных
 */

#include <string>

#include "errorcode.h"
#include "categoryconst.h"

namespace errors
{
//-----------------------------------------------------------------------------
/**
 * @brief The eDataStorageError enum - Перечень ошибок хранилища данных
 */
enum class eDataStorageError
{
    dsSuccess = C_SUCCESS,                          ///< 0 Не явялется ошибкой

    // Хранилище
    dsNotOpen = C_CATEGORY_DATASTORAGE_START,       ///< Хранилище не открыто

    // Пользователи
    dsUserLoginAlreadyRegistered,                   ///< В хранилище уже зарегистрирован пользователь с таким логином
    dsUserPasswordIncorrect,                        ///< Не корректный пароль пользователя
    dsUserNotExists,                                ///< Пользователь не существует
    dsUserAlreadyExists,                            ///< Пользователь уже существует
    dsUserUUIDCorrupted,                            ///< UUID пользователя повреждён
    dsUserRegistrationDateCorrupted,                ///< Время регистрации пользователя поврежено
    dsUserLoginCorrupted,                           ///< Логин пользователя повреждён
    dsUserPasswordHashCorrupted,                    ///< Хеш пароля пользователя повреждён
    dsUserNameCorrupted,                            ///< Имя пользователя повреждено
    dsUserSexCorrupted,                             ///< Пол пользователя поврежден
    dsUserBirthdayCorrupted,                        ///< День рождения пользователя повреждён
    dsUserContactsCorrupted,                        ///< Контакты пользователя повреждены
    dsUserGroupsCorrupted,                          ///< Группы пользователя повреждены
    dsUserContactRelationAlredyExists,              ///< Связь пользователь-контакт уже существует
    dsUserContactRelationNotExists,                 ///< Связь пользователь-контакт не существует
    dsUserContactAlredyExists,                      ///< Контакт пользователя уже существует
    dsUserContactNotExists,                         ///< Контакт пользователя не существует
    dsUserGroupsRelationAlredyExists,               ///< Связь пользователь-группы уже существует
    dsUserGroupsRelationNotExists,                  ///< Связь пользователь-группы не существует

    // Группы
    dsGroupUUIDAlreadyRegistered,                   ///< В хранилище уже зарегистрирована группа с таким UUID
    dsGroupNotExists,                               ///< Группа не существует
    dsGroupAlreadyExists,                           ///< Группа уже существует
    dsGroupUUIDCorrupted,                           ///< UUID группы повреждён
    dsGroupRegistrationDateCorrupted,               ///< Время группы поврежено
    dsGroupNameCorrupted,                           ///< Имя группы повреждено
    dsGroupUsersCorrupted,                          ///< Участники группы повреждены
    dsGroupUserRelationAlredyExists,                ///< Связь группа-пользователь уже существует
    dsGroupUserRelationNotExists,                   ///< Связь группа-пользователь не существует

    // Собщения
    dsMessageNotExists,                             ///< Сообщение не существует
    dsMessageAlreadyExists,                         ///< Сообщение уже существует
    dsMessageUUIDCorrupted,                         ///< UUID сообщения повреждён
    dsMessageGroupUUIDCorrupted,                    ///< UUID группы сообщения повреждён
    dsMessageRegistrationDateCorrupted,             ///< Время сообщения поврежено
    dsMessageTypeCorrupted,                         ///< Тип сообщения повреждён
    dsMessageDataCorrupted,                         ///< Данные сообщения повреждены

    dsCount                                         ///< Количество
};
//-----------------------------------------------------------------------------
/**
 * @brief The DataStorageErrorCategory class - Класс, определяющий категорию ошибко хранилища данных
 *
 * @authors Alekseev_s
 * @date 21.11.2020
 */
class DataStorageErrorCategory : public std::error_category
{
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
/**
 * @brief ConversionDataStorageError_category - Функцию, возвращающую статический экземпляр пользовательской категории
 * @return Вернёт статический экземпляр пользовательской категории
 */
extern inline const DataStorageErrorCategory &ConversionDataStorageError_category();
//-----------------------------------------------------------------------------
} // namespace errors
//-----------------------------------------------------------------------------
namespace std
{
// Сообщаем метапрограммированию C++ 11 STL, что enum
// зарегистрирован в стандартной системе кодов ошибок
template <> struct is_error_code_enum<errors::eDataStorageError> : true_type
{};
}
//-----------------------------------------------------------------------------
/**
 * @brief make_error_code - Перегрузка глобальной функции make_error_code () нашем пользовательским перечислением.
 * @param inErrCode - Код ошибки
 * @return Вернёт сформированный экемпляр errors::error_code
 */
errors::error_code make_error_code(errors::eDataStorageError inErrCode);
//-----------------------------------------------------------------------------

#endif // DATASTORAGEERRORCATEGORY_H
