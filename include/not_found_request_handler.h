#ifndef NOT_FOUND_REQUEST_HANDLER
#define NOT_FOUND_REQUEST_HANDLER

#include "request_handler.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
    
class NotFoundHandler : public RequestHandler {
    public:
        NotFoundHandler(const RequestHandlerData& request_handler_data);
        http::response<http::string_body> handle_request(const http::request<http::string_body>& request);
        static RequestHandler* init(const RequestHandlerData& request_handler_data);
        static bool registered_;
};

#endif