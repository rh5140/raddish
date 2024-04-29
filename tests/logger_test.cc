/*

THIS DOESNT WORK - 1) doesnt delete after done with test, 2) cant see the file while running 
I leave here if we want to coverage 

#include <boost/log/utility/setup/file.hpp>
#include <boost/log/trivial.hpp>
#include <ctime>
#include <string>
#include "gtest/gtest.h"
#include <filesystem>
#include <iostream>
#include "logger.h"

using namespace std; 
namespace fs = std::filesystem;

class LoggerTest : public testing::Test {
    protected:
        logger test = logger(true);     

        time_t runMessages() {
            // Getting time when first log appears
            time_t currentTime = time(nullptr);

            // Messages
            BOOST_LOG_TRIVIAL(trace) << "A trace severity message";
            BOOST_LOG_TRIVIAL(debug) << "A debug severity message";
            BOOST_LOG_TRIVIAL(info) << "An informational severity message";
            BOOST_LOG_TRIVIAL(warning) << "A warning severity message";
            BOOST_LOG_TRIVIAL(error) << "An error severity message";
            BOOST_LOG_TRIVIAL(fatal) << "A fatal severity message";

            return currentTime;
        }

        string date_to_string(time_t time) {
            tm* timeInfo = localtime(&time);
            string year = to_string(timeInfo->tm_year + 1900);
            string month = to_string(timeInfo->tm_mon + 1); 
            if (month.length() < 2) 
                month = "0" + month;
            string day = to_string(timeInfo->tm_mday); 
            if (day.length() < 2) {
                day = "0" + month;
            }

            return year + '-' + month + '-' + day;
        }

        void TearDown() override {
            fs::remove_all("t_logs"); // idk why this doesnt work for me?
        }
};

TEST_F(LoggerTest, CheckLogExists) {
    time_t date = runMessages();
    string filename = "t_logs/" + date_to_string(date) + "_001.log";
    
    ASSERT_TRUE(fs::exists(filename));
    
}
// TODO: Test to 
// TODO: Test to see if new file at midnight 
// TODO: Test to see if remakes when too full
// TODO: Test to see if contents are right


*/ 
