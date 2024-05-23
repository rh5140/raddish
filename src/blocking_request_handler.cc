#include <cstdint>
#include <sstream>
#include <string>
#include <iostream>
#include <unistd.h> 
#include "blocking_request_handler.h"
#include "request_handler_factory.h"
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
namespace beast = boost::beast;  
namespace http = beast::http;    
//temp
#include <boost/log/trivial.hpp>



// calls parent constructor
// can be used to set handler-specific variables - e.g. the root for file handler.
// all data needed should be added to the RequestHandlerData struct contained in info.h.
BlockingRequestHandler::BlockingRequestHandler(const RequestHandlerData& request_handler_data) : RequestHandler(request_handler_data){
}

// factory function
// defined as a static function in the header - added to registry map on startup.
RequestHandler* BlockingRequestHandler::init(const RequestHandlerData& request_handler_data) {
    return new BlockingRequestHandler(request_handler_data); 
}


// add self-init to registry on startup. 
// defined as a static bool in the header file.
bool BlockingRequestHandler::registered_ = RequestHandlerFactory::register_handler("BlockingRequestHandler", BlockingRequestHandler::init);


// implementation of handle_request
http::response<http::string_body> BlockingRequestHandler::handle_request(const http::request<http::string_body>& request) {
    sleep(1);
    init_response(request);
    //set vars
    res_.result(http::status::ok); 
    res_.body() = "Returned response after 1 seconds"; 
    //log
    log_request(request, res_, res_.body());
    return res_;
}