#include <cstdlib>
#include <iostream>
#include "session.h"


session::session(boost::asio::io_service& io_service) : socket_(io_service) {
}

tcp::socket& session::socket() {
    return socket_;
}

void session::start() {
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&session::handle_read, 
            this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}


void session::handle_read(const boost::system::error_code& error, size_t bytes_transferred) {
    std::cout << "Handle Read Data:" << std::endl;
    std::cout << data_ << std::endl;
    if (!error) {
        std::cout << "Sending Response..." << std::endl;
        std::string test_data = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\n";
        test_data = test_data + data_;
        boost::asio::async_write(socket_,
            boost::asio::buffer(test_data, test_data.size()), 
            boost::bind(&session::handle_write, 
                this,
                boost::asio::placeholders::error));
    }
    else {
        //std::cout << "Error in Handle Read" << std::endl;
        delete this;
    }
}


void session::handle_write(const boost::system::error_code& error) {
    if (!error) {
      socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&session::handle_read, 
            this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
    else {
        delete this;
    }
}