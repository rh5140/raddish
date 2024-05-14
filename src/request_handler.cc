
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

namespace beast = boost::beast;  
namespace http = beast::http;   

RequestHandler::RequestHandler(http::request<http::string_body> request, RequestHandlerData requestHandlerData){
    req_ = request;
    log_info_ = requestHandlerData.log_info;

    //generate response object
    res_ = http::response<http::string_body>{http::status::not_found, req_.version()};
    res_.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res_.set(http::field::content_type, "text/html");

    //default error
    res_.body() = "404 Not Found";
    //can add more args as needed
}


void RequestHandler::log_request() {
    std::string client_str = ", Client: " + log_info_.addr_info.client_addr;
    std::string host_str = ", Host: " + log_info_.addr_info.host_addr;
    std::string request_line_str = ", Request: \"" + log_info_.request_line + "\"";

    int status = 404;
    try {
        // extract status code from response
        std::string http_ver = "HTTP/1.1 ";
        status = std::stoi(log_info_.response.substr(http_ver.length(), http_ver.length()+3));
    }
    catch(...) {
        BOOST_LOG_TRIVIAL(error) << "cannot find status code in response - invalid response";
    }

    // can add more cases e.g. 204 for no content
    switch (status) {
        case (200):
            BOOST_LOG_TRIVIAL(info) << std::to_string(status) + client_str + host_str + request_line_str + " " + log_info_.message;
            break;
        default:
            BOOST_LOG_TRIVIAL(warning) << std::to_string(status) + client_str + host_str + request_line_str + " " + log_info_.message;
    }

    BOOST_LOG_TRIVIAL(trace) << log_info_.message << " " << log_info_.response;
}


