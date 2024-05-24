#include <gtest/gtest.h>
#include "echo_request_handler.h"
#include "request_handler_factory.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>  

using CreateRequestHandler = RequestHandler*(*)(const RequestHandlerData&);

using namespace std;

class EchoTest : public testing::Test {
    protected:
        EchoRequestHandler* handler;
        RequestHandlerData request_handler_data;
        http::request<http::string_body> req;
        http::response<http::string_body> res;
        CreateRequestHandler handler_factory; //get factory

        void SetUp() override {
            std::string root = "";
            request_handler_data.root = root;
            AddrInfo addr_info;
            addr_info.host_addr =  "host:8080";
            addr_info.client_addr = "client:8080";
            request_handler_data.addr_info = addr_info;
            handler_factory = RequestHandlerFactory::get_factory("EchoRequestHandler");
        }
};

TEST_F(EchoTest, BasicEcho) {
    RequestHandler* handler = handler_factory(request_handler_data);
    res = handler->handle_request(req);

    EXPECT_EQ(res.result(), http::status::ok);
    EXPECT_EQ(res.at(http::field::content_type), "text/plain");
    EXPECT_EQ(res.body(), " HTTP/1.1\r\n\r\n");

    delete handler;
}
