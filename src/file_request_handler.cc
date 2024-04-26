#include <sstream>
#include <string>
#include <iostream>
#include "request_handler.h"

std::string file_request_handler::handle_request(const char* request, size_t* max_bytes) {
    // TODO - append type in http response, possibly alter in other ways
    std::string http_response = "HTTP/1.1 200 OK\nContent-Type: REPLACE WITH CORRECT TYPE\n";
    std::string content_length = "Content-Length: ";
    std::string response_body;
    
    // TODO - process request, attach file (remember to set/grab location from config)
    // see echo request handler for inspiration

    response_body = "TODO your response";
    content_length = content_length + std::to_string(response_body.size()) + "\n\n"; //+1 is for the extra \n at the end
    http_response = http_response + content_length + response_body;

    // TODO - probably remove and rely on logging later
    std::cout << http_response << std::endl;
    return http_response;
}


