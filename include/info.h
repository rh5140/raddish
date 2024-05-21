#ifndef INFO
#define INFO
#include <string>
#include <map>

// holds structs for use in other files

struct ConfigInfo {
    int port_num;
    std::map<std::string, std::string> location_to_handler;
    std::map<std::string, std::map<std::string, std::string>> location_to_directives; // e.g. /images/ -> (root -> /usr/src/projects...)
};


//host/client info used to generate responses in request handlers.
struct AddrInfo {
    std::string host_addr;
    std::string client_addr;
};

//used for request handler initialization
struct RequestHandlerData{
    std::string location_path;
    std::string root;
    std::string data_path;
    AddrInfo addr_info;
};

#endif