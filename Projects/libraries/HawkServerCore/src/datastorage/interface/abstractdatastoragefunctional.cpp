#include "abstractdatastoragefunctional.h"

#include "systemerrorex.h"
#include "datastorage/datastorageerrorcategory.h"
#include "datastorage/cachedmemorydatastorage/cachedmemorydatastorage.h"

using namespace hmservcommon::datastorage;

//-----------------------------------------------------------------------------
HMAbstractDataStorageFunctional::HMAbstractDataStorageFunctional() :
    HMDataStorage() // Инициализируем предок-интерфейс
{

}
//-----------------------------------------------------------------------------
hmcommon::error_code HMAbstractDataStorageFunctional::checkNewUserUnique(const std::shared_ptr<hmcommon::HMUserInfo> inUser) const
{   // ДАННЫЙ МЕТОД НЕ ДОЛЖЕН ПРОВЕРЯТЬ КЕШ!
    hmcommon::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        if (!inUser) // Работаем только с валидным указателем
            Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
        else
        {
            // Нужно проверить уникальность создаваемого пользователя
            // I) UUID должен быть уникальным
            // II) Логин должен быть уникальным

            hmcommon::error_code ErrorFindByUUID = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех
            std::shared_ptr<hmcommon::HMUserInfo> FindUUIDRes = findUserByUUID(inUser->m_uuid, ErrorFindByUUID); // Ищим пользователя по UUID

            // I) findUserByUUID Вернёт либо:
            // 1) Пользователь найден == Success из любой категории (функция должна вернуть eDataStorageError::dsUserAlreadyExists)
            // 2) Пользователь не найден (функция должна вернуть dlSuccess)
            // 3) Одна из многих ошибок парсинга (функция должна вернуть эту ошибку)

            hmcommon::error_code ErrorFindByAutData = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех
            std::shared_ptr<hmcommon::HMUserInfo> FindAutDataRes = findUserByAuthentication(inUser->getLogin(), inUser->getPasswordHash(), ErrorFindByAutData); // Ищим пользователя по данным аутентификации

            // II) findUserByAuthentication Вернёт либо:
            // 1) Пользователь не найден (функция должна вернуть dlSuccess)
            // 2) Не верный пароль (функция должна вернуть eDataStorageError::dsUserLoginAlreadyRegistered) ибо по логину проверка прошла
            // 3) Пользователь найден == Success из любой категории (функция должна вернуть eDataStorageError::dsUserLoginAlreadyRegistered)
            // 4) Одна из многих ошибок парсинга (функция должна вернуть эту ошибку)

            switch (ErrorFindByUUID.value()) // I
            {
                // Ситуация I (1)
                case 0: { Error = make_error_code(eDataStorageError::dsUserAlreadyExists); break; };
                // Ситуация I (2)
                case static_cast<int32_t>(eDataStorageError::dsUserNotExists):
                { Error = make_error_code(eDataStorageError::dsSuccess); break; }
                // Ситуация I (3)
                default: { Error = ErrorFindByUUID; break; }
            }

            if (!Error) // Если I не выявила ошибки
            {
                switch (ErrorFindByAutData.value()) // II
                {
                    // Ситуация II (1)
                    case static_cast<int32_t>(eDataStorageError::dsUserNotExists):
                    { Error = make_error_code(eDataStorageError::dsSuccess); break; }
                    // Ситуация II (2,3)
                    case 0:
                    case static_cast<int32_t>(eDataStorageError::dsUserPasswordIncorrect):
                    { Error = make_error_code(eDataStorageError::dsUserLoginAlreadyRegistered); break; }
                    // Ситуация II (4)
                    default: { Error = ErrorFindByAutData; break; }
                }
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
hmcommon::error_code HMAbstractDataStorageFunctional::checkNewGroupUnique(const std::shared_ptr<hmcommon::HMGroupInfo> inGroup) const
{   // ДАННЫЙ МЕТОД НЕ ДОЛЖЕН ПРОВЕРЯТЬ КЕШ!
    hmcommon::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        if (!inGroup) // Работаем только с валидным указателем
            Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
        else
        {
            hmcommon::error_code ErrorFindByUUID = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех
            std::shared_ptr<hmcommon::HMGroupInfo> FindUUIDRes = findGroupByUUID(inGroup->m_uuid, ErrorFindByUUID); // Ищим группу по UUID

            // I) findGroupByUUID Вернёт либо:
            // 1) Группа найдена == Success из любой категории (функция должна вернуть eDataStorageError::dsGroupUUIDAlreadyRegistered)
            // 2) Группа не найдена (функция должна вернуть dlSuccess)
            // 3) Одна из многих ошибок парсинга (функция должна вернуть эту ошибку)

            switch (ErrorFindByUUID.value()) // I
            {
                // Ситуация I (1)
                case 0: { Error = make_error_code(eDataStorageError::dsGroupUUIDAlreadyRegistered); break; };
                // Ситуация I (2)
                case static_cast<int32_t>(eDataStorageError::dsGroupNotExists):
                { Error = make_error_code(eDataStorageError::dsSuccess); break; }
                // Ситуация I (3)
                default: { Error = ErrorFindByUUID; break; }
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
//hmcommon::error_code HMAbstractDataStorageFunctional::checkUserContactsUnique(const QUuid& inUserUUID, const std::shared_ptr<hmcommon::HMUserInfoList> inContacts) const
//{   // ДАННЫЙ МЕТОД НЕ ДОЛЖЕН ПРОВЕРЯТЬ КЕШ!
//    hmcommon::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

//    if (!is_open()) // Хранилище должно быть открыто
//        Error = make_error_code(eDataStorageError::dsNotOpen);
//    else
//    {
//        if (!inContacts) // Работаем только с валидным указателем
//            Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
//        else
//        {
//            hmcommon::error_code ErrorFindByUUID = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех
//            std::shared_ptr<hmcommon::HMUserInfoList> FindRes = getUserContactList(inUserUUID, ErrorFindByUUID); // Ищим связь по UUID

//            // I) getUserContactList Вернёт либо:
//            // 1) Связь найдена == Success из любой категории (функция должна вернуть eDataStorageError::dsRelationUCAlreadyExists)
//            // 2) Связь не найдена (функция должна вернуть dlSuccess)
//            // 3) Одна из многих ошибок парсинга (функция должна вернуть эту ошибку)

//            switch (ErrorFindByUUID.value()) // I
//            {
//                // Ситуация I (1)
//                case 0: { Error = make_error_code(eDataStorageError::dsRelationUCAlreadyExists); break; };
//                // Ситуация I (2)
//                case static_cast<int32_t>(eDataStorageError::dsRelationUCNotExists):
//                { Error = make_error_code(eDataStorageError::dsSuccess); break; }
//                // Ситуация I (3)
//                default: { Error = ErrorFindByUUID; break; }
//            }
//        }
//    }

//    return Error;
//}
//-----------------------------------------------------------------------------
