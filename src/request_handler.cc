
#include "request_handler.h"
#include "info.h"
#include <boost/log/trivial.hpp>


void RequestHandler::log_request(LogInfo log_info) {
    std::string client_str = ", Client: " + log_info.addr_info.client_addr;
    std::string host_str = ", Host: " + log_info.addr_info.host_addr;
    std::string request_line_str = ", Request: \"" + log_info.request_line + "\"";

    int status = 404;
    try {
        // extract status code from response
        std::string http_ver = "HTTP/1.1 ";
        status = std::stoi(log_info.response.substr(http_ver.length(), http_ver.length()+3));
    }
    catch(...) {
        BOOST_LOG_TRIVIAL(error) << "cannot find status code in response - invalid response";
    }

    // can add more cases e.g. 204 for no content
    switch (status) {
        case (200):
            BOOST_LOG_TRIVIAL(info) << std::to_string(status) + client_str + host_str + request_line_str + " " + log_info.message;
            break;
        default:
            BOOST_LOG_TRIVIAL(warning) << std::to_string(status) + client_str + host_str + request_line_str + " " + log_info.message;
    }

    BOOST_LOG_TRIVIAL(trace) << log_info.message << " " << log_info.response;

}
