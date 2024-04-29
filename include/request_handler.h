#include <string>
#include <gtest/gtest_prod.h>

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
        file_request_handler(std::string file_path, std::string root);
        // NOTE: Neither parameters are actually used but currently needs to be like this cuz of the virtual method
        std::string handle_request(const char* request, size_t* max_bytes);
    private:
        std::string file_path_;
        std::string root_;

        std::string get_content_type(std::string file_path);
        std::string file_extension_to_content_type(std::string file_extension);

        FRIEND_TEST(FileRequestTest, Extensions);
};
