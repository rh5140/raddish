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
        file_request_handler(std::string file_path);
        std::string handle_request(const char* request, size_t* max_bytes);
    private:
        std::string file_path_;

        std::string get_content_type(std::string file_path);
        std::string file_extension_to_content_type(std::string file_extension);
};
