#include <string>
#include "crud_request_handler.h"
#include "request_handler_factory.h"
#include "info.h"

#include <boost/log/trivial.hpp>

CRUDRequestHandler::CRUDRequestHandler(const RequestHandlerData& request_handler_data) :
    RequestHandler(request_handler_data), data_path_(request_handler_data.data_path) {}

RequestHandler* CRUDRequestHandler::init(const RequestHandlerData& request_handler_data) {
    return new CRUDRequestHandler(request_handler_data);
}

bool CRUDRequestHandler::registered_ = RequestHandlerFactory::register_handler("CRUDRequestHandler", CRUDRequestHandler::init);

http::response<http::string_body> CRUDRequestHandler::handle_request(const http::request<http::string_body>& request) {
    http::verb method = request.method();
    // List of the boost::beast::http verbs can be seen here https://www.boost.org/doc/libs/1_84_0/boost/beast/http/verb.hpp
    std::string body;
    // We could even have a more generic crud_guy_.process(method, request)
    switch (method) {
    case http::verb::get:
        // res_ = crud_guy_.get(request);
        break;
    case http::verb::post:
        // res_ = crud_guy_.post(request)
        break;
    case http::verb::put:
        // res_ = crud_guy_.put(request)
        break;
    case http::verb::delete_:
        // res_ = crud_guy_.delete(request)
        break;
    default:
        // Currently it is impossible to get here because of the blanked
        // ban on all methods other than GET POST PUT and DELETE within the
        // dispatcher but you know what they say about code that's impossible
        // to execute...
        boost::beast::string_view sv = request.method_string();
        std::string method_string = std::string(sv.data(), sv.size());
        res_.body() = "[" + method_string + "] Method Not Allowed";
        res_.result(http::status::method_not_allowed);
        res_.set(http::field::content_type, "text/plain");
        break;
    }
    return res_;
}
