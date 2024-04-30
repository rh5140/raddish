#include <sstream>
#include <string>
#include <iostream>
#include <boost/log/trivial.hpp>
#include "request_handler.h"

std::string echo_request_handler::handle_request(const char* request, size_t* max_bytes) {
    std::string http_response = "HTTP/1.1 200 OK\nContent-Type: text/plain\n";
    std::string content_length = "Content-Length: ";
    std::string response_body;
    
    std::stringstream ss(request);
    std::string to;
    std::string ret = "";
    while(std::getline(ss, to,'\n')){
        //safeguard in case of buffer overflow
        if(ret.length() + to.length() + 1 <= (*max_bytes)){
            ret += to + "\n"; 
        }
        else{
            //only add up to bytes read
            ret += to.substr(0, (*max_bytes) - ret.length()); 
            break;
        }
    }

    response_body = ret;
    content_length = content_length + std::to_string(response_body.size()) + "\n\n"; //+1 is for the extra \n at the end
    http_response = http_response + content_length + response_body;

    BOOST_LOG_TRIVIAL(info) << http_response;
    return http_response;
}


