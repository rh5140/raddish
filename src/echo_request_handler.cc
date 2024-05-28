#include "echo_request_handler.h"
#include "request_handler_factory.h"

#include <sstream>
#include <string>

namespace beast = boost::beast;  
namespace http = beast::http;    

//register
bool EchoRequestHandler::registered_ = RequestHandlerFactory::register_handler("EchoRequestHandler", EchoRequestHandler::init);

EchoRequestHandler::EchoRequestHandler(const RequestHandlerData& request_handler_data) : RequestHandler(request_handler_data){

}

//factory
RequestHandler* EchoRequestHandler::init(const RequestHandlerData& request_handler_data) {
    return new EchoRequestHandler(request_handler_data); 
}

http::response<http::string_body> EchoRequestHandler::handle_request(const http::request<http::string_body>& request) {
    init_response(request);

    //convert request to string
    std::ostringstream oss;
    oss << request; 
    std::string res_body = oss.str();

    //set vars
    res_.body() = res_body;
    res_.result(http::status::ok); 

    //log
    log_request(request, res_, "Echoed", "EchoRequestHandler");

    //return http_response;
    return res_;
}

