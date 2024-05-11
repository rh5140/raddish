#include <string>
#include <gtest/gtest_prod.h>
#include "info.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>


class RequestHandler {
    public:
        RequestHandler(http::request<http::string_body> request);
        virtual http::response<http::string_body> handle_request(LogInfo log_info) = 0;
    protected:
        http::request<http::string_body> req_;
        http::response<http::string_body> res_;
        void log_request(LogInfo log_info);
};

class EchoRequestHandler : public RequestHandler {
    public:
        EchoRequestHandler(http::request<http::string_body> request);
        http::response<http::string_body> handle_request(LogInfo log_info);
    private:
        std::string echo_req_;
        size_t max_bytes_;
};

class FileRequestHandler : public RequestHandler {
    public:
        FileRequestHandler(http::request<http::string_body> request, std::string root);
        http::response<http::string_body> handle_request(LogInfo log_info);
    private:
        std::string file_path_;

        std::string get_content_type(std::string file_path);
        std::string file_extension_to_content_type(std::string file_extension);

        FRIEND_TEST(FileRequestTest, Extensions);
};
