#ifndef REQUEST_HANDLER_FACTORY
#define REQUEST_HANDLER_FACTORY

#include "request_handler.h"

#include <map>
#include <string>

using CreateRequestHandler = RequestHandler*(*)(const RequestHandlerData&);

class RequestHandlerFactory{
    public:
        static CreateRequestHandler get_factory(std::string name);
        static bool register_handler(const std::string name, CreateRequestHandler factory);
        static std::map<std::string, CreateRequestHandler>& get_map();
};

#endif