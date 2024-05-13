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






NotFoundHandler::NotFoundHandler(const RequestHandlerData& request_handler_data) : RequestHandler(request_handler_data){
}

RequestHandler* NotFoundHandler::init(const RequestHandlerData& request_handler_data) {
    return new NotFoundHandler(request_handler_data); 
}

bool NotFoundHandler::registered_ = RequestHandlerFactory::register_handler("NotFoundHandler", NotFoundHandler::init);

http::response<http::string_body> NotFoundHandler::handle_request(const http::request<http::string_body>& request) {
    init_response(request);
    //set vars
    res_.result(http::status::not_found); 
    //log
    log_request(request, res_, "Path not configured");
    return res_;
}