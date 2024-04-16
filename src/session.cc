#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string>
#include "session.h"


using namespace std;

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

std::string session::parse_data(const char* data){
    //TODO: this currently assumes we recieved a valid line, which is fine for assignment 2
    //credit: https://stackoverflow.com/questions/13172158/c-split-string-by-line
    std::stringstream ss(data);
    std::string to;
    std::string ret = "";
    bool found_body = false; //looking for \n\n
    int content_length = -1; //just an init value
    string content_flag = "Content-Length:"; 
    //iterate through data to find content length, line-by-line
    while(std::getline(ss, to,'\n')){
        if(content_length == -1 && to.find(content_flag) != -1){ //if we find content length in the line
            //finds content-length by getting the substring from after the flag to the end
            //basically, Content-Length: 100 -> 100
            content_length = stoi(to.substr(content_flag.length()));
            string s(data);
        }
        //http can EITHER use "\n\n" or "\n\r\n\r" so we have to check for both
        else if(!found_body && to.length() == 0 || to.length() == 1 && to.find("\r") != -1){
            found_body = true;
        }
        else if(found_body){
            //check to see if buffer overflows content_length
            if(ret.length() + to.length() <= content_length){
                ret += to; //normal add
            }
            else{
                //if content-length = 10, ret.length = 8 and to.length = 3, then we get the substr of (0, (10-8)) or (0, 2)
                //i.e. start from 0 and get substr of length 2
                ret += to.substr(0, content_length - ret.length()); 
                return ret; //we've reached content_length so return
            }
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
        std::string response_body = session::parse_data(data_);
        content_length = content_length + std::to_string(response_body.size() + 1) + "\n\n"; //+1 is for the extra \n at the end
        http_response = http_response + content_length + response_body + '\n';
        std::cout << http_response << std::endl;
        //send response
        boost::asio::async_write(socket_,
            boost::asio::buffer(http_response, http_response.size()), 
            boost::bind(&session::handle_write, 
                this,
                boost::asio::placeholders::error));
        std::cout << "-------------" << std::endl;
    }
    else if ((boost::asio::error::eof == error) || (boost::asio::error::connection_reset == error)){ //disconnect
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