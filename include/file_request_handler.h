#ifndef FILE_REQUEST_HANDLER
#define FILE_REQUEST_HANDLER

#include <string>
#include <gtest/gtest_prod.h>
#include "request_handler.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
    

class FileRequestHandler : public RequestHandler {
    public:
        FileRequestHandler(const RequestHandlerData& request_handler_data);
        http::response<http::string_body> handle_request(const http::request<http::string_body>& request);
        static RequestHandler* init(const RequestHandlerData& request_handler_data);
        static bool registered_;
    private:
        std::string root_;
        //std::string file_path_;
        std::string get_content_type(std::string file_path);
        std::string file_extension_to_content_type(std::string file_extension);
        FRIEND_TEST(FileRequestTest, Extensions);
};

#endif