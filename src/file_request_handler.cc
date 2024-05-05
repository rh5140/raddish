#include <sstream>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include "request_handler.h"
#include "info.h"

FileRequestHandler::FileRequestHandler(std::string file_path) {
    file_path_ = file_path;
}

std::string FileRequestHandler::handle_request(LogInfo log_info) {
    std::string content_type = get_content_type(file_path_);
    std::string http_response = "HTTP/1.1 200 OK\nContent-Type: " + content_type + "\n";
    std::string content_length = "Content-Length: ";
    std::string response_body;
    std::string file_content;
    std::string message;

    if (std::filesystem::exists(file_path_)) {
        std::ifstream file_to_read(file_path_, std::ios::in | std::ios::binary); // already reading in as binary

        if (std::filesystem::is_regular_file(file_path_) && file_to_read.is_open()) {
            // Read file contents into string reference: https://stackoverflow.com/questions/2912520/read-file-contents-into-a-string-in-c
            file_content.assign((std::istreambuf_iterator<char>(file_to_read)),
                                (std::istreambuf_iterator<char>()));
            file_to_read.close(); // here for clarity, not necessary since ifstream destructor also closes file automatically
            message = "Sent file";
        }
        else {
            // 404 path is not file
            message = file_path_ + " is not a file";
            http_response = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\n";
            file_content = "404 Not Found";
        }
    }
    else {
        // 404 error - file does not exist 
        message =  "File does not exist at " + file_path_;
        http_response = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\n";
        file_content = "404 Not Found";
    }


    response_body = file_content;
    // Content-Length is size of entity body in decimal number of OCTETS
    content_length = content_length + std::to_string(response_body.size()) + "\n\n"; //+1 is for the extra \n at the end
    http_response = http_response + content_length + response_body;

    log_info.message = message;
    log_info.response = http_response;
    log_request(log_info);
    
    return http_response;
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
