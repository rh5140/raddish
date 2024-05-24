#include "server.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>

using boost::asio::ip::tcp;

Server::Server(boost::asio::io_service& io_service, ConfigInfo& config_info) : io_service_(io_service), acceptor_(io_service, tcp::endpoint(tcp::v4(), config_info.port_num)) {
    config_info_ = config_info;
    start_accept();
}

bool Server::start_accept() {
    Session* new_session = new Session(io_service_, config_info_);
    acceptor_.async_accept(new_session->socket(),
        boost::bind(&Server::handle_accept, 
            this, 
            new_session,
            boost::asio::placeholders::error));
    return true;
}

void Server::handle_accept(Session* new_session, const boost::system::error_code& error) {
    if (!error) {
        new_session->start();
    }
    else {
        delete new_session;
    }

    start_accept();
}


