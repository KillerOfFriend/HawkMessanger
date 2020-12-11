#include "abstractdatastoragefunctional.h"

#include "systemerrorex.h"
#include "datastorage/datastorageerrorcategory.h"
#include "datastorage/cacheddatastorage/cacheddatastorage.h"

using namespace hmservcommon::datastorage;

//-----------------------------------------------------------------------------
HMAbstractDataStorageFunctional::HMAbstractDataStorageFunctional() :
    HMDataStorage() // Инициализируем предок-интерфейс
{

}
//-----------------------------------------------------------------------------
std::error_code HMAbstractDataStorageFunctional::checkNewUserUnique(const std::shared_ptr<hmcommon::HMUser> inUser) const
{   // ДАННЫЙ МЕТОД НЕ ДОЛЖЕН ПРОВЕРЯТЬ КЕШ!
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        if (!inUser) // Работаем только с валидным указателем
            Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
        else
        {
            std::error_code ErrorFindByUUID = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех
            std::shared_ptr<hmcommon::HMUser> FindUUIDRes = findUserByUUID(inUser->m_uuid, ErrorFindByUUID); // Ищим пользователя по UUID

            if (ErrorFindByUUID.value() != static_cast<int32_t>(eDataStorageError::dsUserNotExists)) // Если найден пользователь с таким UUID
                Error = make_error_code(eDataStorageError::dsUserUUIDAlreadyRegistered); // Не должно быть пользователей с одинаковым UUID'ом
            else
            {
                std::error_code ErrorFindByAutData = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех
                std::shared_ptr<hmcommon::HMUser> FindAutDataRes = findUserByAuthentication(inUser->getLogin(), inUser->getPasswordHash(), ErrorFindByAutData); // Ищим пользователя по данным аутентификации

                if (ErrorFindByAutData.value() != static_cast<int32_t>(eDataStorageError::dsUserNotExists)) // Если найден пользователь с такими аутентификационными данными уже найден
                    Error = make_error_code(eDataStorageError::dsUserLoginAlreadyRegistered); // Не должно быть пользователей с одинаковым логином
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMAbstractDataStorageFunctional::checkNewGroupUnique(const std::shared_ptr<hmcommon::HMGroup> inGroup) const
{   // ДАННЫЙ МЕТОД НЕ ДОЛЖЕН ПРОВЕРЯТЬ КЕШ!
    std::error_code Error = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStorageError::dsNotOpen);
    else
    {
        if (!inGroup) // Работаем только с валидным указателем
            Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
        else
        {
            std::error_code ErrorFindByUUID = make_error_code(eDataStorageError::dsSuccess); // Изначально метим как успех
            std::shared_ptr<hmcommon::HMGroup> FindUUIDRes = findGroupByUUID(inGroup->m_uuid, ErrorFindByUUID); // Ищим группу по UUID

            if (ErrorFindByUUID.value() != static_cast<int32_t>(eDataStorageError::dsGroupNotExists)) // Если найдена группа с таким UUID
                Error = make_error_code(eDataStorageError::dsGroupUUIDAlreadyRegistered); // Не должно быть групп с одинаковым UUID'ом

        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
