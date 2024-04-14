#include "gtest/gtest.h"
#include "config_parser.h"
TEST(NginxConfigParserTest, SimpleConfig) {
  NginxConfigParser parser;
  NginxConfig out_config;
  bool success = parser.Parse("example_config", &out_config);
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