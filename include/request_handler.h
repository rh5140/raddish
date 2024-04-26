#include <string>

class request_handler {
    public:
        virtual std::string handle_request(const char* request, size_t* max_bytes) = 0;
};

class echo_request_handler : request_handler {
    public:
        std::string handle_request(const char* request, size_t* max_bytes);
};

class file_request_handler : request_handler {
    public:
        std::string handle_request(const char* request, size_t* max_bytes);
};
