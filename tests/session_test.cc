
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
    size_t reasonable_length = 16;
    size_t max_length = 1024;
    size_t greater_than_max_length = 1025;
    bool error_exists = false;
    Session* test_session;
    ConfigInfo config_info;


    http::request<http::string_body> req;
    http::response<http::string_body> res;

    void SetUp() override {
      test_session = new Session(io_service, config_info);
    }
    // void TearDown() override {
    // }
};

//new unit tests
TEST_F(SessionTest, SessionStart) {
    EXPECT_NO_THROW(test_session->start());
}

TEST_F(SessionTest, Create400Response) {
    test_session->set_req(req);
    res = test_session->create_response();

    EXPECT_EQ(res.result(), http::status::bad_request);
    EXPECT_EQ(res.at(http::field::content_type), "text/plain");
    EXPECT_EQ(res.body(), "400 Bad Request");

    delete test_session;
}


TEST_F(SessionTest, CreateResponseStaticFile) {
    //test_session->config_info_.location_to_handler["/text/"] = "FileRequestHandler";
    //test_session->config_info_.location_to_directives["root"]["/text/"] = "/static_files";
    //test_session->set_buf("GET /text/DOESNOTEXIST.txt HTTP/1.1\n\n");
    //std::string response = test_session->create_response();
    //EXPECT_EQ(response.substr(0,22), "HTTP/1.1 404 Not Found");
    delete test_session;
}


//TODO: not sure exactly why this specific test broke

TEST_F(SessionTest, CreateResponseEcho) {
    test_session->config_info_.location_to_handler["/"] = "EchoRequestHandler";
    req.target("/echo");
    req.method(http::verb::get);
    test_session->set_req(req);
    res = test_session->create_response();

    EXPECT_EQ(res.result(), http::status::ok);
    EXPECT_EQ(res.at(http::field::content_type), "text/plain");
    EXPECT_EQ(res.body(), "GET /echo HTTP/1.1\r\n\r\n");
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
    std::string contents = "GET / HTTP/1.1\n\n";
    test_session->config_info_.location_to_handler["/"] = "EchoRequestHandler";
    //test_session->set_buf(contents);
    EXPECT_NO_THROW(test_session->handle_read(boost::system::error_code(), reasonable_length));
    delete test_session;
}

TEST_F(SessionTest, HandleWrite) {
    size_t written;
    EXPECT_NO_THROW(test_session->handle_write(boost::system::error_code(), written));
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
    size_t written;
    EXPECT_NO_THROW(test_session->handle_write(boost::asio::error::access_denied, written));
}
