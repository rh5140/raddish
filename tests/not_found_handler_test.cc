#include <iostream>
#include <boost/asio.hpp>
#include <gmock/gmock.h>
#include "gtest/gtest.h"
#include "not_found_request_handler.h"
#include "info.h"
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
class NotFoundTest : public testing::Test {
    protected:
        NotFoundHandler* handler;
        const char* request;
        size_t max_bytes;
        std::string response;
        LogInfo log_info;
        void SetUp() override {
            log_info.addr_info.host_addr = "host:8080";
            log_info.addr_info.client_addr = "client:8080";
        }
        void TearDown() override {
            delete handler; 
        }
};
