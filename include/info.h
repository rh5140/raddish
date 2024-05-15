#pragma once
#include <string>
#include <map>
#include <vector>


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
    std::string root;
    AddrInfo addr_info;
};