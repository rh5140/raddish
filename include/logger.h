#include <gtest/gtest_prod.h>

class Logger {
public:
    Logger();
private:
    void init_logs();  

    // test functions
    void set_test_status(bool is_test);
    void force_log_rotation(); 
    bool is_test_;

    friend class LoggerTest;
    FRIEND_TEST(LoggerTest, CheckFileExists);
    FRIEND_TEST(LoggerTest, CheckFormat);
    FRIEND_TEST(LoggerTest, CheckConsole);
    FRIEND_TEST(LoggerTest, RotateWhenFull);
    //FRIEND_TEST(LoggerTest, RotateWhenNight);
};



