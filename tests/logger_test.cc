#include <boost/log/utility/setup/file.hpp>
#include <boost/log/trivial.hpp>
#include "gtest/gtest.h"
#include <filesystem>
#include <iostream>
#include <string>
#include <fstream>
#include <regex>
#include "logger.h"

using namespace std; 
namespace fs = std::filesystem;

class LoggerTest : public testing::Test {
    protected:
        Logger test;     
        string dir_path = fs::current_path().string() + "/t_logs";

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
        }
        void TearDown() override {
            fs::remove_all("t_logs");
        }
};

TEST_F(LoggerTest, CheckConsole) {
    // direct cout into buffer
    stringstream buffer;
    streambuf* old = cout.rdbuf(buffer.rdbuf());
    BOOST_LOG_TRIVIAL(info) << "Insert message here!";
    test.force_log_rotation();
    cout.rdbuf(old); // 

    // checking if console matches 
    string pattern = "^\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{6}\\] \\[.+\\] \\[info\\] Insert message here!\n$";
    regex regexPattern(pattern); 
    ASSERT_TRUE(regex_match(buffer.str(), regexPattern)); 
}

TEST_F(LoggerTest, CheckFileExists) {
    BOOST_LOG_TRIVIAL(info) << "Insert message here!";
    test.force_log_rotation(); 
    ASSERT_EQ(get_num_logs(), 1);
}

TEST_F(LoggerTest, CheckFormat) {
    BOOST_LOG_TRIVIAL(info) << "Insert message here!";
    test.force_log_rotation();

    // get filename (bc time is a pain)
    string filename = ""; 
    for (const auto& entry : fs::directory_iterator(dir_path)) {
        if (entry.is_regular_file()) {
            filename = entry.path().string();
        }
    }

    // comparing with regex
    string pattern = "^\\[[0-9]{4}-[0-9]{2}-[0-9]{2} [0-9]{2}:[0-9]{2}:[0-9]{2}\\.[0-9]{6}\\] \\[.+\\] \\[info\\] Insert message here!\n$";
    regex regexPattern(pattern);    
    string contents = get_contents(filename);
    ASSERT_TRUE(regex_match(contents, regexPattern)); 
}

/*
TEST_F(LoggerTest, RotateWhenFull) {
    // There has to be a better way to do this
    for(int i = 0; i < 88187; i++) {
        BOOST_LOG_TRIVIAL(info) << "This is a test! Pleaze stand by as you watch this tomfoolery!";
    }
    test.force_log_rotation();
    ASSERT_EQ(get_num_logs(), 2);
}

/*
TODO: How to set time
TEST_F(LoggerTest, RotateWhenNight) {
}
*/

