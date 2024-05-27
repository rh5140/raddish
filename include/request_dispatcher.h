#ifndef REQUEST_DISPATCHER
#define REQUEST_DISPATCHER

#include "info.h"

#include <string>
#include <boost/beast/http.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/trivial.hpp> 
#include <boost/log/sources/logger.hpp> 

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace logging = boost::log;
namespace attrs = boost::log::attributes;
namespace sev_lvl = boost::log::trivial;

class RequestDispatcher {
    public:
        RequestDispatcher();
        http::response<http::string_body> dispatch_request(http::request<http::string_body> req, ConfigInfo config_info, std::string host, std::string client);

    private:
        bool is_valid_request(http::request<http::string_body> req);
        std::string get_first_line(std::string request);
        std::string get_path(std::string request);
        http::response<http::string_body> response_;
        std::string host_;
        std::string client_;

        logging::sources::logger lg_;
        attrs::mutable_constant<std::string> process_;
        attrs::mutable_constant<sev_lvl::severity_level> severity_;

};

#endif