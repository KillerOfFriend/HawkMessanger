#ifndef HAWKCOMMONTESTUTILS_HPP
#define HAWKCOMMONTESTUTILS_HPP

#include <memory>

#include <QUuid>
#include <QString>
#include <QDateTime>

#include <HawkCommon.h>

//-----------------------------------------------------------------------------
namespace testscommon
{
//-----------------------------------------------------------------------------
/**
 * @brief make_user - Метод сформирует пользователя для тестирования
 * @param inUserUuid - UUID пользователя
 * @param inUserLogin - Логин пользователя
 * @param inUserPassword - Пароль пользователя
 * @param inCreateDate - Дата создания пользователя
 * @return Вернёт указатель на нового пользователя
 */
std::shared_ptr<hmcommon::HMUserInfo> make_user(const QUuid inUserUuid = QUuid::createUuid(), const QString inUserLogin = "UserLogin@login.com",
                                            const QString inUserPassword = "P@ssworOfUser123", const QDateTime inCreateDate = QDateTime::currentDateTime())//QDateTime(QDate(), QTime::currentTime()))
{
    // Формируем нового пользователя
    std::shared_ptr<hmcommon::HMUserInfo> NewUser = std::make_shared<hmcommon::HMUserInfo>(inUserUuid, inCreateDate);
    // Задаём основные параметры
    NewUser->setLogin(inUserLogin);
    NewUser->setPassword(inUserPassword);

    return NewUser;
}
//-----------------------------------------------------------------------------
/**
 * @brief make_group - Метод сформирует группу для тестирования
 * @param inGroupUuid - UUID группы
 * @param inGroupName - Имя группы
 * @param inCreateDate - Дата создания группы
 * @return Вернёт указатель на новую группу
 */
std::shared_ptr<hmcommon::HMGroupInfo> make_group(const QUuid inGroupUuid = QUuid::createUuid(), const QString inGroupName = "New group name",
                                              const QDateTime inCreateDate = QDateTime::currentDateTime())//QDateTime(QDate(), QTime::currentTime()))
{
    // Формируем новую группу
    std::shared_ptr<hmcommon::HMGroupInfo> NewGroup = std::make_shared<hmcommon::HMGroupInfo>(inGroupUuid, inCreateDate);
    NewGroup->setName(inGroupName); // Задаём имя группы

    return NewGroup;
}
//-----------------------------------------------------------------------------
/**
 * @brief make_groupmessage - Метод сформирует сообщение для тестирования
 * @param inData - Данные сообщения
 * @param inUuid - UUID сообщения
 * @param inGroupUuid - UUID группы, в которую входит сообщение
 * @param inCreateDate - Дата создания сообщения
 * @return Вернёт указатель на новое сообщение
 */
std::shared_ptr<hmcommon::HMGroupInfoMessage> make_groupmessage(const hmcommon::MsgData& inData, const QUuid inUuid = QUuid::createUuid(), const QUuid inGroupUuid = QUuid::createUuid(),
                                                            const QDateTime inCreateDate = QDateTime::currentDateTime())
{
    std::shared_ptr<hmcommon::HMGroupInfoMessage> NewMessage = std::make_shared<hmcommon::HMGroupInfoMessage>(inUuid, inGroupUuid, inCreateDate);
    NewMessage->setMessage(inData);

    return NewMessage;
}
//-----------------------------------------------------------------------------
} // namespace testscommon
//-----------------------------------------------------------------------------

#endif // HAWKCOMMONTESTUTILS_HPP
