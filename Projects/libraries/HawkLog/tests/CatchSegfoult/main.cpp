#include <gtest/gtest.h>
#include <HawkLog.h>

//-----------------------------------------------------------------------------
void initLogSystem()
{
    // Настравиваем вывод логов в терминал и в файл
    hmlog::HMLogSystem::getInstance().setConfig(hmlog::eLogConfig::lcNone);
    // Включаем обработку всех опасных событий
    hmlog::HMEventSystem::getInstance().setConfig(hmlog::eCapturedEvents::ceAll);
}
//-----------------------------------------------------------------------------
TEST(Catch_SegFault,TEST1)
{
    try
    {
        bool TestSuccess = false;
        hmlog::HMEventSystem::getInstance().setHandler(hmlog::eCapturedEvents::ceInteractiveAttention, [&TestSuccess]() -> void
        {
            TestSuccess = true;
            EXPECT_EQ (TestSuccess,  true);
        });

//        int* SegFault = nullptr;
//        *SegFault = 5;
    }
    catch (...)
    {

    }
}
//-----------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    initLogSystem();
    return RUN_ALL_TESTS();
}
//-----------------------------------------------------------------------------
