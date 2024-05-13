#include <cstdint>
#include <sstream>
#include <string>
#include <iostream>
#include "echo_request_handler.h"
#include "request_handler_factory.h"

#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>

namespace beast = boost::beast;  
namespace http = beast::http;    

//temp
#include <boost/log/trivial.hpp>


bool EchoRequestHandler::registered_ = RequestHandlerFactory::register_handler("EchoRequestHandler", EchoRequestHandler::Init);

EchoRequestHandler::EchoRequestHandler(http::request<http::string_body> request, RequestHandlerData requestHandlerData) : RequestHandler(request, requestHandlerData){
        std::ostringstream oss;
        oss << request;
        std::string echo_req = oss.str();
        echo_req_ = echo_req;
        max_bytes_ = size_t(echo_req_.size());
}


RequestHandler* EchoRequestHandler::Init(http::request<http::string_body> request, RequestHandlerData requestHandlerData) {
    return new EchoRequestHandler(request, requestHandlerData); 
}


http::response<http::string_body> EchoRequestHandler::handle_request() {
    //get body
    std::ostringstream oss;
    oss << req_; 
    std::string res_body = oss.str();
    res_.body() = res_body;

    //log
    log_info_.message = "Echoed";
    log_info_.response = res_body;
    log_request();

    //set vars
    res_.result(http::status::ok); 
    return res_;

    //return http_response;
}

