#include <sstream>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <boost/log/trivial.hpp>
#include "request_handler.h"

file_request_handler::file_request_handler(std::string file_path, std::string root) {
    file_path_ = file_path;
    root_ = root;
}

// Note: neither parameter is used
std::string file_request_handler::handle_request(const char* request, size_t* max_bytes) {
    std::string content_type = get_content_type(file_path_);
    std::string http_response = "HTTP/1.1 200 OK\nContent-Type: " + content_type + "\n";
    std::string content_length = "Content-Length: ";
    std::string response_body;
    
    // TODO: Don't hardcode? Actually maybe ok just for the 404 error since it's not being pulled from config.
    std::string error_file_path = "/static_files/404.html"; // DOCKER VERSION
    error_file_path = "/usr/src/projects/raddish/static_files/404.html"; // LOCAL VERSION, COMMENT OUT BEFORE DEPLOY

    // Docker version 
    std::string prepend_path = root_; // DOCKER VERSION
    prepend_path = "/usr/src/projects/raddish" + root_; // LOCAL VERSION, COMMENT OUT BEFORE DEPLOY
    std::string full_path = prepend_path + file_path_;
    std::ifstream file_to_read(full_path, std::ios::in | std::ios::binary); // already reading in as binary
    std::string file_content;

    if (file_to_read.is_open()) {
        // Read file contents into string reference: https://stackoverflow.com/questions/2912520/read-file-contents-into-a-string-in-c
        file_content.assign((std::istreambuf_iterator<char>(file_to_read)),
                            (std::istreambuf_iterator<char>()));
        file_to_read.close(); // here for clarity, not necessary since ifstream destructor also closes file automatically
    }
    else {
        // 404 error
        // DOCKER FILEPATH
        std::ifstream error_404_file(error_file_path, std::ios::in | std::ios::binary);
        http_response = "HTTP/1.1 404 Not Found\nContent-Type: text/html\n";
        file_content.assign((std::istreambuf_iterator<char>(error_404_file)),
                            (std::istreambuf_iterator<char>()));
        error_404_file.close(); // here for clarity, not necessary since ifstream destructor also closes file automatically
    }

    response_body = file_content;
    // Content-Length is size of entity body in decimal number of OCTETS
    content_length = content_length + std::to_string(response_body.size()) + "\n\n"; //+1 is for the extra \n at the end
    http_response = http_response + content_length + response_body;

    // TODO - probably remove and rely on logging later
    BOOST_LOG_TRIVIAL(info) << http_response;
    return http_response;
}

std::string file_request_handler::get_content_type(std::string file_path) {
    // unintelligent parsing i think but whatever
    int dot_idx;
    for (int i = file_path.size() - 1; i--; i > 0) {
        if (file_path[i] == '.') {
            dot_idx = i;
            break;
        }
    }

    std::string file_extension = file_path.substr(dot_idx+1);
    std::cout << "file extension: " << file_extension << std::endl;
    std::string content_type = file_extension_to_content_type(file_extension);
    std::cout << "content type: " << content_type << std::endl;
    return content_type;
}

std::string file_request_handler::file_extension_to_content_type(std::string file_extension) {
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
