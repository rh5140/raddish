#ifndef SESSION
#define SESSION

#include "info.h"

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/logger.hpp>
#include <gtest/gtest_prod.h>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace logging = boost::log;
namespace attrs = boost::log::attributes;
namespace sev_lvl = boost::log::trivial;

using boost::asio::ip::tcp;

struct Body {
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
    http::response<http::string_body> create_response();
    void set_req(http::request<http::string_body> req);
private:
    tcp::socket socket_;
    enum { max_length = 1024 }; //for testing
    ConfigInfo config_info_;

    // logger 
    logging::sources::logger lg_;
    attrs::mutable_constant<std::string> process_;
    attrs::mutable_constant<sev_lvl::severity_level> severity_;
    
    //new object stuff
    beast::flat_buffer buffer_;
    http::request<http::string_body> req_;
    http::response<http::string_body> res_;

    void handle_read(const boost::system::error_code& error, size_t bytes_transferred); 
    void handle_write(const boost::system::error_code& error, size_t bytes_transferred);

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
    FRIEND_TEST(SessionTest, CreateResponseEcho);
};

#endif