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
static const std::string J_USERS            = "USERS";
static const std::string J_GROUPS           = "GROUPS";

static const std::string J_UUID             = "UUID";
static const std::string J_USER_REGDATE     = "registration_date";
static const std::string J_USER_LOGIN       = "login";
static const std::string J_USER_PASS        = "password_hash";
static const std::string J_USER_NAME        = "name";
static const std::string J_USER_SEX         = "sex";
static const std::string J_USER_BIRTHDAY    = "birthday";
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
    std::error_code Error = make_error_code(DataStoragError::dsSuccess); // Изначально метим как успех

    close();

    if (std::filesystem::is_directory(m_jsonPath, Error))
    {
        if (!Error) // Не зарегестрирована внутренняя ошибка filesystem
            Error = make_error_code(hmcommon::SystemErrorEx::seObjectNotFile); // Задаём свою (Путь указывает не на является файлом)
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
                Error = make_error_code(hmcommon::SystemErrorEx::seOpenFileFail);
            else // Если файл успешно открылся
            {
                m_json = nlohmann::json::parse(inFile, nullptr, false);

                if (m_json.is_discarded()) // Если при парсинге произошла ошибка
                {
                    Error = make_error_code(hmcommon::SystemErrorEx::seReadFileFail);
                    m_json.clear();
                }

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

        m_json.clear(); // Очищаем хранилище
    }
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::addUser(const std::shared_ptr<hmcommon::HMUser> inUser)
{
    std::error_code Error = make_error_code(DataStoragError::dsSuccess); // Изначально метим как успех

    if (!inUser) // Работаем только с валидным указателем
        Error = make_error_code(hmcommon::SystemErrorEx::seInvalidPtr);
    else
    {
        auto ExistsIt = m_json[J_USERS].find(inUser->m_uuid.toString().toStdString()); // Проверяем, существует ли такой пользователь

        if (ExistsIt != m_json[J_USERS].end()) // Если такой пользователь уже существует
            Error = make_error_code(DataStoragError::dsUserAlreadyExists);
        else
        {
            //m_json[J_USERS]
        }
    }

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::updateUser(const std::shared_ptr<hmcommon::HMUser> inUser)
{
    std::error_code Error = make_error_code(DataStoragError::dsSuccess); // Изначально метим как успех

    return Error;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMUser> HMJsonDataStorage::findUserByUUID(const QUuid &inUserUUID, std::error_code &outErrorCode)
{
    std::shared_ptr<hmcommon::HMUser> Result = nullptr;
    outErrorCode = make_error_code(DataStoragError::dsSuccess); // Изначально метим как успех

    if (!is_open())
        outErrorCode = make_error_code(DataStoragError::dsNotOpen);
    else
    {
        const std::string UUID = inUserUUID.toString().toStdString(); // Единоразово запоминаем UUID
        // Ищим пользователя
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
            outErrorCode = make_error_code(DataStoragError::dsUserNotExists);
        else // Пользователь найден
        {   // Формируем новую сущьность "Пользователь"
            const nlohmann::json& User = *UserIt;
            // Инициализируем экземпляр класса пользователя
            Result = std::make_shared<hmcommon::HMUser>(QUuid::fromString(QString::fromStdString(User[J_UUID].get<std::string>())),
                                                        QDateTime::fromString(QString::fromStdString(User[J_USER_REGDATE].get<std::string>())));

            Result->setLogin(QString::fromStdString(User[J_USER_LOGIN].get<std::string>()));
            Result->setPasswordHash(jsonToByteArr(User[J_USER_PASS]));
            Result->setName(QString::fromStdString(User[J_USER_NAME].get<std::string>()));
            Result->setSex(User[J_USER_SEX].get<hmcommon::eSex>());
            Result->setBirthday(QDate::fromString(QString::fromStdString(User[J_USER_BIRTHDAY].get<std::string>())));
        }
    }

    return Result;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMUser> HMJsonDataStorage::findUserByAuthentication(const QString &inLogin, const QByteArray &inPasswordHash, std::error_code &outErrorCode)
{

}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::addGroup(const std::shared_ptr<hmcommon::HMGroup> inGroup)
{
    std::error_code Error = make_error_code(DataStoragError::dsSuccess); // Изначально метим как успех

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::updateGroup(const std::shared_ptr<hmcommon::HMGroup> inGroup)
{
    std::error_code Error = make_error_code(DataStoragError::dsSuccess); // Изначально метим как успех

    return Error;
}
//-----------------------------------------------------------------------------
std::shared_ptr<hmcommon::HMUser> HMJsonDataStorage::findGroupByUUID(const QUuid &inGroupUUID, std::error_code &outErrorCode)
{

}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::makeDefault()
{
    close();

    m_json[J_USERS] = nlohmann::json::array();

    nlohmann::json AdminUser = nlohmann::json::value_t::object;

    AdminUser[J_UUID] = QUuid::createUuid().toString().toStdString();
    AdminUser[J_USER_REGDATE] = QDateTime::currentDateTime().toString().toStdString();
    AdminUser[J_USER_LOGIN] = "Admin";

    QByteArray Hash = QCryptographicHash::hash(QString("password").toLocal8Bit(), QCryptographicHash::Md5);
    AdminUser[J_USER_PASS] = ByteArrToJson(Hash);

    AdminUser[J_USER_NAME] = "Admin";
    AdminUser[J_USER_SEX] = hmcommon::eSex::SNotSpecified;
    AdminUser[J_USER_BIRTHDAY] = "";

    m_json[J_USERS].push_back(AdminUser);


    m_json[J_GROUPS] = nlohmann::json::array();

    return write(); // Пишем сформированный файл
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::write() const
{
    std::error_code Error = make_error_code(DataStoragError::dsSuccess); // Изначально метим как успех

    if (std::filesystem::is_directory(m_jsonPath, Error))
    {
        if (!Error) // Не зарегестрирована внутренняя ошибка filesystem
            Error = make_error_code(hmcommon::SystemErrorEx::seObjectNotFile); // Задаём свою (Путь указывает не на является файлом)
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
    std::error_code Error = make_error_code(DataStoragError::dsSuccess); // Изначально метим как успех

    if (inUserObject.find(J_UUID) == inUserObject.end() || inUserObject[J_UUID].is_null() || inUserObject[J_UUID].type() != nlohmann::json::value_t::string)
        Error = make_error_code(DataStoragError::dsUserUUIDCorrupted);

    if (inUserObject.find(J_USER_REGDATE) == inUserObject.end() || inUserObject[J_USER_REGDATE].is_null() || inUserObject[J_USER_REGDATE].type() != nlohmann::json::value_t::string)
        Error = make_error_code(DataStoragError::dsUserRegistrationDate);

    if (inUserObject.find(J_USER_LOGIN) == inUserObject.end() || inUserObject[J_USER_LOGIN].is_null() || inUserObject[J_USER_LOGIN].type() != nlohmann::json::value_t::string)
        Error = make_error_code(DataStoragError::dsUserLoginCorrupted);

    if (inUserObject.find(J_USER_PASS) == inUserObject.end() || inUserObject[J_USER_PASS].is_null() || inUserObject[J_USER_PASS].type() != nlohmann::json::value_t::array)
        Error = make_error_code(DataStoragError::dsUserPasswordHashCorrupted);

    if (inUserObject.find(J_USER_NAME) == inUserObject.end() || inUserObject[J_USER_NAME].is_null() || inUserObject[J_USER_NAME].type() != nlohmann::json::value_t::string)
        Error = make_error_code(DataStoragError::dsUserNameCorrupted);

    if (inUserObject.find(J_USER_SEX) == inUserObject.end() || inUserObject[J_USER_SEX].is_null() || inUserObject[J_USER_SEX].type() != nlohmann::json::value_t::number_unsigned)
        Error = make_error_code(DataStoragError::dsUserSexCorrupted);

    if (inUserObject.find(J_USER_BIRTHDAY) == inUserObject.end() || /*inUserObject[J_USER_BIRTHDAY].is_null() ||*/ inUserObject[J_USER_BIRTHDAY].type() != nlohmann::json::value_t::string)
        Error = make_error_code(DataStoragError::dsUserBirthday);

    return Error;
}
//-----------------------------------------------------------------------------
std::error_code HMJsonDataStorage::checkGroup(const nlohmann::json& inGroupObject) const
{
    std::error_code Error = make_error_code(DataStoragError::dsSuccess); // Изначально метим как успех

    return Error;
}
//-----------------------------------------------------------------------------
