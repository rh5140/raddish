#include "gtest/gtest.h"
#include "config_parser.h"
TEST(NginxConfigParserTest, SimpleConfig) {
  NginxConfigParser parser;
  NginxConfig out_config;
  bool success = parser.Parse("configs/example_config", &out_config);
  EXPECT_TRUE(success);
}
//fixture
class ParseTest : public testing::Test {
 protected:
  void SetUp() override {
  }
  // void TearDown() override {}
  NginxConfigParser parser;
  NginxConfig out_config;  
  int port_num;

};
//new unit tests
TEST_F(ParseTest, BadConfigPath) {
  bool success = parser.Parse("configs/thisdoesnotexist_config", &out_config);
  EXPECT_FALSE(success);
}
TEST_F(ParseTest, EmptyFile){
  bool success = parser.Parse("configs/empty_config", &out_config);
  EXPECT_TRUE(success);
}
TEST_F(ParseTest, NoEnd){
  bool success = parser.Parse("configs/no_end_config", &out_config);
  EXPECT_FALSE(success);
}
TEST_F(ParseTest, LargeConfig){
  bool success = parser.Parse("configs/larger_config", &out_config);
  EXPECT_TRUE(success);
}

TEST_F(ParseTest, OpenEnd){
  bool success = parser.Parse("configs/open_end_config", &out_config);
  EXPECT_FALSE(success);
}


TEST_F(ParseTest, OpenBeginning){
  bool success = parser.Parse("configs/open_config", &out_config);
  EXPECT_FALSE(success);
}






//get server settings test
TEST_F(ParseTest, ServerPortGood){
  bool success = parser.Parse("configs/basic_server_config", &out_config);
  EXPECT_TRUE(success);
  success = parser.GetServerSettings(&out_config, &port_num);
  EXPECT_TRUE(success);
  EXPECT_EQ(port_num, 8080);
}

TEST_F(ParseTest, ServerPortBad){
  bool success = parser.Parse("configs/bad_server_config", &out_config);
  EXPECT_TRUE(success);
    success = parser.GetServerSettings(&out_config, &port_num);
  EXPECT_FALSE(success);
}


TEST_F(ParseTest, ServerPortTooBig){
  bool success = parser.Parse("configs/port_too_big_config", &out_config);
  EXPECT_TRUE(success);
    success = parser.GetServerSettings(&out_config, &port_num);
  EXPECT_FALSE(success);
}

TEST_F(ParseTest, ServerPortTooSmall){
  bool success = parser.Parse("configs/port_too_small_config", &out_config);
  EXPECT_TRUE(success);
    success = parser.GetServerSettings(&out_config, &port_num);
  EXPECT_FALSE(success);
}

TEST_F(ParseTest, BiggerConfig){
  bool success = parser.Parse("configs/big_server_config", &out_config);
  EXPECT_TRUE(success);
    success = parser.GetServerSettings(&out_config, &port_num);
  EXPECT_TRUE(success);
  EXPECT_EQ(port_num, 8080);
}

//to string testing

TEST_F(ParseTest, ConfigToString){
  bool success = parser.Parse("configs/hello_config", &out_config);
  EXPECT_TRUE(success);
  EXPECT_EQ(out_config.ToString(), "hello \"world\";\n");
  EXPECT_EQ((*out_config.statements_[0]).ToString(0), "hello \"world\";\n");
}