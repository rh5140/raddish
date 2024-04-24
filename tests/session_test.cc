
#include <iostream>
#include <boost/asio.hpp>
#include "gtest/gtest.h"
#include "session.h"
#include "config_parser.h"

using boost::asio::ip::tcp;
using namespace std;


//fixture
class SessionTest : public testing::Test {
 protected:
    boost::asio::io_service io_service;
    string parse_result;
    int reasonable_length = 10;
    int max_length = 1024;
    int greater_than_max_length = 1025;
    bool error_exists = false;
    session* test_session;

    void SetUp() override {
      test_session = new session(io_service);
    }
    void TearDown() override {
      delete test_session;
    }
};
//new unit tests
TEST_F(SessionTest, SessionStart) {
  EXPECT_NO_THROW(test_session->start());
}

TEST_F(SessionTest, ParseBody){
    //const char * test_body = "POST / HTTP/1.1\nHost: localhost:8080\nUser-Agent: curl/7.81.0\nAccept: */*\nContent-Length: 17\nContent-Type: application/x-www-form-urlencoded\n\nHello World!";
    const char * test_body = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nhello world";
    size_t max_bytes = 999999;
    parse_result = test_session->parse_data(test_body, &max_bytes);
    EXPECT_EQ(parse_result, string(test_body) + "\n");
}


TEST_F(SessionTest, ParseNoBody){
    const char * test_body = "POST / HTTP/1.1\nHost: localhost:8080\nUser-Agent: curl/7.81.0\nAccept: */*\nContent-Length: 17\nContent-Type: application/x-www-form-urlencoded";
    size_t max_bytes = 999999;
    parse_result = test_session->parse_data(test_body, &max_bytes);
    EXPECT_EQ(parse_result, "POST / HTTP/1.1\nHost: localhost:8080\nUser-Agent: curl/7.81.0\nAccept: */*\nContent-Length: 17\nContent-Type: application/x-www-form-urlencoded\n");
}

TEST_F(SessionTest, CreateResponse) {
    const char * test_body = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 11\n\nhello world";
    test_session->set_buf("hello world");
    EXPECT_EQ(test_session->create_response(), test_body);
}

TEST_F(SessionTest, HandleReadPartialDataRead) { 
    // Marked "should never happen"
    EXPECT_NO_THROW(test_session->handle_read(boost::system::error_code(), greater_than_max_length));
}

TEST_F(SessionTest, HandleRead) {
    EXPECT_NO_THROW(test_session->handle_read(boost::system::error_code(), reasonable_length));
}

TEST_F(SessionTest, HandleWrite) {
    EXPECT_NO_THROW(test_session->handle_write(boost::system::error_code()));
}