
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
  void SetUp() override {
  }
  // void TearDown() override {}
    boost::asio::io_service io_service;
    session* test_session = new session(io_service);
    string parse_result;
};
//new unit tests
TEST_F(SessionTest, SessionStart) {
  EXPECT_NO_THROW(test_session->start());
}



TEST_F(SessionTest, ParseBody){
    //const char * test_body = "POST / HTTP/1.1\nHost: localhost:8080\nUser-Agent: curl/7.81.0\nAccept: */*\nContent-Length: 17\nContent-Type: application/x-www-form-urlencoded\n\nHello World!";
    const char * test_body = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nhello world";
    parse_result = test_session->parse_data(test_body);
    EXPECT_EQ(parse_result, string(test_body) + "\n");
}

TEST_F(SessionTest, ResponseGeneration){
  std::string response = test_session->create_response(0);
  EXPECT_EQ(response,"HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 0\n\n");
}

//TEST_F(SessionTest, ParseNoBody){
//    const char * test_body = "POST / HTTP/1.1\nHost: localhost:8080\nUser-Agent: curl/7.81.0\nAccept: */*\nContent-Length: 17\nContent-Type: application/x-www-form-urlencoded";
//    parse_result = test_session->parse_data(test_body);
//    EXPECT_EQ(parse_result, "");
//}

//TODO: test handle_read