#pragma once
#include <string>
#include <map>
#include <vector>


// holds structs for use in other files

struct AddrInfo {
    std::string host_addr;
    std::string client_addr;
};

struct ConfigInfo {
    int port_num;
    std::map<std::string, std::string> location_to_handler;
    std::map<std::string, std::map<std::string, std::string>> location_to_directives; // e.g. /images/ -> (root -> /usr/src/projects...)
};

struct RequestHandlerData{
    std::string root;
    AddrInfo addr_info;
};