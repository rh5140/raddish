#include "gtest/gtest.h"
#include <sstream>
#include <string>
#include "config_parser.h"
#include "info.h"

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
  bool success = parser.parse("configs/example_config", &out_config);
  EXPECT_TRUE(success);
}

TEST_F(ParseTest, BadConfigPath) {
  bool success = parser.parse("configs/thisdoesnotexist_config", &out_config);
  EXPECT_FALSE(success);
}
TEST_F(ParseTest, EmptyFile){
  bool success = parser.parse("configs/empty_config", &out_config);
  EXPECT_TRUE(success);
}
TEST_F(ParseTest, NoEnd){
  bool success = parser.parse("configs/no_end_config", &out_config);
  EXPECT_FALSE(success);
}
TEST_F(ParseTest, LargeConfig){
  bool success = parser.parse("configs/larger_config", &out_config);
  EXPECT_TRUE(success);
}

TEST_F(ParseTest, OpenEnd){
  bool success = parser.parse("configs/open_end_config", &out_config);
  EXPECT_FALSE(success);
}


TEST_F(ParseTest, OpenBeginning){
  bool success = parser.parse("configs/open_config", &out_config);
  EXPECT_FALSE(success);
}



//TokenAsString testing
TEST_F(ParseTest, TokenToString){
  EXPECT_EQ(string(parser.number_to_token_string(0)), "TOKEN_TYPE_START");
  EXPECT_EQ(string(parser.number_to_token_string(1)), "TOKEN_TYPE_NORMAL");
  EXPECT_EQ(string(parser.number_to_token_string(2)), "TOKEN_TYPE_START_BLOCK");
  EXPECT_EQ(string(parser.number_to_token_string(3)), "TOKEN_TYPE_END_BLOCK");
  EXPECT_EQ(string(parser.number_to_token_string(4)), "TOKEN_TYPE_COMMENT");
  EXPECT_EQ(string(parser.number_to_token_string(5)), "TOKEN_TYPE_STATEMENT_END");
  EXPECT_EQ(string(parser.number_to_token_string(6)), "TOKEN_TYPE_EOF");
  EXPECT_EQ(string(parser.number_to_token_string(7)), "TOKEN_TYPE_ERROR");
}


//get server settings test
TEST_F(ParseTest, ServerPortGood){
  bool success = parser.parse("configs/basic_server_config", &out_config);
  EXPECT_TRUE(success);
  success = parser.get_config_settings(&out_config);
  EXPECT_TRUE(success);
  int port_num = parser.get_port_num();
  EXPECT_EQ(port_num, 8080);
}

TEST_F(ParseTest, ServerPortBad){
  bool success = parser.parse("configs/bad_server_config", &out_config);
  EXPECT_TRUE(success);
  success = parser.get_config_settings(&out_config);
  EXPECT_FALSE(success);
}


TEST_F(ParseTest, ServerPortTooBig){
  bool success = parser.parse("configs/port_too_big_config", &out_config);
  EXPECT_TRUE(success);
  success = parser.get_config_settings(&out_config);
  EXPECT_FALSE(success);
}

TEST_F(ParseTest, ServerPortTooSmall){
  bool success = parser.parse("configs/port_too_small_config", &out_config);
  EXPECT_TRUE(success);
  success = parser.get_config_settings(&out_config);
  EXPECT_FALSE(success);
}

TEST_F(ParseTest, BiggerConfig){
  bool success = parser.parse("configs/big_server_config", &out_config);
  EXPECT_TRUE(success);
  success = parser.get_config_settings(&out_config);
  EXPECT_TRUE(success);
  int port_num = parser.get_port_num();
  EXPECT_EQ(port_num, 8080);
}

TEST_F(ParseTest, StaticFilesConfig) {
  bool success = parser.parse("configs/static_files_config", &out_config);
  EXPECT_TRUE(success);
  success = parser.get_config_settings(&out_config);
  EXPECT_TRUE(success);
  int port_num = parser.get_port_num();
  EXPECT_EQ(port_num, 8080);
  EXPECT_NO_THROW(parser.get_config_info());
}

TEST_F(ParseTest, DuplicatePathConfig) {
  bool success = parser.parse("configs/duplicate_path_config", &out_config);
  EXPECT_TRUE(success);
  success = parser.get_config_settings(&out_config);
  EXPECT_FALSE(success);
}

TEST_F(ParseTest, TrailingSlashConfig) {
  bool success = parser.parse("configs/duplicate_path_trailing_slash_config", &out_config);
  EXPECT_TRUE(success);
  success = parser.get_config_settings(&out_config);
  EXPECT_FALSE(success);
}

TEST_F(ParseTest, HashtagInsideQuoteConfig) {
  bool success = parser.parse("configs/hashtag_inside_quote_config", &out_config);
  EXPECT_TRUE(success);
  success = parser.get_config_settings(&out_config);
  EXPECT_TRUE(success);
  EXPECT_NO_THROW(parser.get_config_info());
  ConfigInfo config_info = parser.get_config_info();
  EXPECT_EQ(config_info.location_to_directives["/"]["root"], "/forimages/blah #should be fine");
}

TEST_F(ParseTest, QuotesConfig) {
  bool success = parser.parse("configs/quotes_config", &out_config);
  EXPECT_TRUE(success);
  success = parser.get_config_settings(&out_config);
  EXPECT_TRUE(success);
  EXPECT_NO_THROW(parser.get_config_info());
  ConfigInfo config_info = parser.get_config_info();
  EXPECT_EQ(config_info.location_to_directives["/test"]["root"], "/forimages/blah");
  EXPECT_EQ(config_info.location_to_directives["/test/check"]["root"], "/forimages/blah");
}

TEST_F(ParseTest, RelativePathConfig) {
  bool success = parser.parse("configs/quotes_config", &out_config);
  EXPECT_TRUE(success);
  success = parser.get_config_settings(&out_config);
  EXPECT_TRUE(success);
  EXPECT_NO_THROW(parser.get_config_info());
}

//to string testing

TEST_F(ParseTest, ConfigToString){
  bool success = parser.parse("configs/hello_config", &out_config);
  EXPECT_TRUE(success);
  EXPECT_EQ(out_config.ToString(), "hello \"world\";\n");
  EXPECT_EQ((*out_config.statements_[0]).ToString(0), "hello \"world\";\n");
}


TEST_F(ParseTest, ConfigToStringLong){
  bool success = parser.parse("configs/larger_config", &out_config);
  EXPECT_TRUE(success);
  EXPECT_EQ(out_config.ToString(), "user www www;\nworker_processes 5;\nerror_log logs/error.log;\npid logs/nginx.pid;\nworker_rlimit_nofile 8192;\nevents {\n  worker_connections 4096;\n}\nhttp {\n  include conf/mime.types;\n  include /etc/nginx/proxy.conf;\n  include /etc/nginx/fastcgi.conf;\n  index index.html index.htm index.php;\n  default_type application/octet-stream;\n  log_format main '$remote_addr - $remote_user [$time_local]  $status ' '\"$request\" $body_bytes_sent \"$http_referer\" ' '\"$http_user_agent\" \"$http_x_forwarded_for\"';\n  access_log logs/access.log main;\n  sendfile on;\n  tcp_nopush on;\n  server_names_hash_bucket_size 128;\n}\n");
}