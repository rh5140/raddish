#include "server.h"

server::server(boost::asio::io_service& io_service, ConfigInfo& config_info) : io_service_(io_service), acceptor_(io_service, tcp::endpoint(tcp::v4(), config_info.port_num)) {
    config_info_ = config_info;
    start_accept();
}

bool server::start_accept() {
    session* new_session = new session(io_service_, config_info_);
    acceptor_.async_accept(new_session->socket(),
        boost::bind(&server::handle_accept, 
            this, 
            new_session,
            boost::asio::placeholders::error));
    return true;
}

void server::handle_accept(session* new_session, const boost::system::error_code& error) {
    if (!error) {
        new_session->start();
    }
    else {
        delete new_session;
    }

    start_accept();
}


