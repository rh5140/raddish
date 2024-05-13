#ifndef NOT_FOUND_REQUEST_HANDLER
#define NOT_FOUND_REQUEST_HANDLER

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
    
class NotFoundHandler : public RequestHandler {
    public:
        NotFoundHandler(http::request<http::string_body> request, RequestHandlerData requestHandlerData);
        http::response<http::string_body> handle_request();
        static RequestHandler* Init(http::request<http::string_body> request, RequestHandlerData requestHandlerData);
        static bool registered_;
};


#endif