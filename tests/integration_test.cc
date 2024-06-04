#include <gtest/gtest.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>

using namespace std;

//fixture
class IntegrationTest : public testing::Test {
 protected:
 
  void TearDown() override {
    pclose(fp);
  }
  char buffer[7];
  FILE *fp;
  std::string result="";
};

// new unit tests
// TEST_F(IntegrationTest, SuccessfulTest) {
//     fp = popen("bash integration_test.sh local requests/get_request responses/get_response", "r");
//     while(!feof(fp)) {
//         if(fgets(buffer, 7, fp) != NULL)
//             result += buffer;
//     }
//     EXPECT_EQ(result.substr(result.size()-7), "SUCCESS");
//     // EXPECT_EQ(result.substr(result.size()-7), "SUCCESS");
// }

// TEST_F(IntegrationTest, FailedTest) {
//     fp = popen("bash integration_test.sh local requests/get_request responses/get_response_failure", "r");
//     while(!feof(fp)) {
//         if(fgets(buffer, 7, fp) != NULL)
//             result += buffer;
//     }
//     EXPECT_EQ(result.substr(result.size()-7), "FAILURE");
// }

/* Note: Netcat tests removed for now, as they don't have a filepath and cannot be echoed 
 the script's nc lines have been replaced by static file paths */

// TEST_F(IntegrationTest, NCSingleNewline) {
//     fp = popen("bash integration_test.sh local requests/nc_single_newline_request responses/nc_single_newline_response", "r");
//     while(!feof(fp)) {
//         if(fgets(buffer, 7, fp) != NULL)
//             result += buffer;
//     }
//     EXPECT_EQ(result.substr(result.size()-7), "SUCCESS");
// }


// TEST_F(IntegrationTest, NCDoubleNewline) {
//     fp = popen("bash integration_test.sh local requests/nc_double_newline_request responses/nc_double_newline_response", "r");
//     while(!feof(fp)) {
//         if(fgets(buffer, 7, fp) != NULL)
//             result += buffer;
//     }
//     EXPECT_EQ(result.substr(result.size()-7), "SUCCESS");
// }

// TEST_F(IntegrationTest, CurlRoot) {
//     fp = popen("bash integration_test.sh local requests/curl_root_request responses/curl_root_response", "r");
//     while(!feof(fp)) {
//         if(fgets(buffer, 7, fp) != NULL)
//             result += buffer;
//     }
//     EXPECT_EQ(result.substr(result.size()-7), "SUCCESS");
// }

//temp disable
/*
TEST_F(IntegrationTest, CurlRootClose) {
    fp = popen("bash integration_test.sh local requests/curl_root_close_request responses/curl_root_close_response", "r");
    while(!feof(fp)) {
        if(fgets(buffer, 7, fp) != NULL)
            result += buffer;
    }
    EXPECT_EQ(result.substr(result.size()-7), "SUCCESS");
}
*/

// TEST_F(IntegrationTest, CurlRootKeepAlive) {
//     fp = popen("bash integration_test.sh local requests/curl_root_keep_alive_request responses/curl_root_keep_alive_response requests/curl_root_close_request responses/curl_root_close_response", "r");
//     while(!feof(fp)) {
//         if(fgets(buffer, 7, fp) != NULL)
//             result += buffer;
//     }
//     EXPECT_EQ(result.substr(result.size()-7), "SUCCESS");
// }

// TEST_F(IntegrationTest, CurlPath) {
//     fp = popen("bash integration_test.sh local requests/curl_path_request responses/curl_path_response", "r");
//     while(!feof(fp)) {
//         if(fgets(buffer, 7, fp) != NULL)
//             result += buffer;
//     }
//     EXPECT_EQ(result.substr(result.size()-7), "SUCCESS");
// }

// TEST_F(IntegrationTest, CurlShortHeader) {
//     fp = popen("bash integration_test.sh local requests/curl_short_header_request responses/curl_short_header_response", "r");
//     while(!feof(fp)) {
//         if(fgets(buffer, 7, fp) != NULL)
//             result += buffer;
//     }
//     EXPECT_EQ(result.substr(result.size()-7), "SUCCESS");
// }

// TEST_F(IntegrationTest, CurlLongHeader) {
//     fp = popen("bash integration_test.sh local requests/curl_long_header_request responses/curl_long_header_response", "r");
//     while(!feof(fp)) {
//         if(fgets(buffer, 7, fp) != NULL)
//             result += buffer;
//     }
//     EXPECT_EQ(result.substr(result.size()-7), "SUCCESS");
// }


// TEST_F(IntegrationTest, CurlTextStaticFile) {
//     fp = popen("bash integration_test.sh local requests/curl_text_static_file_request responses/curl_text_static_file_response", "r");
//     while(!feof(fp)) {
//         if(fgets(buffer, 7, fp) != NULL)
//             result += buffer;
//     }
//     EXPECT_EQ(result.substr(result.size()-7), "SUCCESS");
// }



// TEST_F(IntegrationTest, NCSingleNewlineCloud) {
//     fp = popen("bash integration_test.sh cloud requests/nc_single_newline_request responses/nc_single_newline_response", "r");
//     while(!feof(fp)) {
//         if(fgets(buffer, 7, fp) != NULL)
//             result += buffer;
//     }
//     EXPECT_EQ(result.substr(result.size()-7), "SUCCESS");
// }

// TEST_F(IntegrationTest, NCDoubleNewlineCloud) {
//     fp = popen("bash integration_test.sh cloud requests/nc_double_newline_request responses/nc_double_newline_response", "r");
//     while(!feof(fp)) {
//         if(fgets(buffer, 7, fp) != NULL)
//             result += buffer;
//     }
//     EXPECT_EQ(result.substr(result.size()-7), "SUCCESS");
// }

// TEST_F(IntegrationTest, CurlRootCloud) {
//     fp = popen("bash integration_test.sh cloud requests/curl_root_request responses/curl_root_response", "r");
//     while(!feof(fp)) {
//         if(fgets(buffer, 7, fp) != NULL)
//             result += buffer;
//     }
//     EXPECT_EQ(result.substr(result.size()-7), "SUCCESS");
// }

// TEST_F(IntegrationTest, CurlRootCloseCloud) {
//     fp = popen("bash integration_test.sh cloud requests/curl_root_close_request responses/curl_root_close_response", "r");
//     while(!feof(fp)) {
//         if(fgets(buffer, 7, fp) != NULL)
//             result += buffer;
//     }
//     EXPECT_EQ(result.substr(result.size()-7), "SUCCESS");
// }

// TEST_F(IntegrationTest, CurlRootKeepAliveCloud) {
//     fp = popen("bash integration_test.sh cloud requests/curl_root_keep_alive_request responses/curl_root_keep_alive_response requests/curl_root_close_request responses/curl_root_close_response", "r");
//     while(!feof(fp)) {
//         if(fgets(buffer, 7, fp) != NULL)
//             result += buffer;
//     }
//     EXPECT_EQ(result.substr(result.size()-7), "SUCCESS");
// }

// TEST_F(IntegrationTest, CurlPathCloud) {
//     fp = popen("bash integration_test.sh cloud requests/curl_path_request responses/curl_path_response", "r");
//     while(!feof(fp)) {
//         if(fgets(buffer, 7, fp) != NULL)
//             result += buffer;
//     }
//     EXPECT_EQ(result.substr(result.size()-7), "SUCCESS");
// }

// TEST_F(IntegrationTest, CurlShortHeaderCloud) {
//     fp = popen("bash integration_test.sh cloud requests/curl_short_header_request responses/curl_short_header_response", "r");
//     while(!feof(fp)) {
//         if(fgets(buffer, 7, fp) != NULL)
//             result += buffer;
//     }
//     EXPECT_EQ(result.substr(result.size()-7), "SUCCESS");
// }

// TEST_F(IntegrationTest, CurlLongHeaderCloud) {
//     fp = popen("bash integration_test.sh cloud requests/curl_long_header_request responses/curl_long_header_response", "r");
//     while(!feof(fp)) {
//         if(fgets(buffer, 7, fp) != NULL)
//             result += buffer;
//     }
//     EXPECT_EQ(result.substr(result.size()-7), "SUCCESS");
// }

// TEST_F(IntegrationTest, CurlTextStaticFileCloud) {
//     fp = popen("bash integration_test.sh cloud requests/curl_text_static_file_request responses/curl_text_static_file_response", "r");
//     while(!feof(fp)) {
//         if(fgets(buffer, 7, fp) != NULL)
//             result += buffer;
//     }
//     EXPECT_EQ(result.substr(result.size()-7), "SUCCESS");
// }



