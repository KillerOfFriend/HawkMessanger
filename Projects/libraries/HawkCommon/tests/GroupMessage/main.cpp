#include <gtest/gtest.h>

#include <groupmessage.h>
#include <systemerrorex.h>

//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест создания сообщения
 */
TEST(Message, Create)
{
    QUuid MessageUuid = QUuid::createUuid();
    QUuid MessageGroupUuid = QUuid::createUuid();
    QDateTime CreateTime = QDateTime::currentDateTime();

    hmcommon::HMGroupInfoMessage Message(MessageUuid, MessageGroupUuid, CreateTime);

    EXPECT_EQ(Message.m_uuid, MessageUuid);
    EXPECT_EQ(Message.m_group, MessageGroupUuid);
    EXPECT_EQ(Message.m_createTime, CreateTime);
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест формирования данных сообщения
 */
TEST(Message, CreateMessageData)
{
    QString MessageText = "Message text";
    hmcommon::MsgData Data(hmcommon::eMsgType::mtText, MessageText.toLocal8Bit());

    EXPECT_EQ(Data.m_type, hmcommon::eMsgType::mtText);
    EXPECT_EQ(QString::fromLocal8Bit(Data.m_data), MessageText);
}
//-----------------------------------------------------------------------------
/**
 * @brief TEST - Тест хранения параметров сообщения
 */
TEST(Message, CheckParams)
{
    QString MessageText = "Message text";
    hmcommon::HMGroupInfoMessage Message(QUuid::createUuid(), QUuid::createUuid());

    std::error_code Error = Message.setMessage(hmcommon::MsgData(hmcommon::eMsgType::mtText, MessageText.toLocal8Bit()));
    ASSERT_FALSE(Error); // Ошибки быть не должно

    hmcommon::MsgData RetData = Message.getMesssage();

    EXPECT_EQ(RetData.m_type, hmcommon::eMsgType::mtText);
    EXPECT_EQ(QString::fromLocal8Bit(RetData.m_data), MessageText);
}
//-----------------------------------------------------------------------------
/**
 * @brief main - Входная точка тестировани функционала HMJsonDataStorage
 * @param argc - Количество аргументов
 * @param argv - Перечень аргументов
 * @return Вернёт признак успешности тестирования
 */
int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
//-----------------------------------------------------------------------------
