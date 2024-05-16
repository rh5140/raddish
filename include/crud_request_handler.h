#ifndef CRUD_REQUEST_HANDLER
#define CRUD_REQUEST_HANDLER

#include <string>
#include <gtest/gtest_prod.h>
#include "request_handler.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>


class CRUDRequestHandler : public RequestHandler {
    public:
        CRUDRequestHandler(const RequestHandlerData& request_handler_data);
        http::response<http::string_body> handle_request(const http::request<http::string_body>& request);
        static RequestHandler* init(const RequestHandlerData& request_handler_data);
        static bool registered_;
    private:
        std::string data_path_;
        // FRIEND_TEST(CRUDRequestTest, Extensions); <- will be helpful for testing?
};

#endif
