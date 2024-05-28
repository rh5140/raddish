#include "health_request_handler.h"
#include "request_handler_factory.h"

namespace beast = boost::beast;  
namespace http = beast::http;  

bool HealthRequestHandler::registered_ = RequestHandlerFactory::register_handler("HealthRequestHandler", HealthRequestHandler::init);

HealthRequestHandler::HealthRequestHandler(const RequestHandlerData& request_handler_data) : RequestHandler(request_handler_data) { }

//factory
RequestHandler* HealthRequestHandler::init(const RequestHandlerData& request_handler_data) {
    return new HealthRequestHandler(request_handler_data); 
}

http::response<http::string_body> HealthRequestHandler::handle_request(const http::request<http::string_body>& request) {
    init_response(request);

    //set vars
    res_.body() = "OK";
    res_.result(http::status::ok); 

    //log
    log_request(request, res_, "Health check", "HealthRequestHandler");

    return res_;

}

