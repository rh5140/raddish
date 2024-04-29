#include "gtest/gtest.h"
#include <sstream>
#include <string>
#include "config_parser.h"

using namespace std; //for str

//fixture
class ParseTest : public testing::Test {
 protected:
  void SetUp() override {
  }
  // void TearDown() override {}
  NginxConfigParser parser;
  NginxConfig out_config;

};

TEST_F(ParseTest, SimpleConfig) {
  bool success = parser.Parse("configs/example_config", &out_config);
  EXPECT_TRUE(success);
}

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



//TokenAsString testing
TEST_F(ParseTest, TokenToString){
  EXPECT_EQ(string(parser.NumberToTokenString(0)), "TOKEN_TYPE_START");
  EXPECT_EQ(string(parser.NumberToTokenString(1)), "TOKEN_TYPE_NORMAL");
  EXPECT_EQ(string(parser.NumberToTokenString(2)), "TOKEN_TYPE_START_BLOCK");
  EXPECT_EQ(string(parser.NumberToTokenString(3)), "TOKEN_TYPE_END_BLOCK");
  EXPECT_EQ(string(parser.NumberToTokenString(4)), "TOKEN_TYPE_COMMENT");
  EXPECT_EQ(string(parser.NumberToTokenString(5)), "TOKEN_TYPE_STATEMENT_END");
  EXPECT_EQ(string(parser.NumberToTokenString(6)), "TOKEN_TYPE_EOF");
  EXPECT_EQ(string(parser.NumberToTokenString(7)), "TOKEN_TYPE_ERROR");
}


//get server settings test
TEST_F(ParseTest, ServerPortGood){
  bool success = parser.Parse("configs/basic_server_config", &out_config);
  EXPECT_TRUE(success);
  success = parser.GetServerSettings(&out_config);
  EXPECT_TRUE(success);
  int port_num = parser.GetPortNum();
  EXPECT_EQ(port_num, 8080);
}

TEST_F(ParseTest, ServerPortBad){
  bool success = parser.Parse("configs/bad_server_config", &out_config);
  EXPECT_TRUE(success);
  success = parser.GetServerSettings(&out_config);
  EXPECT_FALSE(success);
}


TEST_F(ParseTest, ServerPortTooBig){
  bool success = parser.Parse("configs/port_too_big_config", &out_config);
  EXPECT_TRUE(success);
  success = parser.GetServerSettings(&out_config);
  EXPECT_FALSE(success);
}

TEST_F(ParseTest, ServerPortTooSmall){
  bool success = parser.Parse("configs/port_too_small_config", &out_config);
  EXPECT_TRUE(success);
  success = parser.GetServerSettings(&out_config);
  EXPECT_FALSE(success);
}

TEST_F(ParseTest, BiggerConfig){
  bool success = parser.Parse("configs/big_server_config", &out_config);
  EXPECT_TRUE(success);
  success = parser.GetServerSettings(&out_config);
  EXPECT_TRUE(success);
  int port_num = parser.GetPortNum();
  EXPECT_EQ(port_num, 8080);
}

//to string testing

TEST_F(ParseTest, ConfigToString){
  bool success = parser.Parse("configs/hello_config", &out_config);
  EXPECT_TRUE(success);
  EXPECT_EQ(out_config.ToString(), "hello \"world\";\n");
  EXPECT_EQ((*out_config.statements_[0]).ToString(0), "hello \"world\";\n");
}


TEST_F(ParseTest, ConfigToStringLong){
  bool success = parser.Parse("configs/larger_config", &out_config);
  EXPECT_TRUE(success);
  EXPECT_EQ(out_config.ToString(), "user www www;\nworker_processes 5;\nerror_log logs/error.log;\npid logs/nginx.pid;\nworker_rlimit_nofile 8192;\nevents {\n  worker_connections 4096;\n}\nhttp {\n  include conf/mime.types;\n  include /etc/nginx/proxy.conf;\n  include /etc/nginx/fastcgi.conf;\n  index index.html index.htm index.php;\n  default_type application/octet-stream;\n  log_format main '$remote_addr - $remote_user [$time_local]  $status ' '\"$request\" $body_bytes_sent \"$http_referer\" ' '\"$http_user_agent\" \"$http_x_forwarded_for\"';\n  access_log logs/access.log main;\n  sendfile on;\n  tcp_nopush on;\n  server_names_hash_bucket_size 128;\n}\n");
}