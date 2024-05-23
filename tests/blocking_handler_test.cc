#include <iostream>
#include <boost/asio.hpp>
#include <gmock/gmock.h>
#include <chrono>
#include "gtest/gtest.h"
#include "blocking_request_handler.h"
#include "request_handler_factory.h"
#include "echo_request_handler.h"
#include "info.h"
#include <boost/log/trivial.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
    

using CreateRequestHandler = RequestHandler*(*)(const RequestHandlerData&);

using boost::asio::ip::tcp;
using namespace std;

using std::chrono::high_resolution_clock;
using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::milliseconds;


class BlockingTest : public testing::Test {
    protected:
        RequestHandlerData request_handler_data;
        CreateRequestHandler blocking_handler_factory;
        RequestHandler* blocking_handler;
        // LogInfo log_info;

        void SetUp() override {
            std::string root = "";
            request_handler_data.root = root;
            AddrInfo addr_info;
            addr_info.host_addr =  "host:8080";
            addr_info.client_addr = "client:8080";
            request_handler_data.addr_info = addr_info;
            blocking_handler_factory = RequestHandlerFactory::get_factory("BlockingRequestHandler");
            blocking_handler = blocking_handler_factory(request_handler_data);
        }

        void TearDown() override {
            delete blocking_handler; 
        }
};

TEST_F(BlockingTest, CompareHandlerTimes) {
    std::chrono::time_point<high_resolution_clock> pre_blocked_time;
    std::chrono::time_point<high_resolution_clock> post_blocked_time;
    
    pre_blocked_time = high_resolution_clock::now();
    http::request<http::string_body> req;
    blocking_handler->handle_request(req);
    post_blocked_time = high_resolution_clock::now();

    int blocked_time_ms = duration_cast<milliseconds>(post_blocked_time-pre_blocked_time).count();
    EXPECT_GE(blocked_time_ms, 1000);
}
