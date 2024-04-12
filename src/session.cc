#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string>
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

std::string parse_data(char* data){
    //TODO: this currently assumes we recieved a valid line
    //TODO: will implement in next commit
    //std::stringstream ss(data);
}

void session::handle_read(const boost::system::error_code& error, size_t bytes_transferred) {
    std::cout << "Handle Read Data:" << std::endl;
    std::cout << data_ << std::endl;
    if (!error) {
        std::cout << "Sending Response..." << std::endl;
        std::string http_response = "HTTP/1.1 200 OK\nContent-Type: text/plain\n";
        std::string content_length = "Content-Length: ";
        std::string response_body = data_;
        content_length = content_length + std::to_string(response_body.size()) + "\n\n";
        http_response = http_response + content_length + response_body;
        boost::asio::async_write(socket_,
            boost::asio::buffer(http_response, http_response.size()), 
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