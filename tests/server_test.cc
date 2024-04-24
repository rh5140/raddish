
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
    int port_num = 8000;
    server* test_server;
    
    void SetUp() override {
      test_server = new server(io_s, port_num);
    }
    void TearDown() override {
      delete test_server; // session is deleted in server's destructor
    }
};

TEST_F(ServerTest, StartAccept) {
    EXPECT_TRUE(test_server->start_accept());
}

TEST_F(ServerTest, HandleAccept) {
    session* test_session = new session(io_s);
    EXPECT_NO_THROW(test_server->handle_accept(test_session, boost::system::error_code()));
    delete test_session;
}

TEST_F(ServerTest, HandleAcceptError) {
    session* test_session = new session(io_s);
    EXPECT_NO_THROW(test_server->handle_accept(test_session, boost::asio::error::access_denied));
    delete test_session;
}

TEST(BasicServerSetupTest, ServerStart) {
    boost::asio::io_service io_s;
    int port_num = 8080;
    server* test_server = new server(io_s, port_num);
    EXPECT_NE(test_server, nullptr);
    delete test_server;
}
