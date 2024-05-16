#include <string>
#include <filesystem>
#include <stdexcept>
#include <regex>
#include "crud_request_handler.h"
#include "request_handler_factory.h"
#include "info.h"

#include <boost/log/trivial.hpp>

CRUDRequestHandler::CRUDRequestHandler(const RequestHandlerData& request_handler_data) :
    RequestHandler(request_handler_data), data_path_(request_handler_data.data_path), location_path_(request_handler_data.location_path) {
    if (location_path_[location_path_.length()-1] == '/')
        location_path_.erase(location_path_.length()-1, 1);
}

RequestHandler* CRUDRequestHandler::init(const RequestHandlerData& request_handler_data) {
    std::filesystem::path data_path = std::filesystem::path(request_handler_data.data_path);
    if (!std::filesystem::is_directory(data_path))
        throw std::runtime_error("Given data path for CRUDRequestHandler is not a directory");
    return new CRUDRequestHandler(request_handler_data);
}

bool CRUDRequestHandler::registered_ = RequestHandlerFactory::register_handler("CRUDRequestHandler", CRUDRequestHandler::init);

http::response<http::string_body> CRUDRequestHandler::handle_request(const http::request<http::string_body>& request) {
    // List of the boost::beast::http verbs can be seen here https://www.boost.org/doc/libs/1_84_0/boost/beast/http/verb.hpp
    http::verb method = request.method();
    std::string log_message;

    std::string relative_path = request.target().to_string().substr(location_path_.length());
    std::optional<std::pair<std::string, int>> elements = extract_elements(relative_path);
    if (!elements.has_value()) {
        log_message = "Invalid request format for CRUD API";
        set_bad_request_response(log_message);
        log_request(request, res_, log_message);
        return res_;
    }
    std::string entity = elements.value().first;
    int id = elements.value().second;

    switch (method) {
    case http::verb::get:
        break;
    case http::verb::post:
        if (id != -1) {
            log_message = "POST requests cannot specify an entity ID";
            set_bad_request_response(log_message);
            break;
        }
        // res_ = crud_guy_.post(request)
        break;
    case http::verb::put:
        if (id == -1) {
            log_message = "PUT requests must specify an entity ID";
            set_bad_request_response(log_message);
            break;
        }
        // res_ = crud_guy_.put(request)
        break;
    case http::verb::delete_:
        if (id == -1) {
            log_message = "DELETE requests must specify an entity ID";
            set_bad_request_response(log_message);
            break;
        }
        // res_ = crud_guy_.delete(request)
        break;
    default:
        // Currently it is impossible to get here because of the blanked
        // ban on all methods other than GET POST PUT and DELETE within the
        // dispatcher but you know what they say about code that's impossible
        // to execute...
        boost::beast::string_view sv = request.method_string();
        std::string method_string = std::string(sv.data(), sv.size());
        log_message = "[" + method_string + "] Method Not Allowed";
        res_.body() = log_message;
        res_.result(http::status::method_not_allowed);
        res_.set(http::field::content_type, "text/plain");
        break;
    }
    log_request(request, res_, log_message);
    return res_;
}

void CRUDRequestHandler::set_bad_request_response(std::string message) {
    res_.result(http::status::bad_request);
    res_.body() = message;
    res_.set(http::field::content_type, "text/plain");
}

std::optional<std::pair<std::string, int>> CRUDRequestHandler::extract_elements(const std::string& relative_path) {
    // Only allows urls in the form /*[Entity](/[ID])? where
    // [ENTITY] can be any word with upper or lowercase letters
    // [ID] can be any number of digits so long as there is at least 1,
    // and an ID must be preceded by a / if it exists. (note ID will be cast to int so beware of overflow)
    // The [ENTITY] and [ID] are returned as an optional pair
    std::regex pattern(R"(/*([A-Za-z]+)(/(\d+))?$)");
    std::smatch matches;

    std::string entity;
    int id = -1;
    // In case the URL doesn't adhere to the above requirements we should fail early so we return nullopt and check in calling function to set bad request (400)
    if (!std::regex_search(relative_path, matches, pattern)) {
        return std::nullopt;
    }
    entity = matches[1];
    if (matches[2].matched)
        id = std::stoi(matches[3]);
    return std::make_pair(entity, id);
}
