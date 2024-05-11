#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <gtest/gtest_prod.h>
#include "info.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>

using boost::asio::ip::tcp;

struct Body
{
    // The type of message::body when used
    struct value_type;

    /// The algorithm used during parsing
    class reader;

    /// The algorithm used during serialization
    class writer;

    /// Returns the body's payload size
    static
    std::uint64_t
    size(value_type const& body);
};


class Session {
public:
    Session(boost::asio::io_service& io_service, ConfigInfo& config_info);
    tcp::socket& socket();
    void start();
    std::string create_response();
    void set_buf(std::string buf);
private:
    tcp::socket socket_;
    enum { max_length = 1024 }; //for testing
    char data_[max_length];
    std::vector<char> buf_;
    ConfigInfo config_info_;

    //new object stuff
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;

    void handle_read(const boost::system::error_code& error, size_t bytes_transferred); 
    void handle_write(const boost::system::error_code& error);

    // Friend tests: https://github.com/google/googletest/blob/main/docs/advanced.md#testing-private-code
    // There's a way to make the entire fixture a friend of the class, but I didn't find good documentation/examples yet, so I'm making individual tests friends for now
    FRIEND_TEST(SessionTest, SetData);
    FRIEND_TEST(SessionTest, HandleReadMaxLength);
    FRIEND_TEST(SessionTest, HandleReadPartialDataRead);
    FRIEND_TEST(SessionTest, HandleReadEOF);
    FRIEND_TEST(SessionTest, HandleReadConnectionReset);
    FRIEND_TEST(SessionTest, HandleReadOtherError);
    FRIEND_TEST(SessionTest, HandleRead);
    FRIEND_TEST(SessionTest, HandleWriteError);
    FRIEND_TEST(SessionTest, HandleWrite);
    FRIEND_TEST(SessionTest, CreateResponseStaticFile);
    FRIEND_TEST(SessionTest, CreateResponseEcho);
};