#ifndef ECHO_REQUEST_HANDLER
#define ECHO_REQUEST_HANDLER

#include <string>
#include <gtest/gtest_prod.h>
#include "request_handler.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
    

class EchoRequestHandler : public RequestHandler {
    public:
        EchoRequestHandler(http::request<http::string_body> request, RequestHandlerData requestHandlerData);
        http::response<http::string_body> handle_request();
        static RequestHandler* Init(http::request<http::string_body> request, RequestHandlerData requestHandlerData); //can't put these in parent because they're static.
        static bool registered_;
    private:
        std::string echo_req_;
        size_t max_bytes_;
};

#endif