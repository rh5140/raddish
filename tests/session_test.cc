
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

TEST_F(SessionTest, CreateResponse) {
    const char * test_body = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 11\n\nhello world";
    test_session->set_buf("hello world");
    EXPECT_EQ(test_session->create_response(), test_body);
    delete test_session;
}

TEST_F(SessionTest, CreateResponseStaticFile) {
    test_session->config_info_.static_file_locations["/text/"] = "/static_files";
    test_session->set_buf("GET /text/DOESNOTEXIST.txt HTTP/1.1");
    std::string response = test_session->create_response();
    EXPECT_EQ(response.substr(0,22), "HTTP/1.1 404 Not Found");
    delete test_session;
}

TEST_F(SessionTest, CreateResponseEcho) {
    std::string contents = "GET / HTTP/1.1\nUser-Agent: curl/7.81.0\nAccept:*/*\n\n";
    test_session->config_info_.echo_locations.push_back("/");
    test_session->set_buf(contents);
    EXPECT_EQ(test_session->create_response(), "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 51\n\n"+contents);
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
