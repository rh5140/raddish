
#include <iostream>
#include <boost/asio.hpp>
#include <gmock/gmock.h>
#include "gtest/gtest.h"
#include "server.h"

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

class ServerTest : public testing::Test {
  protected:
    boost::asio::io_service io_s;
    // int port_num = 8000;
    ConfigInfo config_info;
    Server* test_server;
    
    void SetUp() override {
      config_info.port_num = 8000;
      test_server = new Server(io_s, config_info);
    }
    void TearDown() override {
      delete test_server; // session is deleted in server's destructor
    }
};

TEST_F(ServerTest, StartAccept) {
    EXPECT_TRUE(test_server->start_accept());
}

TEST_F(ServerTest, HandleAccept) {
    Session* test_session = new Session(io_s, config_info);
    EXPECT_NO_THROW(test_server->handle_accept(test_session, boost::system::error_code()));
    delete test_session;
}

TEST_F(ServerTest, HandleAcceptError) {
    Session* test_session = new Session(io_s, config_info);
    EXPECT_NO_THROW(test_server->handle_accept(test_session, boost::asio::error::access_denied));
    delete test_session;
}

TEST(BasicServerSetupTest, ServerStart) {
    boost::asio::io_service io_s;
    ConfigInfo config_info;
    config_info.port_num = 8080;
    Server* test_server = new Server(io_s, config_info);
    EXPECT_NE(test_server, nullptr);
    delete test_server;
}
