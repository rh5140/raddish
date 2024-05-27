#include "not_found_request_handler.h"
#include "request_handler_factory.h"

namespace beast = boost::beast;  
namespace http = beast::http;    

// calls parent constructor
// can be used to set handler-specific variables - e.g. the root for file handler.
// all data needed should be added to the RequestHandlerData struct contained in info.h.
NotFoundHandler::NotFoundHandler(const RequestHandlerData& request_handler_data) : RequestHandler(request_handler_data) {
    handler_name_ = "NotFoundHandler";
}

// factory function
// defined as a static function in the header - added to registry map on startup.
RequestHandler* NotFoundHandler::init(const RequestHandlerData& request_handler_data) {
    return new NotFoundHandler(request_handler_data); 
}

// add self-init to registry on startup. 
// defined as a static bool in the header file.
bool NotFoundHandler::registered_ = RequestHandlerFactory::register_handler("NotFoundHandler", NotFoundHandler::init);

// implementation of handle_request
http::response<http::string_body> NotFoundHandler::handle_request(const http::request<http::string_body>& request) {
    init_response(request);
    //set vars
    res_.result(http::status::not_found);  //technically already done in init_response, but done here for clarity. 
    //log
    log_request(request, res_, "Path not configured");
    return res_;
}