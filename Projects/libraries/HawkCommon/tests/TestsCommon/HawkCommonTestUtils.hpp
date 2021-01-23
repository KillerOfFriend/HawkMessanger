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
 * @brief make_user_info - Метод сформирует информацию о пользователе для тестирования
 * @param inUserUuid - UUID пользователя
 * @param inUserLogin - Логин пользователя
 * @param inUserPassword - Пароль пользователя
 * @param inCreateDate - Дата создания пользователя
 * @return Вернёт указатель на нового пользователя
 */
std::shared_ptr<hmcommon::HMUserInfo> make_user_info(const QUuid inUserUuid = QUuid::createUuid(), const QString inUserLogin = "UserLogin@login.com",
                                            const QString inUserPassword = "P@ssworOfUser123", const QDateTime inCreateDate = QDateTime::currentDateTime())
{
    std::shared_ptr<hmcommon::HMUserInfo> NewUser = std::make_shared<hmcommon::HMUserInfo>(inUserUuid, inCreateDate);
    // Задаём основные параметры
    NewUser->setLogin(inUserLogin);
    NewUser->setPassword(inUserPassword);

    return NewUser;
}
//-----------------------------------------------------------------------------
/**
 * @brief make_group_info - Метод сформирует информацию о группе для тестирования
 * @param inGroupUuid - UUID группы
 * @param inGroupName - Имя группы
 * @param inCreateDate - Дата создания группы
 * @return Вернёт указатель на новую группу
 */
std::shared_ptr<hmcommon::HMGroupInfo> make_group_info(const QUuid inGroupUuid = QUuid::createUuid(), const QString inGroupName = "New group name",
                                              const QDateTime inCreateDate = QDateTime::currentDateTime())
{
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

    errors::error_code Error = NewMessage->setMessage(inData);
    assert(!Error);

    return NewMessage;
}
//-----------------------------------------------------------------------------
} // namespace testscommon
//-----------------------------------------------------------------------------

#endif // HAWKCOMMONTESTUTILS_HPP
