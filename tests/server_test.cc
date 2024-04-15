
/*
#include "gtest/gtest.h"
#include "server.h"
#include <iostream>
#include <boost/asio.hpp>
#include "server.h"

using boost::asio::ip::tcp;


//fixture
class ServerTest : public testing::Test {
 protected:
  void SetUp() override {
  }
  // void TearDown() override {}
    boost::asio::io_service io_service;
    server s(io_service, 8080);
};



//new unit tests
TEST_F(ParseTest, BadConfigPath) {
  bool success = parser.Parse("thisdoesnotexist_config", &out_config);
  EXPECT_FALSE(success);
}
TEST_F(ParseTest, EmptyFile){
  bool success = parser.Parse("empty_config", &out_config);
  EXPECT_TRUE(success);
}
TEST_F(ParseTest, NoEnd){
  bool success = parser.Parse("no_end_config", &out_config);
  EXPECT_FALSE(success);
}
TEST_F(ParseTest, LargeConfig){
  bool success = parser.Parse("larger_config", &out_config);
  EXPECT_TRUE(success);
}
*/