#include <cstdint>
#include <sstream>
#include <string>
#include <iostream>
#include "not_found_request_handler.h"
#include "request_handler_factory.h"
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
namespace beast = boost::beast;  
namespace http = beast::http;    
//temp
#include <boost/log/trivial.hpp>






NotFoundHandler::NotFoundHandler(http::request<http::string_body> request, RequestHandlerData requestHandlerData) : RequestHandler(request, requestHandlerData){
}

RequestHandler* NotFoundHandler::Init(http::request<http::string_body> request, RequestHandlerData requestHandlerData) {
    return new NotFoundHandler(request, requestHandlerData); 
}

bool NotFoundHandler::registered_ = RequestHandlerFactory::register_handler("NotFoundHandler", NotFoundHandler::Init);

http::response<http::string_body> NotFoundHandler::handle_request() {
    //set vars
    res_.result(http::status::not_found); 
    //log
    log_info_.message = "Path not configured";
    log_info_.response = "";
    log_request();
    return res_;
}