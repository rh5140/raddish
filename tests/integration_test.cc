
#include <iostream>
#include <boost/asio.hpp>
#include <gmock/gmock.h>
#include "gtest/gtest.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
// #include "server.h"




using boost::asio::ip::tcp;
using namespace std;

//fixture
class IntegrationTest : public testing::Test {
 protected:
  void SetUp() override {
  }
  // void TearDown() override {}
  char buffer[7];
};

// new unit tests
TEST_F(IntegrationTest, SuccessfulTest) {
    FILE *fp = popen("bash integration_test.sh local requests/get_request responses/get_response", "r");
    fgets(buffer, 7, fp);
    EXPECT_TRUE(strcmp(buffer,"SUCCESS"));
    pclose(fp);
}

TEST_F(IntegrationTest, FailedTest) {
    FILE *fp = popen("bash integration_test.sh local requests/get_request responses/get_response_failure", "r");
    fgets(buffer, 7, fp);
    EXPECT_TRUE(strcmp(buffer,"FAILURE"));
    pclose(fp);
}

TEST_F(IntegrationTest, NCSingleNewline) {
    FILE *fp = popen("bash integration_test.sh local requests/nc_single_newline_request responses/nc_single_newline_response", "r");
    fgets(buffer, 7, fp);
    EXPECT_TRUE(strcmp(buffer,"SUCCESS"));
    pclose(fp);
}


TEST_F(IntegrationTest, NCDoubleNewline) {
    FILE *fp = popen("bash integration_test.sh local requests/nc_double_newline_request responses/nc_double_newline_response", "r");
    fgets(buffer, 7, fp);
    EXPECT_TRUE(strcmp(buffer,"SUCCESS"));
    pclose(fp);
}

TEST_F(IntegrationTest, CurlRoot) {
    FILE *fp = popen("bash integration_test.sh local requests/curl_root_request responses/curl_root_response", "r");
    fgets(buffer, 7, fp);
    EXPECT_TRUE(strcmp(buffer,"SUCCESS"));
    pclose(fp);
}

TEST_F(IntegrationTest, CurlRootClose) {
    FILE *fp = popen("bash integration_test.sh local requests/curl_root_close_request responses/curl_root_close_response", "r");
    fgets(buffer, 7, fp);
    EXPECT_TRUE(strcmp(buffer,"SUCCESS"));
    pclose(fp);
}

TEST_F(IntegrationTest, CurlRootKeepAlive) {
    FILE *fp = popen("bash integration_test.sh local requests/curl_root_keep_alive_request responses/curl_root_keep_alive_response requests/curl_root_close_request responses/curl_root_close_response", "r");
    fgets(buffer, 7, fp);
    EXPECT_TRUE(strcmp(buffer,"SUCCESS"));
    pclose(fp);
}

TEST_F(IntegrationTest, CurlPath) {
    FILE *fp = popen("bash integration_test.sh local requests/curl_path_request responses/curl_path_response", "r");
    fgets(buffer, 7, fp);
    EXPECT_TRUE(strcmp(buffer,"SUCCESS"));
    pclose(fp);
}

TEST_F(IntegrationTest, CurlShortHeader) {
    FILE *fp = popen("bash integration_test.sh local requests/curl_short_header_request responses/curl_short_header_response", "r");
    fgets(buffer, 7, fp);
    EXPECT_TRUE(strcmp(buffer,"SUCCESS"));
    pclose(fp);
}

// TODO: uncomment after header length issue is fixed
// TEST_F(IntegrationTest, CurlLongHeader) {
//     FILE *fp = popen("bash integration_test.sh local requests/curl_long_header_request responses/curl_long_header_response", "r");
//     fgets(buffer, 7, fp);
//     EXPECT_TRUE(strcmp(buffer,"SUCCESS"));
//     pclose(fp);
// }

// TODO: uncomment after buffer garbage issue is fixed
// TEST_F(IntegrationTest, NCSingleNewlineCloud) {
//     FILE *fp = popen("bash integration_test.sh cloud requests/nc_single_newline_request responses/nc_single_newline_response", "r");
//     fgets(buffer, 7, fp);
//     EXPECT_TRUE(strcmp(buffer,"SUCCESS"));
//     pclose(fp);
// }




