#include <iostream>
#include <boost/asio.hpp>
#include <gmock/gmock.h>
#include "gtest/gtest.h"
#include "not_found_request_handler.h"
#include "request_handler_factory.h"
#include "info.h"
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


class NotFoundTest : public testing::Test {
    protected:
        NotFoundHandler* handler;
        RequestHandlerData request_handler_data;
        http::request<http::string_body> req;
        http::response<http::string_body> res;
        CreateRequestHandler handler_factory;
        // LogInfo log_info;

        void SetUp() override {
            std::string root = "";
            request_handler_data.root = root;
            AddrInfo addr_info;
            addr_info.host_addr =  "host:8080";
            addr_info.client_addr = "client:8080";
            request_handler_data.addr_info = addr_info;
            handler_factory = RequestHandlerFactory::get_factory("NotFoundHandler"); //get factory
        }

        void TearDown() override {
            // delete handler; 
        }
};


TEST_F(NotFoundTest, BasicNotFound) {
    RequestHandler* handler = handler_factory(request_handler_data);
    res = handler->handle_request(req);

    EXPECT_EQ(res.result(), http::status::not_found);
    EXPECT_EQ(res.at(http::field::content_type), "text/plain");
    EXPECT_EQ(res.body(), "404 Not Found");
    delete handler;
}
