#include <string>
#include <map>
#include "request_handler.h"

using CreateRequestHandler = RequestHandler*(*)(const RequestHandlerData&);

class RequestHandlerFactory{
    public:
        static CreateRequestHandler get_factory(std::string name);
        static bool register_handler(const std::string name, CreateRequestHandler factory);
        static std::map<std::string, CreateRequestHandler>& get_map();
};
