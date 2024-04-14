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

    //credit: https://stackoverflow.com/questions/13172158/c-split-string-by-line
    std::stringstream ss(data);
    std::string to;
    std::string ret = "";
    bool foundBody = false;
    while(std::getline(ss, to,'\n')){
        if(foundBody){
            ret = ret + to + "\n";
        }
        if(!foundBody && to.size() == 1){ //not sure how to find body exactly
            foundBody = true;
        }
    }
    return ret;
}

void session::handle_read(const boost::system::error_code& error, size_t bytes_transferred) {
    if (!error) {
        //log
        std::cout << "-------------" << std::endl;
        std::cout << "Handle Read Data:" << std::endl;
        std::cout << data_ << std::endl;
        std::cout << "-------------" << std::endl;
        std::cout << "Sending Response..." << std::endl;
        //generate response
        std::string http_response = "HTTP/1.1 200 OK\nContent-Type: text/plain\n";
        std::string content_length = "Content-Length: ";
        std::string response_body = parse_data(data_);
        content_length = content_length + std::to_string(response_body.size()) + "\n\n";
        http_response = http_response + content_length + response_body;
        std::cout << http_response << std::endl;
        //send response
        boost::asio::async_write(socket_,
            boost::asio::buffer(http_response, http_response.size()), 
            boost::bind(&session::handle_write, 
                this,
                boost::asio::placeholders::error));
        std::cout << "-------------" << std::endl;
    }
    else if ((boost::asio::error::eof == error) || (boost::asio::error::connection_reset == error)){ //discocnnect
        delete this;
    }
    else {
        //log
        std::cout << "-------------" << std::endl;
        std::cout << "Handle Read Data:" << std::endl;
        std::cout << data_ << std::endl;
        std::cout << "Error in Handle Read" << std::endl;
        std::cout << "-------------" << std::endl;
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