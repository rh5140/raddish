
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

//call in each handler - creates the response object and initializes it to a 404.
//means that you can just early return on fail.
void RequestHandler::init_response(const http::request<http::string_body>& request){
    //generate response object
    res_ = http::response<http::string_body>{http::status::not_found, request.version()};
    res_.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res_.set(http::field::content_type, "text/plain");

    //default error
    res_.body() = "404 Not Found";
    //can add more args as needed
}

//logs client/server ip, result, etc.
void RequestHandler::log_request(const http::request<http::string_body>& request, const http::response<http::string_body>& response, std::string log_message) {
    std::string client_str = ", Client: " + addr_info_.client_addr;
    std::string host_str = ", Host: " + addr_info_.host_addr;
    std::string req_header = boost::lexical_cast<std::string>(request.base());
    std::string request_line_str = ", Request: \"" + req_header.substr(0, req_header.find('\n')-1) + "\"";

    int status = res_.result_int();

    // can add more cases e.g. 204 for no content
    switch (status) {
        case (200):
            BOOST_LOG_TRIVIAL(info) << "Status " + std::to_string(status) + " - " + log_message + client_str + host_str + request_line_str;
            break;
        default:
            BOOST_LOG_TRIVIAL(warning) << "Status " + std::to_string(status) + " - " + log_message + client_str + host_str + request_line_str;
    }

    std::string res_header = boost::lexical_cast<std::string>(response.base());
    std::string res_body = boost::lexical_cast<std::string>(response.body());
    if (response[http::field::content_type].to_string() != "text/plain") { 
        res_body = "Binary of " + std::string(request.target()); // for clarity in logs
    }

    BOOST_LOG_TRIVIAL(trace) << "Full HTTP response :\n" << res_header << res_body;
}


