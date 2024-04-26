#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <gtest/gtest_prod.h>

using boost::asio::ip::tcp;

class session {
public:
    session(boost::asio::io_service& io_service);
    tcp::socket& socket();
    void start();
    std::string create_response();
    
    void set_buf(std::string buf);
private:
    tcp::socket socket_;
    enum { max_length = 1024 }; //for testing
    char data_[max_length];
    std::vector<char> buf_;
    void handle_read(const boost::system::error_code& error, size_t bytes_transferred); 
    void handle_write(const boost::system::error_code& error);

    // Friend tests: https://github.com/google/googletest/blob/main/docs/advanced.md#testing-private-code
    // There's a way to make the entire fixture a friend of the class, but I didn't find good documentation/examples yet, so I'm making individual tests friends for now
    FRIEND_TEST(SessionTest, SetData);
    FRIEND_TEST(SessionTest, HandleReadPartialDataRead);
    FRIEND_TEST(SessionTest, HandleReadEOF);
    FRIEND_TEST(SessionTest, HandleReadConnectionReset);
    FRIEND_TEST(SessionTest, HandleReadOtherError);
    FRIEND_TEST(SessionTest, HandleRead);
    FRIEND_TEST(SessionTest, HandleWriteError);
    FRIEND_TEST(SessionTest, HandleWrite);
};