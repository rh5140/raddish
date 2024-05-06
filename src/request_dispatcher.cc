#include "request_dispatcher.h"
#include "request_handler.h"
#include <regex>
#include <string>
#include <boost/log/trivial.hpp>

RequestDispatcher::RequestDispatcher() {
    // default response, if path is not echo nor static file
    response_ = "HTTP/1.1 400 Bad Request\nContent-Type: text/plain\nContent-Length: 0\n\n";
}

std::string RequestDispatcher::dispatch_request(RequestDispatcherInfo info) {

    BOOST_LOG_TRIVIAL(debug) << "Request received: \n" << info.request.substr(0, info.request_size);

    if (!is_valid_request(info.request)) {
        BOOST_LOG_TRIVIAL(warning) << "invalid request";
        return response_;
    }
    
    BOOST_LOG_TRIVIAL(debug) << "valid request";

    std::string file_path = get_path(info.request);
    BOOST_LOG_TRIVIAL(debug) << file_path;

    // compare with list of locations parsed from config
    Handlers selected_handler;
    std::string root = "";
    std::string curr_longest_match = ""; 

    BOOST_LOG_TRIVIAL(debug) << info.config_info.static_file_locations.size();

    for (auto const& pair : info.config_info.static_file_locations){
        // checking whether location is a substring of file_path, and starts at index 0
        if (file_path.find(pair.first) == 0 && pair.first.length() > curr_longest_match.length()) {
            BOOST_LOG_TRIVIAL(debug) << "Using static path";
            root = pair.second;
            curr_longest_match = pair.first;
            selected_handler = static_file;
        }
    }
    for (auto const& location : info.config_info.echo_locations){
        if (file_path.find(location) == 0 && location.length() > curr_longest_match.length()) {
            BOOST_LOG_TRIVIAL(debug) << "Using echo path";
            curr_longest_match = location;
            selected_handler = echo;
        }
    }

    RequestHandler* handler;
    LogInfo log_info;
    log_info.addr_info = info.addr_info;
    log_info.request_line = get_first_line(info.request);
    // response empty, will be filled in in handler

    switch(selected_handler) {
        case static_file:
            handler = new FileRequestHandler(root + file_path);
            response_ = handler->handle_request(log_info); // trying to move this out, but segfaulting - TODO
            break;
        case echo:
            handler = new EchoRequestHandler(info.request, &info.request_size);
            response_ = handler->handle_request(log_info); 
            break;
        default:
            BOOST_LOG_TRIVIAL(warning) << "no valid dispatcher for request";
            break;
    }
        
    return response_;
}

// currently only checks first line essentially
bool RequestDispatcher::is_valid_request(std::string request) {
    std::regex request_regex("GET \/.* HTTP\/1\.1(\n|\r\n)(.+:.+(\n|\r\n))*(\n|\r\n).*", std::regex::extended);
    return std::regex_search(request, request_regex);
}

std::string RequestDispatcher::get_first_line(std::string request) {
    std::stringstream ss(request);
    std::string first_line;
    std::getline(ss, first_line, '\n'); // get the first line
    return first_line.substr(0, first_line.length()-1); // remove trailing newline
}

std::string RequestDispatcher::get_path(std::string request) {
    std::string first_line = get_first_line(request);

    bool foundSpace = false;
    std::string file_path = "";

    // extract path
    for (int i = 0; i<first_line.length(); i++) {
        if (first_line[i] == ' ') 
            if (!foundSpace)
            {
                foundSpace = true;
                i++;
            }
            else 
                break;
        if (foundSpace) {
            file_path += first_line[i];
        }
    }

    return file_path;
}