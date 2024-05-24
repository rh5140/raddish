#include <gtest/gtest.h>
#include "file_request_handler.h"
#include "request_handler_factory.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>

using CreateRequestHandler = RequestHandler*(*)(const RequestHandlerData&);

using namespace std;

class FileRequestTest : public testing::Test {
    protected:
        std::string file_path;
        FileRequestHandler* handler;
        RequestHandlerData request_handler_data;
        http::request<http::string_body> req;
        http::response<http::string_body> res;
        CreateRequestHandler handler_factory;

        void SetUp() override {
            std::string root = "";
            request_handler_data.root = root;
            AddrInfo addr_info;
            addr_info.host_addr =  "host:8080";
            addr_info.client_addr = "client:8080";
            request_handler_data.addr_info = addr_info;
            handler_factory = RequestHandlerFactory::get_factory("FileRequestHandler"); //get factory
        }
};


TEST_F(FileRequestTest, FileNotFound) {
    file_path = "/static_files/images/DOESNTEXIST.png";

    req.target(file_path);
    RequestHandler* handler = handler_factory(request_handler_data);
    res = handler->handle_request(req);
    EXPECT_EQ(res.result(), http::status::not_found);
    EXPECT_EQ(res.at(http::field::content_type), "text/plain");
    delete handler;
}

TEST_F(FileRequestTest, NotAFile) {
    std::string file = __FILE__;
    std::string file_name = "file_handler_test.cc";
    file_path = file.substr(0, file.length() - file_name.length()) + "/../static_files/images";

    req.target(file_path);
    RequestHandler* handler = handler_factory(request_handler_data);
    res = handler->handle_request(req);
    EXPECT_EQ(res.result(), http::status::not_found);
    EXPECT_EQ(res.at(http::field::content_type), "text/plain");
    delete handler;
}


TEST_F(FileRequestTest, LobsterTest) {
    std::string file = __FILE__;
    std::string file_name = "file_handler_test.cc";
    file_path = file.substr(0, file.length() - file_name.length()) + "/../static_files/text/lobster.txt";

    req.target(file_path);
    RequestHandler* handler = handler_factory(request_handler_data);
    res = handler->handle_request(req);
    EXPECT_EQ(res.result(), http::status::ok);
    EXPECT_EQ(res.at(http::field::content_type), "text/plain");
    EXPECT_EQ(res.body(), "lobster");
    delete handler;

}



TEST_F(FileRequestTest, Extensions) {
    RequestHandler* temp_handler = handler_factory(request_handler_data);

    FileRequestHandler* handler = static_cast<FileRequestHandler*>(temp_handler);

    EXPECT_EQ("text/html", handler->file_extension_to_content_type("html"));
    EXPECT_EQ("text/plain", handler->file_extension_to_content_type("txt"));
    EXPECT_EQ("image/jpeg", handler->file_extension_to_content_type("jpg"));
    EXPECT_EQ("image/png", handler->file_extension_to_content_type("png"));
    EXPECT_EQ("application/pdf", handler->file_extension_to_content_type("pdf"));
    EXPECT_EQ("application/zip", handler->file_extension_to_content_type("zip"));
    EXPECT_EQ("application/octet-stream", handler->file_extension_to_content_type("bin"));
}

