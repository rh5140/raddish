#include "request_dispatcher.h"
#include "echo_request_handler.h"
#include "request_handler_factory.h"
//#include "request_handler.h"
#include <regex>
#include <string>
#include <boost/log/trivial.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
using namespace std;


using CreateRequestHandler = RequestHandler*(*)(http::request<http::string_body>, RequestHandlerData);

RequestDispatcher::RequestDispatcher() {
    // default response, if path is not echo nor static file
    // moved default to dispatch_request because we need the req.
}

http::response<http::string_body> RequestDispatcher::dispatch_request(http::request<http::string_body> req, ConfigInfo config_info, std::string host, std::string client){

    BOOST_LOG_TRIVIAL(debug) << "Request received: \n" << req; //info.request.substr(0, info.request_size);
    BOOST_LOG_TRIVIAL(debug) << req.target(); 

    //TODO: fix this - not checking if it's valid yet.

    if (!is_valid_request(req)) {
        BOOST_LOG_TRIVIAL(warning) << "invalid request";
        response_ = http::response<http::string_body>{http::status::not_found, req.version()};
        response_.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        response_.set(http::field::content_type, "text/html");
        response_.body() = "404 Not Found\n"; //for clarity, added \n
        return response_;
    }


    
    BOOST_LOG_TRIVIAL(debug) << "valid request";


    //file path is now just "req.target()"
    std::string file_path = std::string(req.target());
    BOOST_LOG_TRIVIAL(debug) << file_path;

    //factory pointer - "CreateRequestHandler" is just a shorthand for the function pointer.
    CreateRequestHandler handler_factory;
    std::string curr_longest_match = ""; 
    

    for (auto const& pair : config_info.location_to_handler) {
        // adding a / at the end if it doesn't already exist - prevents matching of half a directory name
        std::string path = pair.first[pair.first.length()-1]=='/' ? pair.first : pair.first+'/';
        std::string compare_file_path = file_path[file_path.length()-1]=='/' ? file_path : file_path+'/';
        if (compare_file_path.find(path) == 0 && path.length() > curr_longest_match.length()) {
            curr_longest_match = pair.first;
            handler_factory = RequestHandlerFactory::get_factory(pair.second); //get factory
        }
    }

    //RequestHandler* handler;
    LogInfo log_info;
    log_info.addr_info.host_addr = host;
    log_info.addr_info.client_addr = client;
    //log_info.request_line = get_first_line(info.request);
    log_info.request_line = "test";
    // response empty, will be filled in in handler

    //generate handler data
    RequestHandlerData requestHandlerData;
    std::string root = config_info.location_to_root[curr_longest_match];
    requestHandlerData.root = root;

    //create handler and call
    RequestHandler* handler = handler_factory(req, requestHandlerData);
    response_ = handler->handle_request();
    delete handler; //handlers are short lived.

    //return res object
    return response_;
}

// currently only checks first line essentially
bool RequestDispatcher::is_valid_request(http::request<http::string_body> req) {
    std::ostringstream oss;
    oss << req;
    std::string str_req = oss.str();
    BOOST_LOG_TRIVIAL(debug) << str_req;
    std::regex request_regex("GET \/.* HTTP\/1\.1(\n|\r\n)(.+:.+(\n|\r\n))*(\n|\r\n).*", std::regex::extended);
    return std::regex_search(str_req, request_regex);
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