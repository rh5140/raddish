#ifndef REQUEST_HANDLER
#define REQUEST_HANDLER

#include <string>
#include <gtest/gtest_prod.h>
#include "info.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>



class RequestHandler {
    public:
        RequestHandler(http::request<http::string_body> request, RequestHandlerData requestHandlerData);
        virtual http::response<http::string_body> handle_request() = 0;
    protected:
        http::request<http::string_body> req_;
        http::response<http::string_body> res_;
        void log_request();
        LogInfo log_info_;
        
};





#endif