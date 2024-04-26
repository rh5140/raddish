#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include "session.h"
#include <gtest/gtest_prod.h>

using boost::asio::ip::tcp;

class server {
public:
    server(boost::asio::io_service& io_service, ConfigInfo& config_info);

private:
    boost::asio::io_service& io_service_;
    tcp::acceptor acceptor_;
    ConfigInfo config_info_;

    bool start_accept();
    void handle_accept(session* new_session, const boost::system::error_code& error);

    FRIEND_TEST(ServerTest, StartAccept);
    FRIEND_TEST(ServerTest, HandleAccept);
    FRIEND_TEST(ServerTest, HandleAcceptError);
};