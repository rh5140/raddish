#include <string>
#include <gtest/gtest_prod.h>
#include "info.h"

class RequestHandler {
    public:
        virtual std::string handle_request(LogInfo log_info) = 0;
    protected:
        void log_request(LogInfo log_info);
};

class EchoRequestHandler : public RequestHandler {
    public:
        EchoRequestHandler(std::string request, size_t* max_bytes);
        std::string handle_request(LogInfo log_info);
    private:
        std::string request_;
        size_t* max_bytes_;
};

class FileRequestHandler : public RequestHandler {
    public:
        FileRequestHandler(std::string file_path);
        std::string handle_request(LogInfo log_info);
    private:
        std::string file_path_;

        std::string get_content_type(std::string file_path);
        std::string file_extension_to_content_type(std::string file_extension);

        FRIEND_TEST(FileRequestTest, Extensions);
};
