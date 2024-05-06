
#include <iostream>
#include <boost/asio.hpp>
#include <gmock/gmock.h>
#include "gtest/gtest.h"
#include "request_dispatcher.h"
#include "config_parser.h"
#include "info.h"

using boost::asio::ip::tcp;
using namespace std;

class DispatcherTest : public testing::Test {
    protected:
        RequestDispatcher* dispatcher;
        LogInfo log_info;
        RequestDispatcherInfo req_dis_info;
        vector<char> buf_;

        NginxConfigParser parser_;
        NginxConfig out_config;
        ConfigInfo config_info_;

        void SetUp() override {
            log_info.addr_info.host_addr = "host:8080";
            log_info.addr_info.client_addr = "client:8080";
        }

        void TearDown() override {
            //delete dispatcher; 
        }

        void SetBuf(std::string buf) {
            buf_.clear();
            for (char c : buf) {
                buf_.push_back(c);
            }
        }
};

TEST_F(DispatcherTest, BasicDispatch) {
    bool success = parser_.Parse("configs/static_files_config", &out_config);
    EXPECT_TRUE(success);
    success = parser_.GetServerSettings(&out_config);
    EXPECT_TRUE(success);

    req_dis_info.addr_info.client_addr = "client:8080";
    req_dis_info.addr_info.host_addr = "host:8080";
    req_dis_info.request = "bad request!";
    req_dis_info.request_size = 12;
    req_dis_info.config_info = parser_.GetConfigInfo();

    RequestDispatcher* dispatcher = new RequestDispatcher();
    EXPECT_EQ(dispatcher->dispatch_request(req_dis_info), "HTTP/1.1 400 Bad Request\nContent-Type: text/plain\nContent-Length: 0\n\n");
}


TEST_F(DispatcherTest, BasicEcho) {
    std::string request = "GET / HTTP/1.1\nUser-Agent: curl/7.81.0\nAccept:*/*\n\n";
    bool success = parser_.Parse("configs/static_files_config", &out_config);
    EXPECT_TRUE(success);
    success = parser_.GetServerSettings(&out_config);
    EXPECT_TRUE(success);

    req_dis_info.addr_info.client_addr = "client:8080";
    req_dis_info.addr_info.host_addr = "host:8080";
    req_dis_info.request = request;
    req_dis_info.request_size = request.size();
    req_dis_info.config_info = parser_.GetConfigInfo();
    RequestDispatcher* dispatcher = new RequestDispatcher();

    EXPECT_EQ(dispatcher->dispatch_request(req_dis_info), "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 51\n\n"+request);
}   