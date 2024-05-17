#ifndef CRUD_REQUEST_HANDLER
#define CRUD_REQUEST_HANDLER

#include <string>
#include <optional>
#include <utility>
#include <memory>
#include <gtest/gtest_prod.h>
#include "request_handler.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

class CRUDStore;

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>


class CRUDRequestHandler : public RequestHandler {
    public:
        CRUDRequestHandler(const RequestHandlerData& request_handler_data, std::unique_ptr<CRUDStore> crud_store);
        http::response<http::string_body> handle_request(const http::request<http::string_body>& request);
        static RequestHandler* init(const RequestHandlerData& request_handler_data);
        static bool registered_;
    private:
        void get(int id, std::string entity, std::string json);
        void post(int id, std::string entity, std::string json);
        void put(int id, std::string entity, std::string json);
        void del(int id, std::string entity, std::string json);
    private:
        std::string location_path_;
        std::string data_path_;
        void set_bad_request_response(std::string message);
        std::optional<std::pair<std::string, int>> extract_elements(const std::string& relative_path);
        std::unique_ptr<CRUDStore> crud_store_;
        // FRIEND_TEST(CRUDRequestTest, Extensions); <- will be helpful for testing?
};

#endif
