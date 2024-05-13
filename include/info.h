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
    std::map<std::string, std::string> location_to_root;
};

struct LogInfo {
    std::string request_line;
    AddrInfo addr_info;
    std::string response;
    std::string message; // for additional info
};

struct RequestDispatcherInfo {
    std::string request;
    size_t request_size;
    ConfigInfo config_info;
    AddrInfo addr_info;
};

struct RequestHandlerData{
    std::string root;
    LogInfo log_info;
};