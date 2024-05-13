#include <cstdint>
#include <sstream>
#include <string>
#include <iostream>
#include "echo_request_handler.h"
#include "request_handler_factory.h"

#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <type_traits>

namespace beast = boost::beast;  
namespace http = beast::http;    

//temp
#include <boost/log/trivial.hpp>


bool EchoRequestHandler::registered_ = RequestHandlerFactory::register_handler("EchoRequestHandler", EchoRequestHandler::init);

EchoRequestHandler::EchoRequestHandler(const RequestHandlerData& request_handler_data) : RequestHandler(request_handler_data){

}


RequestHandler* EchoRequestHandler::init(const RequestHandlerData& request_handler_data) {
    return new EchoRequestHandler(request_handler_data); 
}


http::response<http::string_body> EchoRequestHandler::handle_request(const http::request<http::string_body>& request) {
    init_response(request);
    //get body
    std::ostringstream oss;
    oss << request; 
    std::string res_body = oss.str();
    res_.body() = res_body;

    //set vars
    res_.result(http::status::ok); 

    //log
    log_request(request, res_, "Echoed");

    return res_;

    //return http_response;
}

