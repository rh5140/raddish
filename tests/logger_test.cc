#include <gtest/gtest.h>
#include "logger.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <fstream>
#include <regex>
#include <boost/log/attributes.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/utility/setup/file.hpp>


using namespace std; 
namespace fs = std::filesystem;
namespace logging = boost::log;
namespace attrs = boost::log::attributes;
namespace sev_lvl = boost::log::trivial;

class LoggerTest : public testing::Test {
    protected:
        Logger test;     
        string dir_path = fs::current_path().string() + "/t_logs";
        logging::sources::logger lg;

        int get_num_logs() {
            int fileCount = 0;
            if (fs::exists(dir_path) && fs::is_directory(dir_path)) {
                for (const auto& entry : fs::directory_iterator(dir_path)) {
                    fileCount++;
                }
            }
            return fileCount;
        }

        string get_contents(string file) {
            string contents = "";

            ifstream fileStream(file);
            if (fileStream.is_open()) {
                string line;
                while (getline(fileStream, line)) {
                    contents = contents + line + "\n";
                }
                fileStream.close();
            }

            return contents;
        }
        
        void SetUp() override {
            test.set_test_status(true);
            lg.add_attribute("Process", attrs::mutable_constant<std::string>("Test"));
            lg.add_attribute("Severity", attrs::mutable_constant<sev_lvl::severity_level>(sev_lvl::info));
        }
        void TearDown() override {
            fs::remove_all("t_logs");
        }
};

TEST_F(LoggerTest, CheckConsole) {
    // direct cout into buffer
    stringstream buffer;
    streambuf* old = cout.rdbuf(buffer.rdbuf());
    BOOST_LOG(lg) << "Insert message here!";
    test.force_log_rotation();
    cout.rdbuf(old); // redirect cout to normal output

    EXPECT_NE(buffer.str(), ""); 
}

TEST_F(LoggerTest, CheckFileExists) {
    BOOST_LOG(lg) << "Insert message here!";
    test.force_log_rotation(); 
    ASSERT_EQ(get_num_logs(), 1);
}

TEST_F(LoggerTest, CheckFormat) {
    // getting output
    stringstream buffer;
    streambuf* old = cout.rdbuf(buffer.rdbuf());
    BOOST_LOG(lg) << "Insert message here!";
    test.force_log_rotation();
    cout.rdbuf(old);

    // comparing with regex
    string pattern = "^\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{6}\\] \\[.+\\] \\[Test\\] \\[info\\] Insert message here!\n$";
    regex regexPattern(pattern);    
    ASSERT_TRUE(regex_match(buffer.str(), regexPattern)); 
}

TEST_F(LoggerTest, RotateWhenFull) {
    // There has to be a better way to do this
    for(int i = 0; i < 79375; i++) {
        BOOST_LOG(lg) << "This is a test! Pleaze stand by as you watch this tomfoolery!";
    }
    test.force_log_rotation();
    ASSERT_GE(get_num_logs(), 2);
}

/*
TODO: How to set time
TEST_F(LoggerTest, RotateWhenNight) {
}
*/

