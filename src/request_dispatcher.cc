#include "request_dispatcher.h"
#include "request_handler.h"
#include <regex>
#include <string>
#include <boost/log/trivial.hpp>

RequestDispatcher::RequestDispatcher() {
    // default response, if path is not echo nor static file
    response_ = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\nContent-Length: 0\n\n";
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
    std::string selected_handler = "(no handler found)";
    // std::string root = "";
    std::string curr_longest_match = ""; 

    // BOOST_LOG_TRIVIAL(debug) << info.config_info.static_file_locations.size();

    for (auto const& pair : info.config_info.location_to_handler) {
        // adding a / at the end if it doesn't already exist - prevents matching of half a directory name
        std::string path = pair.first[pair.first.length()-1]=='/' ? pair.first : pair.first+'/';
        std::string compare_file_path = file_path[file_path.length()-1]=='/' ? file_path : file_path+'/';
        if (compare_file_path.find(path) == 0 && path.length() > curr_longest_match.length()) {
            curr_longest_match = pair.first;
            selected_handler = pair.second;
        }
    }
    BOOST_LOG_TRIVIAL(debug) << "Using " << selected_handler;


    RequestHandler* handler;
    LogInfo log_info;
    log_info.addr_info = info.addr_info;
    log_info.request_line = get_first_line(info.request);
    // response empty, will be filled in in handler

    if (selected_handler == "FileRequestHandler") {
        std::string root = info.config_info.location_to_root[curr_longest_match];
        handler = new FileRequestHandler(root+file_path);
        response_ = handler->handle_request(log_info); // trying to move this out, but segfaulting - TODO
    }
    else if (selected_handler == "EchoRequestHandler") {
        handler = new EchoRequestHandler(info.request, &info.request_size);
        response_ = handler->handle_request(log_info);
    }
    else {
        BOOST_LOG_TRIVIAL(warning) << "no valid dispatcher for request";
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