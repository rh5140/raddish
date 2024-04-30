#include <sstream>
#include <string>
#include <iostream>
#include <boost/log/trivial.hpp>
#include "request_handler.h"

echo_request_handler::echo_request_handler(const char* request, size_t* max_bytes) : request_(request), max_bytes_(max_bytes) {
}

std::string echo_request_handler::handle_request() {
    std::string http_response = "HTTP/1.1 200 OK\nContent-Type: text/plain\n";
    std::string content_length = "Content-Length: ";
    std::string response_body;
    
    std::stringstream ss(request_);
    std::string to;
    std::string ret = "";
    while(std::getline(ss, to,'\n')){
        //safeguard in case of buffer overflow
        if(ret.length() + to.length() + 1 <= (*max_bytes_)){
            ret += to + "\n"; 
        }
        else{
            //only add up to bytes read
            ret += to.substr(0, (*max_bytes_) - ret.length()); 
            break;
        }
    }

    response_body = ret;
    content_length = content_length + std::to_string(response_body.size()) + "\n\n"; //+1 is for the extra \n at the end
    http_response = http_response + content_length + response_body;

    BOOST_LOG_TRIVIAL(info) << "Echo response: \n" << http_response;
    return http_response;
}


