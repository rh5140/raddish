#include <iostream>
#include <boost/asio.hpp>
#include "gtest/gtest.h"
#include "request_handler.h"

class FileRequestTest : public testing::Test {
  protected:
    file_request_handler* handler;
    std::string file_path;

    // NOTE: request and max_bytes aren't actually used at all, virtual function in base class needs to be changed
    std::string contents = "GET /images/DOESNTEXIST.png HTTP/1.1\nUser-Agent: curl/7.81.0\nAccept:*/*\n\n";
    const char* request = contents.c_str();
    size_t max_bytes = 51;
    // ^^not used

    std::string response;    
    void SetUp() override {
      // nothing that applies to all since constructor will vary
    }
    void TearDown() override {
      delete handler; 
    }
};

TEST_F(FileRequestTest, FileNotFound) {
    file_path = "/static_files/images/DOESNTEXIST.png";
    handler = new file_request_handler(file_path);
    response = handler->handle_request(request, &max_bytes);
    EXPECT_EQ(response.substr(0,22), "HTTP/1.1 404 Not Found");
}

TEST_F(FileRequestTest, LobsterTest) {
    std::string file = __FILE__;
    file_path = file.substr(0, file.length() - 21) + "/../static_files/text/lobster.txt";
    handler = new file_request_handler(file_path);
    response = handler->handle_request(request, &max_bytes);
    EXPECT_NE(response.substr(0,22), "HTTP/1.1 404 Not Found");
    EXPECT_EQ(response, "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 7\n\nlobster");
}

TEST_F(FileRequestTest, Extensions) {
    // The actual file path is irrelevant to the test and is just necessary for the constructor
    file_path = "/static_files/text/lobster.txt";
    handler = new file_request_handler(file_path);

    EXPECT_EQ("text/html", handler->file_extension_to_content_type("html"));
    EXPECT_EQ("text/plain", handler->file_extension_to_content_type("txt"));
    EXPECT_EQ("image/jpeg", handler->file_extension_to_content_type("jpg"));
    EXPECT_EQ("image/png", handler->file_extension_to_content_type("png"));
    EXPECT_EQ("application/pdf", handler->file_extension_to_content_type("pdf"));
    EXPECT_EQ("application/zip", handler->file_extension_to_content_type("zip"));
    EXPECT_EQ("application/octet-stream", handler->file_extension_to_content_type("bin"));
}