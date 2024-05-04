#include <string>
#include <gtest/gtest_prod.h>

class request_handler {
    public:
        virtual std::string handle_request(std::string& return_msg) = 0;
};

class echo_request_handler : request_handler {
    public:
        echo_request_handler(const char* request, size_t* max_bytes);
        std::string handle_request(std::string& return_msg);
    private:
        const char* request_;
        size_t* max_bytes_;
};

class file_request_handler : request_handler {
    public:
        file_request_handler(std::string file_path);
        std::string handle_request(std::string& return_msg);
    private:
        std::string file_path_;

        std::string get_content_type(std::string file_path);
        std::string file_extension_to_content_type(std::string file_extension);

        FRIEND_TEST(FileRequestTest, Extensions);
};
