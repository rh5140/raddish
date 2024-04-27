
#include <iostream>
#include <boost/asio.hpp>
#include "gtest/gtest.h"
#include "session.h"

using boost::asio::ip::tcp;
using namespace std;


//fixture
class SessionTest : public testing::Test {
  protected:
    boost::asio::io_service io_service;
    string parse_result;
    size_t reasonable_length = 10;
    size_t max_length = 1024;
    size_t greater_than_max_length = 1025;
    bool error_exists = false;
    session* test_session;
    ConfigInfo config_info;

    void SetUp() override {
      test_session = new session(io_service, config_info);
    }
    // void TearDown() override {
    // }
};

//new unit tests
TEST_F(SessionTest, SessionStart) {
    EXPECT_NO_THROW(test_session->start());
}

// TEST_F(SessionTest, ParseBody){
//     //const char * test_body = "POST / HTTP/1.1\nHost: localhost:8080\nUser-Agent: curl/7.81.0\nAccept: */*\nContent-Length: 17\nContent-Type: application/x-www-form-urlencoded\n\nHello World!";
//     const char * test_body = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nhello world";
//     size_t max_bytes = 999999;
//     parse_result = test_session->parse_data(test_body, &max_bytes);
//     EXPECT_EQ(parse_result, string(test_body) + "\n");
// }


// TEST_F(SessionTest, ParseNoBody){
//     const char * test_body = "POST / HTTP/1.1\nHost: localhost:8080\nUser-Agent: curl/7.81.0\nAccept: */*\nContent-Length: 17\nContent-Type: application/x-www-form-urlencoded";
//     size_t max_bytes = 999999;
//     parse_result = test_session->parse_data(test_body, &max_bytes);
//     EXPECT_EQ(parse_result, "POST / HTTP/1.1\nHost: localhost:8080\nUser-Agent: curl/7.81.0\nAccept: */*\nContent-Length: 17\nContent-Type: application/x-www-form-urlencoded\n");
// }

TEST_F(SessionTest, CreateResponse) {
    const char * test_body = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 11\n\nhello world";
    test_session->set_buf("hello world");
    EXPECT_EQ(test_session->create_response(), test_body);
    delete test_session;
}

TEST_F(SessionTest, HandleReadMaxLength) {
    EXPECT_NO_THROW(test_session->handle_read(boost::system::error_code(), max_length));
    delete test_session;
}

TEST_F(SessionTest, HandleReadPartialDataRead) { 
    // Marked "should never happen"
    EXPECT_NO_THROW(test_session->handle_read(boost::system::error_code(), greater_than_max_length));
    delete test_session;
}

TEST_F(SessionTest, HandleRead) {
    EXPECT_NO_THROW(test_session->handle_read(boost::system::error_code(), reasonable_length));
    delete test_session;
}

TEST_F(SessionTest, HandleWrite) {
    EXPECT_NO_THROW(test_session->handle_write(boost::system::error_code()));
    delete test_session;
}

// Failure cases delete the session
TEST_F(SessionTest, HandleReadEOF) {
    EXPECT_NO_THROW(test_session->handle_read(boost::asio::error::eof, reasonable_length));
}

TEST_F(SessionTest, HandleReadConnectionReset) {
  EXPECT_NO_THROW(test_session->handle_read(boost::asio::error::connection_reset, reasonable_length));
}

TEST_F(SessionTest, HandleReadOtherError) {
    EXPECT_NO_THROW(test_session->handle_read(boost::asio::error::access_denied, reasonable_length));
}

TEST_F(SessionTest, HandleWriteError) {
    EXPECT_NO_THROW(test_session->handle_write(boost::asio::error::access_denied));
}
