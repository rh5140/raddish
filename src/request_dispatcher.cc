#include "request_dispatcher.h"
#include "echo_request_handler.h"
#include "request_handler_factory.h"

#include <regex>
#include <string>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>

using namespace std;

using CreateRequestHandler = RequestHandler*(*)(const RequestHandlerData&);

RequestDispatcher::RequestDispatcher() : process_("Req Dispatch"), severity_(sev_lvl::info) {
    lg_.add_attribute("Process", process_);
    lg_.add_attribute("Severity", severity_);
}

http::response<http::string_body> RequestDispatcher::dispatch_request(http::request<http::string_body> req, ConfigInfo config_info, std::string host, std::string client){
    process_.set(string(req.method_string()) + " " + string(req.target()) + " - Req Dispatch");

    severity_.set(sev_lvl::debug);
    BOOST_LOG(lg_) << "Received " << req.method_string() << " request from " << client;
    severity_.set(sev_lvl::trace);
    BOOST_LOG(lg_) << "Full HTTP request object :\n" << req;

    if (!is_valid_request(req)) {
        response_ = http::response<http::string_body>{http::status::bad_request, req.version()};
        response_.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        response_.set(http::field::content_type, "text/plain");
        response_.body() = "400 Bad Request"; //removed /n for consistancy
        
        severity_.set(sev_lvl::warning);
        BOOST_LOG(lg_) 
        << "Status 400 Bad Request -"
        << " Client" + client 
        << " requests \"" + req.method_string().to_string() + " " + req.target().to_string() + " HTTP/" + std::to_string(req.version() / 10) + "." + std::to_string(req.version() % 10) + "\"";
        
        process_.set("ResponseMetrics");
        severity_.set(sev_lvl::info);
        BOOST_LOG(lg_) 
        << "response_code:" << 400 
        << " request_path:" << std::string(req.target())
        << " request_ip:" << client 
        << " request_handler_name:" << "BadRequestHandler";
        
        
        return response_;
    }
    
    //file path is now just "req.target()"
    std::string file_path = std::string(req.target());
    severity_.set(sev_lvl::trace);
    BOOST_LOG(lg_) << "Path of request : " << file_path;

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

    //generate handler data
    RequestHandlerData request_handler_data;
    std::string root = config_info.location_to_directives[curr_longest_match]["root"]; // empty if DNE
    request_handler_data.root = root;
    request_handler_data.data_path = config_info.location_to_directives[curr_longest_match]["data_path"];
    request_handler_data.location_path = curr_longest_match;
    AddrInfo addr_info;
    addr_info.host_addr = host;
    addr_info.client_addr = client;
    request_handler_data.addr_info = addr_info;

    //create handler and call

    RequestHandler* handler = handler_factory(request_handler_data);
    
    response_ = handler->handle_request(req);
    delete handler; //handlers are short lived.

    //return res object
    return response_;
}

// currently only checks first line essentially
bool RequestDispatcher::is_valid_request(http::request<http::string_body> req) {
    bool is_valid; 
    std::ostringstream oss;
    oss << req;
    std::string str_req = oss.str();
    std::regex request_regex("(GET|POST|PUT|DELETE) \/.* HTTP\/1\.1(\n|\r\n)(.+:.+(\n|\r\n))*(\n|\r\n).*", std::regex::extended);
    is_valid = std::regex_search(str_req, request_regex);
    severity_.set(sev_lvl::trace);
    BOOST_LOG(lg_) << "Request Valid? " << is_valid ? "true" : "false";
    return is_valid;
}
