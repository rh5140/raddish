#include <gtest/gtest_prod.h>
#include <ctime>
#include <string.h>

class logger {
public:
    logger(bool isTest);
private:
    void init_logs();   
    time_t force_log_rotation(); // for test purposes
    bool isTest;

    friend class LoggerTest;
    FRIEND_TEST(LoggerTest, CheckFileExists);
    FRIEND_TEST(LoggerTest, CheckFormat);
    FRIEND_TEST(LoggerTest, CheckConsole);
    FRIEND_TEST(LoggerTest, RotateWhenFull);
    //FRIEND_TEST(LoggerTest, RotateWhenNight);
};



