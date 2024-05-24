#ifndef CRUD_REQUEST_HANDLER
#define CRUD_REQUEST_HANDLER

#include "request_handler.h"
#include "nlohmann/json.hpp"

#include <memory>
#include <optional>
#include <string>
#include <utility>

class CRUDStore;

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>

using json = nlohmann::json;

class CRUDRequestHandler : public RequestHandler {
    public:
        CRUDRequestHandler(const RequestHandlerData& request_handler_data, std::unique_ptr<CRUDStore> crud_store);
        http::response<http::string_body> handle_request(const http::request<http::string_body>& request);
        static RequestHandler* init(const RequestHandlerData& request_handler_data);
        static bool registered_;
    private:
        void get(int id, std::string entity);
        void del(int id, std::string entity);        
        void post(std::string entity, std::string json);
        void put(int id, std::string entity, std::string json);
    private:
        std::string location_path_;
        std::string data_path_;
        void set_bad_request_response(const std::string& message = "");
        void set_file_not_found_response();
        void set_internal_server_error_response();
        std::optional<json> validate_json(const std::string& json);
        std::optional<std::pair<std::string, int>> extract_elements(const std::string& relative_path);
        std::unique_ptr<CRUDStore> crud_store_;
};

#endif
