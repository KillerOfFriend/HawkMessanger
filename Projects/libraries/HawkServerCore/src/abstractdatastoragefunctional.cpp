#include "abstractdatastoragefunctional.h"

#include "systemerrorex.h"
#include "datastorageerrorcategory.h"
#include "cacheddatastorage.h"

using namespace hmservcommon;

//-----------------------------------------------------------------------------
HMAbstractDataStorageFunctional::HMAbstractDataStorageFunctional(const bool inMakeCache) :
    HMDataStorage() // Инициализируем предок-интерфейс

{
    if (inMakeCache) // Если создание хеша разрешено
        m_cache = std::make_shared<HMCachedDataStorage>(); // Инициализируем хеширующее хранилище
}
//-----------------------------------------------------------------------------
std::shared_ptr<HMDataStorage> HMAbstractDataStorageFunctional::cache() const
{ return m_cache; }
//-----------------------------------------------------------------------------
std::error_code HMAbstractDataStorageFunctional::checkNewUserUnique(const std::shared_ptr<hmcommon::HMUser> inUser) const
{   // ДАННЫЙ МЕТОД НЕ ДОЛЖЕН ПРОВЕРЯТЬ КЕШ!
    std::error_code Error = make_error_code(eDataStoragError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStoragError::dsNotOpen);
    else
    {
        if (!inUser) // Работаем только с валидным указателем
            Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
        else
        {
            std::error_code ErrorFindByUUID = make_error_code(eDataStoragError::dsSuccess); // Изначально метим как успех
            std::shared_ptr<hmcommon::HMUser> FindUUIDRes = findUserByUUID(inUser->m_uuid, ErrorFindByUUID); // Ищим пользователя по UUID

            if (ErrorFindByUUID.value() != static_cast<int32_t>(eDataStoragError::dsUserNotExists)) // Если найден пользователь с таким UUID
                Error = make_error_code(eDataStoragError::dsUserUUIDAlreadyRegistered); // Не должно быть пользователей с одинаковым UUID'ом
            else
            {
                std::error_code ErrorFindByAutData = make_error_code(eDataStoragError::dsSuccess); // Изначально метим как успех
                std::shared_ptr<hmcommon::HMUser> FindAutDataRes = findUserByAuthentication(inUser->getLogin(), inUser->getPasswordHash(), ErrorFindByAutData); // Ищим пользователя по данным аутентификации

                if (ErrorFindByAutData.value() != static_cast<int32_t>(eDataStoragError::dsUserNotExists)) // Если найден пользователь с такими аутентификационными данными уже найден
                    Error = make_error_code(eDataStoragError::dsUserLoginAlreadyRegistered); // Не должно быть пользователей с одинаковым логином
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMAbstractDataStorageFunctional::checkNewGroupUnique(const std::shared_ptr<hmcommon::HMGroup> inGroup) const
{   // ДАННЫЙ МЕТОД НЕ ДОЛЖЕН ПРОВЕРЯТЬ КЕШ!
    std::error_code Error = make_error_code(eDataStoragError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStoragError::dsNotOpen);
    else
    {
        if (!inGroup) // Работаем только с валидным указателем
            Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
        else
        {
            std::error_code ErrorFindByUUID = make_error_code(eDataStoragError::dsSuccess); // Изначально метим как успех
            std::shared_ptr<hmcommon::HMGroup> FindUUIDRes = findGroupByUUID(inGroup->m_uuid, ErrorFindByUUID); // Ищим группу по UUID

            if (ErrorFindByUUID.value() != static_cast<int32_t>(eDataStoragError::dsGroupNotExists)) // Если найдена группа с таким UUID
                Error = make_error_code(eDataStoragError::dsGroupUUIDAlreadyRegistered); // Не должно быть групп с одинаковым UUID'ом

        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
