#include <boost/bind.hpp>
#include <boost/asio.hpp>


using boost::asio::ip::tcp;

class session {
public:
    session(boost::asio::io_service& io_service);
    tcp::socket& socket();
    void start();
    std::string parse_data(const char* data, size_t* max_bytes);
    std::string create_response();
private:
    tcp::socket socket_;
    enum { max_length = 1024 }; //for testing
    char data_[max_length];
    std::vector<char> buf;
    void handle_read(const boost::system::error_code& error, size_t bytes_transferred); 
    void handle_write(const boost::system::error_code& error);
};