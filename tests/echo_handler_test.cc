
#include <iostream>
#include <boost/asio.hpp>
#include <gmock/gmock.h>
#include "gtest/gtest.h"
#include "request_handler.h"

using boost::asio::ip::tcp;
using namespace std;

//Leaving this commented out for now, as we don't need it but it could prove useful in the future as a template for mocks.
/*
class MockServer : public server{
  public:
    MockServer(boost::asio::io_service& io_service, short port) : server(io_service, port){}
    MOCK_METHOD(void, handle_accept, (session* new_session, const boost::system::error_code& error));
    
};

//fixture
class ServerTest : public testing::Test {
 protected:
  void SetUp() override {}
  // void TearDown() override {}
  boost::asio::io_service io_s;
  int port_num = 8080;
  MockServer* test_server = new MockServer(io_s, port_num);

};

TEST_F(ServerTest, ServerStart) {
}
*/

class EchoTest : public testing::Test {
    protected:
        echo_request_handler* handler;
        const char* request;
        size_t max_bytes;
        std::string response;
        std::string log_msg;

        void TearDown() override {
            delete handler; 
        }
};

TEST_F(EchoTest, BasicEcho) {
    std::string contents = "GET / HTTP/1.1\nUser-Agent: curl/7.81.0\nAccept:*/*\n\n";
    request = contents.c_str();
    max_bytes = 51;
    handler = new echo_request_handler(request, &max_bytes);
    response = handler->handle_request(log_msg);
    EXPECT_EQ(response, "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 51\n\n"+contents);
    EXPECT_EQ(log_msg, "200 - Raddish echoed what Charlie said - ");
}
