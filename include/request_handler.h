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
        RequestHandler(const RequestHandlerData& request_handler_data);
        virtual http::response<http::string_body> handle_request(const http::request<http::string_body>& request) = 0;
        //can't put the static init function here for registering, because of how subclassing works.
    protected:
        http::response<http::string_body> res_;
        void init_response(const http::request<http::string_body>& request);
        void log_request(const http::request<http::string_body>& request, const http::response<http::string_body>& response, std::string log_message);
        AddrInfo addr_info_;
        
};


#endif