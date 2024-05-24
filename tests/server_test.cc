#include <gtest/gtest.h>
#include "server.h"

#include <boost/asio.hpp>

using boost::asio::ip::tcp;
using namespace std;

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
