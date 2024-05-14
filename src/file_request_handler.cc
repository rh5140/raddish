#include <sstream>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include "file_request_handler.h"
#include "request_handler_factory.h"
#include "info.h"

//temp
#include <boost/log/trivial.hpp>

FileRequestHandler::FileRequestHandler(const RequestHandlerData& request_handler_data) : RequestHandler(request_handler_data){
    root_ = request_handler_data.root;
}

RequestHandler* FileRequestHandler::init(const RequestHandlerData& request_handler_data) {
    return new FileRequestHandler(request_handler_data); 
}
bool FileRequestHandler::registered_ = RequestHandlerFactory::register_handler("FileRequestHandler", FileRequestHandler::init);


http::response<http::string_body> FileRequestHandler::handle_request(const http::request<http::string_body>& request) {

    std::string file_path = root_ + std::string(request.target());
    init_response(request);
    
    std::string content_type = get_content_type(file_path);
    //std::string http_response = "HTTP/1.1 200 OK\nContent-Type: " + content_type + "\n";
    //std::string content_length = "Content-Length: ";
    

    //std::string response_body;
    std::string file_content;
    std::string log_message;

    bool message_ok = false;

    if (std::filesystem::exists(file_path)) {
        std::ifstream file_to_read(file_path, std::ios::in | std::ios::binary); // already reading in as binary

        if (std::filesystem::is_regular_file(file_path) && file_to_read.is_open()) {
            // Read file contents into string reference: https://stackoverflow.com/questions/2912520/read-file-contents-into-a-string-in-c
            file_content.assign((std::istreambuf_iterator<char>(file_to_read)),
                                (std::istreambuf_iterator<char>()));
            file_to_read.close(); // here for clarity, not necessary since ifstream destructor also closes file automatically
            log_message = "Sent file";
            message_ok = true;
        }
        else {
            // 404 path is not file
            log_message = file_path + " is not a file";
        }
    }
    else {
        // 404 error - file does not exist 
        log_message =  "File does not exist at " + file_path;
    }


    //response_body = file_content;
    // Content-Length is size of entity body in decimal number of OCTETS
    //content_length = content_length + std::to_string(response_body.size()) + "\n\n"; //+1 is for the extra \n at the end
    //http_response = http_response + content_length + response_body;

    log_info_.message = log_message;
    log_info_.response = file_content;
    log_request();
    
    //we just use the default if we didn't find the file
    if(message_ok){ 
        res_.body() = file_content;
        //set vars
        res_.result(http::status::ok); 
        res_.set(http::field::content_type, content_type);
    }

    return res_;

    //return http_response;
}

std::string FileRequestHandler::get_content_type(std::string file_path) {
    // unintelligent parsing i think but whatever
    int dot_idx = -1;
    for (int i = file_path.size() - 1; i--; i > 0) {
        if (file_path[i] == '.') {
            dot_idx = i;
            break;
        }
    }
    
    std::string file_extension = file_path.substr(dot_idx+1);
    std::string content_type = file_extension_to_content_type(file_extension);
    return content_type;
}

std::string FileRequestHandler::file_extension_to_content_type(std::string file_extension) {
    if (file_extension == "html") {
        return "text/html";
    }
    else if (file_extension == "txt") {
        return "text/plain";
    }
    else if (file_extension == "jpg" || file_extension == "jpeg") {
        return "image/jpeg";
    }
    else if (file_extension == "png") {
        return "image/png";
    }
    else if (file_extension == "pdf") {
        return "application/pdf";
    }
    else if (file_extension == "zip") {
        return "application/zip";
    }
    else { // for binary files
        return "application/octet-stream";
    }
}
