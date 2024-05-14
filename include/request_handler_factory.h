#include <string>
#include <gtest/gtest_prod.h>
#include "request_handler.h"
//#include "echo_request_handler.h"
//#include "file_request_handler.h"
//#include "not_found_request_handler.h"


#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
using CreateRequestHandler = RequestHandler*(*)(const RequestHandlerData&);


class RequestHandlerFactory{
    public:
        static CreateRequestHandler get_factory(std::string name);
        static bool register_handler(const std::string name, CreateRequestHandler factory);
        static std::map<std::string, CreateRequestHandler>& get_map();
};
