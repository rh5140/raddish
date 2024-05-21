#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <string>
#include "info.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>

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
};

