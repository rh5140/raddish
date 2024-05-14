
#include <iostream>
#include <boost/asio.hpp>
#include <gmock/gmock.h>
#include "gtest/gtest.h"
#include "request_handler.h"
#include "echo_request_handler.h"
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

//Leaving this commented out for now, as we don't need it but it could prove useful in the future as a template for mocks.
/*
class MockServer : public server{
  public:
    MockServer(boost::asio::io_service& io_service, short port) : server(io_service, port){}
    MOCK_METHOD(void, handle_accept, (session* new_session, const boost::system::error_code& error));
    
};

//fixture
class ServerTest : public testing::Test {
 protected:
  void SetUp() override {}
  // void TearDown() override {}
  boost::asio::io_service io_s;
  int port_num = 8080;
  MockServer* test_server = new MockServer(io_s, port_num);

};

TEST_F(ServerTest, ServerStart) {
}
*/

class EchoTest : public testing::Test {
    protected:
        EchoRequestHandler* handler;
        RequestHandlerData request_handler_data;
        http::request<http::string_body> req;
        http::response<http::string_body> res;
        CreateRequestHandler handler_factory; //get factory
        // LogInfo log_info;

        void SetUp() override {
            std::string root = "";
            request_handler_data.root = root;
            AddrInfo addr_info;
            addr_info.host_addr =  "host:8080";
            addr_info.client_addr = "client:8080";
            request_handler_data.addr_info = addr_info;
            handler_factory = RequestHandlerFactory::get_factory("EchoRequestHandler");
        }

        void TearDown() override {
            // delete handler; 
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
