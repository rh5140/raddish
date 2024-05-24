#ifndef ECHO_REQUEST_HANDLER
#define ECHO_REQUEST_HANDLER

#include "request_handler.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
    

class EchoRequestHandler : public RequestHandler {
    public:
        EchoRequestHandler( const RequestHandlerData& request_handler_data);
        http::response<http::string_body> handle_request(const http::request<http::string_body>& request);
        static RequestHandler* init(const RequestHandlerData& request_handler_data); //can't put these in parent because they're static.
        static bool registered_;
    private:
};

#endif