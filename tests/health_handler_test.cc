#include "gtest/gtest.h"
#include "health_request_handler.h"
#include "request_handler_factory.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>

using CreateRequestHandler = RequestHandler*(*)(const RequestHandlerData&);

class HealthHandlerTest : public testing::Test {
    protected:
        HealthRequestHandler* handler;
        RequestHandlerData request_handler_data;
        http::request<http::string_body> req;
        http::response<http::string_body> res;
        CreateRequestHandler handler_factory;

        void SetUp() override {
            request_handler_data.root = "";
            AddrInfo addr_info;
            addr_info.host_addr =  "host:8080";
            addr_info.client_addr = "client:8080";
            request_handler_data.addr_info = addr_info;
            handler_factory = RequestHandlerFactory::get_factory("HealthRequestHandler"); //get factory
        }

        void TearDown() override {
            delete handler;
        }

};

TEST_F(HealthHandlerTest, BasicHealthTest) {
    RequestHandler* handler = handler_factory(request_handler_data);
    res = handler->handle_request(req);

    EXPECT_EQ(res.result(), http::status::ok);
    EXPECT_EQ(res.at(http::field::content_type), "text/plain");
    EXPECT_EQ(res.body(), "OK");

    delete handler;
}