#include <boost/bind.hpp>
#include <boost/asio.hpp>


using boost::asio::ip::tcp;

class session {
public:
    session(boost::asio::io_service& io_service);
    tcp::socket& socket();
    void start();
    std::string parse_data(const char* data);
    std::string create_response(size_t bytes_transferred);
private:
    tcp::socket socket_;
    enum { max_length = 1024 };
    char data_[max_length];

    void handle_read(const boost::system::error_code& error, size_t bytes_transferred); 
    void handle_write(const boost::system::error_code& error);
};