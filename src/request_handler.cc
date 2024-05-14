
#include "request_handler.h"
#include "info.h"
#include <cstdint>
#include <sstream>
#include <string>
#include <iostream>

#include <boost/log/trivial.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/lexical_cast.hpp>

namespace beast = boost::beast;  
namespace http = beast::http;   

RequestHandler::RequestHandler(const RequestHandlerData& request_handler_data){
    addr_info_ = request_handler_data.addr_info;
}


void RequestHandler::init_response(const http::request<http::string_body>& request){
    //generate response object
    res_ = http::response<http::string_body>{http::status::not_found, request.version()};
    res_.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res_.set(http::field::content_type, "text/plain");

    //default error
    res_.body() = "404 Not Found";
    //can add more args as needed
}

void RequestHandler::log_request(const http::request<http::string_body>& request, const http::response<http::string_body>& response, std::string log_message) {
    std::string client_str = ", Client: " + addr_info_.client_addr;
    std::string host_str = ", Host: " + addr_info_.host_addr;
    std::string req_header = boost::lexical_cast<std::string>(request.base());
    std::string request_line_str = ", Request: \"" + req_header.substr(0, req_header.find('\n')-1) + "\"";

    int status = res_.result_int();

    // can add more cases e.g. 204 for no content
    switch (status) {
        case (200):
            BOOST_LOG_TRIVIAL(info) << "Status " + std::to_string(status) + client_str + host_str + request_line_str + " " + log_message;
            break;
        default:
            BOOST_LOG_TRIVIAL(warning) << "Status " + std::to_string(status) + client_str + host_str + request_line_str + " " + log_message;
    }

    std::string res_header = boost::lexical_cast<std::string>(response.base());
    std::string res_body = boost::lexical_cast<std::string>(response.body());

    BOOST_LOG_TRIVIAL(trace) << res_header << res_body;
}


