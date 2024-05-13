#include <iostream>
#include <boost/asio.hpp>
#include "gtest/gtest.h"
#include "file_request_handler.h"
#include "info.h"

class FileRequestTest : public testing::Test {
    protected:
        FileRequestHandler* handler;
        std::string file_path;
        // LogInfo log_info;
        
        std::string response;    
        void SetUp() override {
            // log_info.addr_info.host_addr = "host:8080";
            // log_info.addr_info.client_addr = "client:8080";
        }
        void TearDown() override {
            delete handler; 
        }
};

//TODO: req objects
/*
TEST_F(FileRequestTest, FileNotFound) {
    file_path = "/static_files/images/DOESNTEXIST.png";
    handler = new FileRequestHandler(file_path);
    log_info.request_line = "GET "+ file_path +" HTTP/1.1";
    response = handler->handle_request(log_info);
    EXPECT_EQ(response.substr(0,22), "HTTP/1.1 404 Not Found");
}

TEST_F(FileRequestTest, NotAFile) {
    std::string file = __FILE__;
    std::string file_name = "file_handler_test.cc";
    file_path = file.substr(0, file.length() - file_name.length()) + "/../static_files/images";
    handler = new FileRequestHandler(file_path);
    log_info.request_line = "GET "+ file_path +" HTTP/1.1";
    response = handler->handle_request(log_info);
    EXPECT_EQ(response.substr(0,22), "HTTP/1.1 404 Not Found");
}

TEST_F(FileRequestTest, LobsterTest) {
    std::string file = __FILE__;
    std::string file_name = "file_handler_test.cc";
    file_path = file.substr(0, file.length() - file_name.length()) + "/../static_files/text/lobster.txt";
    handler = new FileRequestHandler(file_path);
    log_info.request_line = "GET "+ file_path +" HTTP/1.1";
    response = handler->handle_request(log_info);
    EXPECT_NE(response.substr(0,22), "HTTP/1.1 404 Not Found");
    EXPECT_EQ(response, "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 7\n\nlobster");
}

TEST_F(FileRequestTest, Extensions) {
    // The actual file path is irrelevant to the test and is just necessary for the constructor
    file_path = "/static_files/text/lobster.txt";
    handler = new FileRequestHandler(file_path);

    EXPECT_EQ("text/html", handler->file_extension_to_content_type("html"));
    EXPECT_EQ("text/plain", handler->file_extension_to_content_type("txt"));
    EXPECT_EQ("image/jpeg", handler->file_extension_to_content_type("jpg"));
    EXPECT_EQ("image/png", handler->file_extension_to_content_type("png"));
    EXPECT_EQ("application/pdf", handler->file_extension_to_content_type("pdf"));
    EXPECT_EQ("application/zip", handler->file_extension_to_content_type("zip"));
    EXPECT_EQ("application/octet-stream", handler->file_extension_to_content_type("bin"));
}

*/