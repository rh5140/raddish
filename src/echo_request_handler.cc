#include <cstdint>
#include <sstream>
#include <string>
#include <iostream>
#include "request_handler.h"

#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>

namespace beast = boost::beast;  
namespace http = beast::http;    

//temp
#include <boost/log/trivial.hpp>


EchoRequestHandler::EchoRequestHandler(http::request<http::string_body> request) : RequestHandler(request){
        std::ostringstream oss;
        oss << request;
        std::string echo_req = oss.str();
        echo_req_ = echo_req;
        max_bytes_ = size_t(echo_req_.size());
}

http::response<http::string_body> EchoRequestHandler::handle_request(LogInfo log_info) {
    //get body
    std::ostringstream oss;
    oss << req_; 
    std::string res_body = oss.str();
    res_.body() = res_body;

    //log
    log_info.message = "Echoed";
    log_info.response = res_body;
    log_request(log_info);

    //set vars
    res_.result(http::status::ok); 
    return res_;

    //return http_response;
}
