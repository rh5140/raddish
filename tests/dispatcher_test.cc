
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

        NginxConfigParser parser_;
        NginxConfig out_config;
        ConfigInfo config_info_;


        //new stuff
        http::request<http::string_body> req;
        http::response<http::string_body> res;

        void SetUp() override {
            // log_info.addr_info.host_addr = "host:8080";
            // log_info.addr_info.client_addr = "client:8080";
        }

        void TearDown() override {
            //delete dispatcher; 
        }

};



TEST_F(DispatcherTest, BasicDispatch) {
    bool success = parser_.parse("configs/static_files_config", &out_config);
    EXPECT_TRUE(success);
    success = parser_.get_config_settings(&out_config);
    EXPECT_TRUE(success);

    RequestDispatcher* dispatcher = new RequestDispatcher();
    res = dispatcher->dispatch_request(req, parser_.get_config_info(), "8080", "8080");
    EXPECT_EQ(res.result(), http::status::not_found);
    EXPECT_EQ(res.at(http::field::content_type), "text/plain");
    EXPECT_EQ(res.body(), "404 Not Found");
}


TEST_F(DispatcherTest, BasicEcho) {
    bool success = parser_.parse("../server_config", &out_config);
    EXPECT_TRUE(success);
    success = parser_.get_config_settings(&out_config);
    EXPECT_TRUE(success);

    req.target("/echo");
    req.method(http::verb::get);

    RequestDispatcher* dispatcher = new RequestDispatcher();
    res = dispatcher->dispatch_request(req, parser_.get_config_info(), "8080", "8080");
    EXPECT_EQ(res.result(), http::status::ok);
    EXPECT_EQ(res.at(http::field::content_type), "text/plain");
    EXPECT_EQ(res.body(), "GET /echo HTTP/1.1\r\n\r\n");
}   


TEST_F(DispatcherTest, NoMatch) {

    bool success = parser_.parse("../server_config", &out_config);
    EXPECT_TRUE(success);
    success = parser_.get_config_settings(&out_config);
    EXPECT_TRUE(success);

    req.target("/foo");
    req.method(http::verb::get);

    RequestDispatcher* dispatcher = new RequestDispatcher();
    res = dispatcher->dispatch_request(req, parser_.get_config_info(), "8080", "8080");
    EXPECT_EQ(res.result(), http::status::not_found);
    EXPECT_EQ(res.at(http::field::content_type), "text/plain");
    EXPECT_EQ(res.body(), "404 Not Found");
}   