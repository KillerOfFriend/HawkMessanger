#include "jsondatastorage.h"

#include <string>
#include <fstream>
#include <algorithm>

#include <HawkLog.h>
#include <systemerrorex.h>

#include "datastorageerrorcategory.h"

#include <QCryptographicHash>

using namespace hmservcommon;

//-----------------------------------------------------------------------------
static const std::string FORMAT_VESION      = "0.0.0.1";
//-----------------------------------------------------------------------------
static const std::string J_VERSION          = "VERSION";
static const std::string J_USERS            = "USERS";
static const std::string J_GROUPS           = "GROUPS";

static const std::string J_UUID             = "UUID";
static const std::string J_REGDATE          = "registration_date";

static const std::string J_USER_LOGIN       = "login";
static const std::string J_USER_PASS        = "password_hash";
static const std::string J_USER_NAME        = "name";
static const std::string J_USER_SEX         = "sex";
static const std::string J_USER_BIRTHDAY    = "birthday";

static const std::string J_GROUP_NAME       = "name";
static const std::string J_GROUP_USERS      = "users";
//-----------------------------------------------------------------------------
/**
 * @brief jsonToByteArr - Функция преобразует JSON в QByteArray
 * @param inJson - Обрабатываемый JSON
 * @return Вернёт байтовую последовательность
 */
QByteArray jsonToByteArr(const nlohmann::json& inJson)
{
    QByteArray Result;

    if (inJson.is_null() || inJson.type() != nlohmann::json::value_t::array)
        Result = QByteArray();
    else
    {
        std::vector<std::byte> ByteVector = inJson.get<std::vector<std::byte>>();
        Result = QByteArray(reinterpret_cast<const char*>(ByteVector.data()), ByteVector.size());
    }

    return Result;
}
//-----------------------------------------------------------------------------
/**
 * @brief ByteArrToJson - Функция преобразует QByteArray в JSON
 * @param inByteArr - Обрабатываемый QByteArray
 * @return Вернёт JSON array содержащий байтовую последоваельность
 */
nlohmann::json ByteArrToJson(const QByteArray& inByteArr)
{
    nlohmann::json Result = nlohmann::json::value_type::array();

    std::vector<std::byte> ByteVector;
    ByteVector.assign(reinterpret_cast<const std::byte*>(inByteArr.data()), reinterpret_cast<const std::byte*>(inByteArr.data()) + inByteArr.size());
    Result = ByteVector;

    return Result;
}
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
HMJsonDataStorage::HMJsonDataStorage(const std::filesystem::path &inJsonPath) :
    HMDataStorage(),
    m_jsonPath(inJsonPath)
{

}
//-----------------------------------------------------------------------------
HMJsonDataStorage::~HMJsonDataStorage()
{
    close();
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::open()
{
    std::error_code Error = make_error_code(eDataStoragError::dsSuccess); // Изначально метим как успех

    close();

    if (std::filesystem::is_directory(m_jsonPath, Error))
    {
        if (!Error) // Не зарегестрирована внутренняя ошибка filesystem
            Error = make_error_code(hmcommon::eSystemErrorEx::seObjectNotFile); // Задаём свою (Путь указывает не на является файлом)
    }
    else // Объект файл
    {
        if (!std::filesystem::exists(m_jsonPath, Error)) // Проверяем что файл вообще существует
        {
            if (!Error) // Не зарегестрирована внутренняя ошибка filesystem
                makeDefault(); // Создадим пустой файл
        }
        else // Файл существует
        {
            std::ifstream inFile(m_jsonPath, std::ios_base::in);

            if (!inFile.is_open())
                Error = make_error_code(hmcommon::eSystemErrorEx::seOpenFileFail);
            else // Если файл успешно открылся
            {
                m_json = nlohmann::json::parse(inFile, nullptr, false);

                if (m_json.is_discarded()) // Если при парсинге произошла ошибка
                {
                    Error = make_error_code(hmcommon::eSystemErrorEx::seReadFileFail);
                    m_json.clear();
                }
                else
                    Error = checkCorrectStruct(); // Проверяем корректность считанной структуры

                inFile.close();
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
bool HMJsonDataStorage::is_open() const
{
    return !m_json.is_null();
}
//-----------------------------------------------------------------------------
void HMJsonDataStorage::close()
{
    if (is_open()) // Только при "открытом файле"
    {
        std::error_code Error = write(); // Вызываем запись

        if (Error)
            LOG_ERROR(QString::fromStdString(Error.message()));

        m_json = nlohmann::json(); // Очищаем хранилище
    }
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::addUser(const std::shared_ptr<hmcommon::HMUser> inUser)
{
    std::error_code Error = make_error_code(eDataStoragError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStoragError::dsNotOpen);
    else
    {
        if (!inUser) // Работаем только с валидным указателем
            Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
        else
        {
            if (findUserByUUID(inUser->m_uuid, Error)) // Если пользователь с таким UUID уже существует
                Error = make_error_code(eDataStoragError::dsUserAlreadyExists);
            else // Нет такого пользователя
            {   // Будем добавлять
                nlohmann::json NewUser = userToJson(inUser, Error); // Формируем объект пользователя

                if (!Error) // Если объект сформирован корректно
                    m_json[J_USERS].push_back(NewUser); // Добавляем пользователя
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::updateUser(const std::shared_ptr<hmcommon::HMUser> inUser)
{
    std::error_code Error = make_error_code(eDataStoragError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStoragError::dsNotOpen);
    else
    {
        if (!inUser) // Работаем только с валидным указателем
            Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
        else
        {
            const std::string UUID = inUser->m_uuid.toString().toStdString(); // Единоразово запоминаем UUID
            auto UserIt = std::find_if(m_json[J_USERS].begin(), m_json[J_USERS].end(), [&](const nlohmann::json& UserObject)
            {
                std::error_code Error = checkUser(UserObject); // Проверяем валидность объекта пользователя
                if (Error)
                {
                    LOG_WARNING(QString::fromStdString(Error.message()));
                    return false; // Повреждённый пользователь игнорируется
                }
                else
                    return UserObject[J_UUID].get<std::string>() == UUID; // Сравниваем UUID пользователя с заданым
            });

            if (UserIt == m_json[J_USERS].end()) // Если пользователь не найден
                Error = make_error_code(eDataStoragError::dsUserNotExists);
            else // Пользователь найден
            {
                nlohmann::json UpdateUser = userToJson(inUser, Error); // Формируем объект пользователя

                if (!Error) // Если объект сформирован корректно
                    *UserIt = UpdateUser; // Обновляем данные пользователя
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMUser> HMJsonDataStorage::findUserByUUID(const QUuid &inUserUUID, std::error_code &outErrorCode)
{
    std::shared_ptr<hmcommon::HMUser> Result = nullptr;
    outErrorCode = make_error_code(eDataStoragError::dsSuccess); // Изначально метим как успех

    if (!is_open())
        outErrorCode = make_error_code(eDataStoragError::dsNotOpen);
    else
    {
        const std::string UUID = inUserUUID.toString().toStdString(); // Единоразово запоминаем UUID
        // Ищим пользователя
        auto UserIt = std::find_if(m_json[J_USERS].cbegin(), m_json[J_USERS].cend(), [&](const nlohmann::json& UserObject)
        {
            std::error_code Error = checkUser(UserObject); // Проверяем валидность объекта пользователя
            if (Error)
            {
                LOG_WARNING(QString::fromStdString(Error.message()));
                return false; // Повреждённый пользователь игнорируется
            }
            else
                return UserObject[J_UUID].get<std::string>() == UUID; // Сравниваем UUID пользователя с заданым
        });

        if (UserIt == m_json[J_USERS].cend()) // Если пользователь не найден
            outErrorCode = make_error_code(eDataStoragError::dsUserNotExists);
        else // Пользователь найден
            Result = jsonToUser(*UserIt, outErrorCode); // Преобразуем JSON объект в пользователя
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMUser> HMJsonDataStorage::findUserByAuthentication(const QString &inLogin, const QByteArray &inPasswordHash, std::error_code &outErrorCode)
{
    std::shared_ptr<hmcommon::HMUser> Result = nullptr;
    outErrorCode = make_error_code(eDataStoragError::dsSuccess); // Изначально метим как успех

    if (!is_open())
        outErrorCode = make_error_code(eDataStoragError::dsNotOpen);
    else
    {
        const std::string Login = inLogin.toStdString(); // Единоразово запоминаем Login
        // Ищим пользователя
        auto UserIt = std::find_if(m_json[J_USERS].cbegin(), m_json[J_USERS].cend(), [&](const nlohmann::json& UserObject)
        {
            std::error_code Error = checkUser(UserObject); // Проверяем валидность объекта пользователя
            if (Error)
            {
                LOG_WARNING(QString::fromStdString(Error.message()));
                return false; // Повреждённый пользователь игнорируется
            }
            else
            {
                return UserObject[J_USER_LOGIN].get<std::string>() == Login &&      // Сравниваем Login пользователя с заданым
                        jsonToByteArr(UserObject[J_USER_PASS]) == inPasswordHash;   // Срваниваем PasswordHash с заданным
            }
        });

        if (UserIt == m_json[J_USERS].cend()) // Если пользователь не найден
            outErrorCode = make_error_code(eDataStoragError::dsUserNotExists);
        else // Пользователь найден
            Result = jsonToUser(*UserIt, outErrorCode); // Преобразуем JSON объект в пользователя
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::removeUser(const QUuid& inUserUUID)
{
    std::error_code Error = make_error_code(eDataStoragError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStoragError::dsNotOpen);
    else
    {
        const std::string UUID = inUserUUID.toString().toStdString(); // Единоразово запоминаем UUID
        // Ищим пользователя
        auto UserIt = std::find_if(m_json[J_USERS].cbegin(), m_json[J_USERS].cend(), [&](const nlohmann::json& UserObject)
        {
            std::error_code Error = checkUser(UserObject); // Проверяем валидность объекта пользователя
            if (Error)
            {
                LOG_WARNING(QString::fromStdString(Error.message()));
                return false; // Повреждённый пользователь игнорируется
            }
            else
                return UserObject[J_UUID].get<std::string>() == UUID; // Сравниваем UUID пользователя с заданым
        });

        if (UserIt != m_json[J_USERS].cend()) // Если пользователь существует
            m_json[J_USERS].erase(UserIt); // Удаляем пользователя
        // Если не найден пользователь на удаление то это не ошибка
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::addGroup(const std::shared_ptr<hmcommon::HMGroup> inGroup)
{
    std::error_code Error = make_error_code(eDataStoragError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStoragError::dsNotOpen);
    else
    {
        if (!inGroup) // Работаем только с валидным указателем
            Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
        else
        {
            if (findGroupByUUID(inGroup->m_uuid, Error)) // Если группа с таким UUID уже существует
                Error = make_error_code(eDataStoragError::dsGroupAlreadyExists);
            else // Нет такого пользователя
            {   // Будем добавлять
                nlohmann::json NewGroup = groupToJson(inGroup, Error); // Формируем объект группы

                if (!Error) // Если объект сформирован корректно
                    m_json[J_GROUPS].push_back(NewGroup); // Добавляем группу
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::updateGroup(const std::shared_ptr<hmcommon::HMGroup> inGroup)
{
    std::error_code Error = make_error_code(eDataStoragError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStoragError::dsNotOpen);
    else
    {
        if (!inGroup) // Работаем только с валидным указателем
            Error = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
        else
        {
            const std::string UUID = inGroup->m_uuid.toString().toStdString(); // Единоразово запоминаем UUID
            auto UserIt = std::find_if(m_json[J_GROUPS].begin(), m_json[J_GROUPS].end(), [&](const nlohmann::json& GroupObject)
            {
                std::error_code Error = checkGroup(GroupObject); // Проверяем валидность объекта группы
                if (Error)
                {
                    LOG_WARNING(QString::fromStdString(Error.message()));
                    return false; // Повреждённая группа игнорируется
                }
                else
                    return GroupObject[J_UUID].get<std::string>() == UUID; // Сравниваем UUID группы с заданым
            });

            if (UserIt == m_json[J_GROUPS].end()) // Если группа не найдена
                Error = make_error_code(eDataStoragError::dsGroupNotExists);
            else // Пользователь найден
            {
                nlohmann::json UpdateGroup = groupToJson(inGroup, Error); // Формируем объект группы

                if (!Error) // Если объект сформирован корректно
                    *UserIt = UpdateGroup; // Обновляем данные группы
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMGroup> HMJsonDataStorage::findGroupByUUID(const QUuid &inGroupUUID, std::error_code &outErrorCode)
{
    std::shared_ptr<hmcommon::HMGroup> Result = nullptr;
    outErrorCode = make_error_code(eDataStoragError::dsSuccess); // Изначально метим как успех

    if (!is_open())
        outErrorCode = make_error_code(eDataStoragError::dsNotOpen);
    else
    {
        const std::string UUID = inGroupUUID.toString().toStdString(); // Единоразово запоминаем UUID
        // Ищим пользователя
        auto GroupIt = std::find_if(m_json[J_GROUPS].cbegin(), m_json[J_GROUPS].cend(), [&](const nlohmann::json& GroupObject)
        {
            std::error_code Error = checkGroup(GroupObject); // Проверяем валидность объекта группы
            if (Error)
            {
                LOG_WARNING(QString::fromStdString(Error.message()));
                return false; // Повреждённая группа игнорируется
            }
            else
                return GroupObject[J_UUID].get<std::string>() == UUID; // Сравниваем UUID группы с заданым
        });

        if (GroupIt == m_json[J_GROUPS].cend()) // Если группа не найдена
            outErrorCode = make_error_code(eDataStoragError::dsGroupNotExists);
        else // Пользователь найден
            Result = jsonToGroup(*GroupIt, outErrorCode); // Преобразуем JSON объект в группы
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::removeGroup(const QUuid& inGroupUUID)
{
    std::error_code Error = make_error_code(eDataStoragError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStoragError::dsNotOpen);
    else
    {
        const std::string UUID = inGroupUUID.toString().toStdString(); // Единоразово запоминаем UUID
        // Ищим пользователя
        auto UserIt = std::find_if(m_json[J_GROUPS].cbegin(), m_json[J_GROUPS].cend(), [&](const nlohmann::json& GroupObject)
        {
            std::error_code Error = checkGroup(GroupObject); // Проверяем валидность объекта группы
            if (Error)
            {
                LOG_WARNING(QString::fromStdString(Error.message()));
                return false; // Повреждённая группа игнорируется
            }
            else
                return GroupObject[J_UUID].get<std::string>() == UUID; // Сравниваем UUID группы с заданым
        });

        if (UserIt != m_json[J_GROUPS].cend()) // Если группа существует
            m_json[J_GROUPS].erase(UserIt); // Удаляем группу
        // Если не найдена группа на удаление то это не ошибка
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::makeDefault()
{
    close();

    m_json[J_VERSION] = FORMAT_VESION;          // Задаём версию формата
    m_json[J_USERS] = nlohmann::json::array();  // Формируем пользователей

    nlohmann::json AdminUser = nlohmann::json::value_t::object;
    // Создаём пользователя "адиминистратора"
    AdminUser[J_UUID] = QUuid::createUuid().toString().toStdString();
    AdminUser[J_REGDATE] = QDateTime::currentDateTime().toString().toStdString();
    AdminUser[J_USER_LOGIN] = "Admin";

    QByteArray Hash = QCryptographicHash::hash(QString("password").toLocal8Bit(), QCryptographicHash::Md5);
    AdminUser[J_USER_PASS] = ByteArrToJson(Hash);

    AdminUser[J_USER_NAME] = "Admin";
    AdminUser[J_USER_SEX] = hmcommon::eSex::SNotSpecified;
    AdminUser[J_USER_BIRTHDAY] = "";

    m_json[J_USERS].push_back(AdminUser);


    m_json[J_GROUPS] = nlohmann::json::array(); // Формируем группы

    return write(); // Пишем сформированный файл
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::checkCorrectStruct() const
{
    std::error_code Error = make_error_code(eDataStoragError::dsSuccess); // Изначально метим как успех

    if (!is_open()) // Хранилище должно быть открыто
        Error = make_error_code(eDataStoragError::dsNotOpen);
    else
    {
        // Проверяем версию
        if (m_json.find(J_VERSION) == m_json.end() || m_json[J_VERSION].is_null() || m_json[J_VERSION].type() != nlohmann::json::value_t::string)
            Error = make_error_code(hmcommon::eSystemErrorEx::seIncorrecVersion);

        if (m_json.find(J_USERS) == m_json.end() || m_json[J_USERS].is_null() || m_json[J_USERS].type() != nlohmann::json::value_t::array)
            Error = make_error_code(hmcommon::eSystemErrorEx::seIncorretData);
        else
            for (auto& User : m_json[J_USERS].items()) // Перебиреам пользователей группы
            {
                Error = checkUser(User.value());
                if (Error)
                    break;
            }

        if (m_json.find(J_GROUPS) == m_json.end() || m_json[J_GROUPS].is_null() || m_json[J_GROUPS].type() != nlohmann::json::value_t::array)
            Error = make_error_code(hmcommon::eSystemErrorEx::seIncorretData);
        else
            for (auto& Group : m_json[J_GROUPS].items()) // Перебиреам пользователей группы
            {
                Error = checkGroup(Group.value());
                if (Error)
                    break;
            }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::write() const
{
    std::error_code Error = make_error_code(eDataStoragError::dsSuccess); // Изначально метим как успех

    if (std::filesystem::is_directory(m_jsonPath, Error))
    {
        if (!Error) // Не зарегестрирована внутренняя ошибка filesystem
            Error = make_error_code(hmcommon::eSystemErrorEx::seObjectNotFile); // Задаём свою (Путь указывает не на является файлом)
    }
    else // Объект файл
    {
        std::ofstream outFile(m_jsonPath, std::ios_base::out);
        outFile << m_json; // Пишем JSON в файл
        outFile.flush();
        outFile.close();
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::checkUser(const nlohmann::json& inUserObject) const
{
    std::error_code Error = make_error_code(eDataStoragError::dsSuccess); // Изначально метим как успех

    if (inUserObject.find(J_UUID) == inUserObject.end() || inUserObject[J_UUID].is_null() || inUserObject[J_UUID].type() != nlohmann::json::value_t::string)
        Error = make_error_code(eDataStoragError::dsUserUUIDCorrupted);

    if (inUserObject.find(J_REGDATE) == inUserObject.end() || inUserObject[J_REGDATE].is_null() || inUserObject[J_REGDATE].type() != nlohmann::json::value_t::string)
        Error = make_error_code(eDataStoragError::dsUserRegistrationDateCorrupted);

    if (inUserObject.find(J_USER_LOGIN) == inUserObject.end() || inUserObject[J_USER_LOGIN].is_null() || inUserObject[J_USER_LOGIN].type() != nlohmann::json::value_t::string)
        Error = make_error_code(eDataStoragError::dsUserLoginCorrupted);

    if (inUserObject.find(J_USER_PASS) == inUserObject.end() || inUserObject[J_USER_PASS].is_null() || inUserObject[J_USER_PASS].type() != nlohmann::json::value_t::array)
        Error = make_error_code(eDataStoragError::dsUserPasswordHashCorrupted);

    if (inUserObject.find(J_USER_NAME) == inUserObject.end() || inUserObject[J_USER_NAME].is_null() || inUserObject[J_USER_NAME].type() != nlohmann::json::value_t::string)
        Error = make_error_code(eDataStoragError::dsUserNameCorrupted);

    if (inUserObject.find(J_USER_SEX) == inUserObject.end() || inUserObject[J_USER_SEX].is_null() || inUserObject[J_USER_SEX].type() != nlohmann::json::value_t::number_unsigned)
        Error = make_error_code(eDataStoragError::dsUserSexCorrupted);

    if (inUserObject.find(J_USER_BIRTHDAY) == inUserObject.end() || /*inUserObject[J_USER_BIRTHDAY].is_null() ||*/ inUserObject[J_USER_BIRTHDAY].type() != nlohmann::json::value_t::string)
        Error = make_error_code(eDataStoragError::dsUserBirthday);

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::checkGroup(const nlohmann::json& inGroupObject) const
{
    std::error_code Error = make_error_code(eDataStoragError::dsSuccess); // Изначально метим как успех

    if (inGroupObject.find(J_UUID) == inGroupObject.end() || inGroupObject[J_UUID].is_null() || inGroupObject[J_UUID].type() != nlohmann::json::value_t::string)
        Error = make_error_code(eDataStoragError::dsGroupUUIDCorrupted);

    if (inGroupObject.find(J_REGDATE) == inGroupObject.end() || inGroupObject[J_REGDATE].is_null() || inGroupObject[J_REGDATE].type() != nlohmann::json::value_t::string)
        Error = make_error_code(eDataStoragError::dsGroupRegistrationDateCorrupted);

    if (inGroupObject.find(J_GROUP_NAME) == inGroupObject.end() || inGroupObject[J_GROUP_NAME].is_null() || inGroupObject[J_GROUP_NAME].type() != nlohmann::json::value_t::string)
        Error = make_error_code(eDataStoragError::dsGroupNameCorrupted);

    if (inGroupObject.find(J_GROUP_USERS) == inGroupObject.end() || inGroupObject[J_GROUP_USERS].is_null() || inGroupObject[J_GROUP_USERS].type() != nlohmann::json::value_t::array)
        Error = make_error_code(eDataStoragError::dsGroupUsersCorrupted);
    else // Список пользователей группы валиден
    {
        for (auto& UserUUID : inGroupObject[J_GROUP_USERS].items())
        {   // Перебираем всех пользователй группы
            if (!UserUUID.value().is_null() && UserUUID.value().type() != nlohmann::json::value_t::string)
            {   // Если хоть один повреждён, метим группу как повреждённую
                Error = make_error_code(eDataStoragError::dsGroupUsersCorrupted);
                break;
            }
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMUser> HMJsonDataStorage::jsonToUser(const nlohmann::json& inUserObject, std::error_code& outErrorCode) const
{
    std::shared_ptr<hmcommon::HMUser> Result = nullptr;
    outErrorCode = checkUser(inUserObject); // Проверяем валидность пользователя

    if (!outErrorCode) // Если объект валиден
    {   // Инициализируем экземпляр класса пользователя
        Result = std::make_shared<hmcommon::HMUser>(QUuid::fromString(QString::fromStdString(inUserObject[J_UUID].get<std::string>())),
                                                    QDateTime::fromString(QString::fromStdString(inUserObject[J_REGDATE].get<std::string>())));

        Result->setLogin(QString::fromStdString(inUserObject[J_USER_LOGIN].get<std::string>()));
        Result->setPasswordHash(jsonToByteArr(inUserObject[J_USER_PASS]));
        Result->setName(QString::fromStdString(inUserObject[J_USER_NAME].get<std::string>()));
        Result->setSex(inUserObject[J_USER_SEX].get<hmcommon::eSex>());
        Result->setBirthday(QDate::fromString(QString::fromStdString(inUserObject[J_USER_BIRTHDAY].get<std::string>())));
    }

    return Result;
}
//-----------------------------------------------------------------------------
nlohmann::json HMJsonDataStorage::userToJson(std::shared_ptr<hmcommon::HMUser> inUser, std::error_code& outErrorCode) const
{
    nlohmann::json Result = nlohmann::json::value_type::object();
    outErrorCode = make_error_code(hmcommon::eSystemErrorEx::seSuccess);

    if (!inUser) // Проверяем валидность указателя
        outErrorCode = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
    else // Объект валиден
    {
        Result[J_UUID] = inUser->m_uuid.toString().toStdString();
        Result[J_REGDATE] = inUser->m_registrationDate.toString().toStdString();
        Result[J_USER_LOGIN] = inUser->getLogin().toStdString();
        Result[J_USER_PASS] = ByteArrToJson(inUser->getPasswordHash());
        Result[J_USER_NAME] = inUser->getName().toStdString();
        Result[J_USER_SEX] = static_cast<std::uint32_t>(inUser->getSex());
        Result[J_USER_BIRTHDAY] = inUser->getBirthday().toString().toStdString();

        outErrorCode = checkUser(Result); // Заранее проверяем корректность создаваемого пользователя

        if (outErrorCode)
            Result.clear();
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMGroup> HMJsonDataStorage::jsonToGroup(const nlohmann::json& inGroupObject, std::error_code& outErrorCode) const
{
    std::shared_ptr<hmcommon::HMGroup> Result = nullptr;
    outErrorCode = checkGroup(inGroupObject); // Проверяем валидность группы

    if (!outErrorCode) // Если объект валиден
    {
        Result = std::make_shared<hmcommon::HMGroup>(QUuid::fromString(QString::fromStdString(inGroupObject[J_UUID].get<std::string>())),
                                                     QDateTime::fromString(QString::fromStdString(inGroupObject[J_REGDATE].get<std::string>())));

        Result->setName(QString::fromStdString(inGroupObject[J_GROUP_NAME].get<std::string>()));

        for (auto& UserUUID : inGroupObject[J_GROUP_USERS].items()) // Перебиреам пользователей группы
            Result->addUser(QUuid::fromString(QString::fromStdString(UserUUID.value().get<std::string>()))); // Добавляем пользователя в группу
    }

    return Result;
}
//-----------------------------------------------------------------------------
nlohmann::json HMJsonDataStorage::groupToJson(std::shared_ptr<hmcommon::HMGroup> inGroup, std::error_code& outErrorCode) const
{
    nlohmann::json Result = nlohmann::json::value_type::object();
    outErrorCode = make_error_code(hmcommon::eSystemErrorEx::seSuccess);

    if (!inGroup) // Проверяем валидность указателя
        outErrorCode = make_error_code(hmcommon::eSystemErrorEx::seInvalidPtr);
    else // Объект валиден
    {
        Result[J_UUID] = inGroup->m_uuid.toString().toStdString();
        Result[J_REGDATE] = inGroup->m_registrationDate.toString().toStdString();
        Result[J_GROUP_NAME] = inGroup->getName().toStdString();
        Result[J_GROUP_USERS] = nlohmann::json::value_type::array();

        for (std::size_t Index = 0; Index < inGroup->usersCount(); ++Index)
            Result[J_GROUP_USERS].push_back(inGroup->getUser(Index, outErrorCode).toString().toStdString());

        outErrorCode = checkGroup(Result); // Заранее проверяем корректность создаваемой группы

        if (outErrorCode)
            Result.clear();
    }

    return Result;
}
//-----------------------------------------------------------------------------
