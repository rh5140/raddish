
#include "request_handler.h"

#include <cstdint>
#include <sstream>
#include <string>
#include <iostream>
#include <boost/lexical_cast.hpp>

namespace beast = boost::beast;  
namespace http = beast::http; 

RequestHandler::RequestHandler(const RequestHandlerData& request_handler_data) : process_("Unknown - Handler"), severity_(sev_lvl::info) {
    addr_info_ = request_handler_data.addr_info;
    lg_.add_attribute("Process", process_);
    lg_.add_attribute("Severity", severity_);
}

//call in each handler - creates the response object and initializes it to a 404.
//means that you can just early return on fail.
void RequestHandler::init_response(const http::request<http::string_body>& request){
    // set logs 
    process_.set(std::string(request.method_string()) + " " + std::string(request.target()) + " - " + handler_name_);

    //generate response object
    res_ = http::response<http::string_body>{http::status::not_found, request.version()};
    res_.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res_.set(http::field::content_type, "text/plain");

    //default error
    res_.body() = "404 Not Found";
    //can add more args as needed
}

//logs client/server ip, result, etc.
void RequestHandler::log_request(const http::request<http::string_body>& request, const http::response<http::string_body>& response, std::string log_message) {    
    std::string req_header = boost::lexical_cast<std::string>(request.base());
    std::string request_line_str = "\"" + req_header.substr(0, req_header.find('\n')-1) + "\"";

    int status = response.result_int();

    // can add more cases e.g. 204 for no content
    switch (status) {
        case (200):
            severity_.set(sev_lvl::info);
            break;
        case (500):
            severity_.set(sev_lvl::error);
            break;
        default:
            severity_.set(sev_lvl::warning);
            
    }
    severity_.set(sev_lvl::info);
    BOOST_LOG(lg_) << "Status " << status << " " << log_message << " - Client " << addr_info_.client_addr << " requests "<< request_line_str;
    log_machine(status, std::string(request.target()));

    std::string res_header = boost::lexical_cast<std::string>(response.base());
    std::string res_body = boost::lexical_cast<std::string>(response.body());
    if (response[http::field::content_type].to_string() != "text/plain") { 
        res_body = "Binary of " + std::string(request.target()); // for clarity in logs
    }
    
    severity_.set(sev_lvl::trace);
    BOOST_LOG(lg_) << "Full HTTP response :\n" << res_header << res_body;
}

void RequestHandler::log_machine(int status, std::string target) {
    logging::sources::logger lg;
    lg.add_attribute("Process", attrs::mutable_constant<std::string>("ResponseMetrics"));
    lg.add_attribute("Severity", attrs::mutable_constant<sev_lvl::severity_level>(sev_lvl::info));
    BOOST_LOG(lg) 
        << "response_code:" << status 
        << " request_path:" << target
        << " request_ip:" << addr_info_.client_addr 
        << " request_handler_name:" << handler_name_;
}