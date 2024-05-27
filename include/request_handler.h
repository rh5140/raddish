#ifndef REQUEST_HANDLER
#define REQUEST_HANDLER

#include "info.h"

#include <string>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/logger.hpp>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace logging = boost::log;
namespace attrs = boost::log::attributes;
namespace sev_lvl = boost::log::trivial;

class RequestHandler {
    public:
        RequestHandler(const RequestHandlerData& request_handler_data);
        virtual http::response<http::string_body> handle_request(const http::request<http::string_body>& request) = 0;
        //can't put the static init function here for registering, because of how subclassing works.
    protected:
        http::response<http::string_body> res_;
        void init_response(const http::request<http::string_body>& request);
        void log_request(const http::request<http::string_body>& request, const http::response<http::string_body>& response, std::string log_message);
        AddrInfo addr_info_;

        // logger stuff
        std::string handler_name_;
        logging::sources::logger lg_;
        attrs::mutable_constant<std::string> process_;
        attrs::mutable_constant<sev_lvl::severity_level> severity_;
        void log_machine(int status, std::string target);
};

#endif