#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string>
#include <boost/log/trivial.hpp>
#include "session.h"
#include "request_dispatcher.h"

using namespace std;

Session::Session(boost::asio::io_service& io_service, ConfigInfo& config_info) : socket_(io_service) {
    config_info_ = config_info;
}

tcp::socket& Session::socket() {
    return socket_;
}

void Session::start() {
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&Session::handle_read, 
            this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
}

// Setter for buf_ (made for testing)
void Session::set_buf(std::string buf) {
    for (char c : buf) {
        buf_.push_back(c);
    }
}

//public
std::string Session::create_response(){
    RequestDispatcherInfo req_dis_info;
    try {
        tcp::endpoint client = socket().remote_endpoint();
        tcp::endpoint host = socket().local_endpoint();
        req_dis_info.addr_info.client_addr = client.address().to_string() + ":" + to_string(client.port());
        req_dis_info.addr_info.host_addr = host.address().to_string() + ":" + to_string(host.port());
    } 
    catch (const boost::system::system_error& e) {
        BOOST_LOG_TRIVIAL(error) << "Sockets do not exist";
    }
    
    req_dis_info.request = buf_.data();
    req_dis_info.request_size = buf_.size();
    req_dis_info.config_info = config_info_;

    RequestDispatcher* dispatcher = new RequestDispatcher();
    return dispatcher->dispatch_request(req_dis_info);
}

//private
void Session::handle_read(const boost::system::error_code& error, size_t bytes_transferred) {
    if (!error) {
        buf_.insert(buf_.end(), data_, data_ + bytes_transferred);
        if(bytes_transferred >= max_length){ //should never be greater but just in case...
            BOOST_LOG_TRIVIAL(debug) << "Bytes transferred greater than max length";
            Session::handle_write(error);
        }
        else{
            //create response
            std::string http_response = create_response();

            //clear buffer for next
            buf_.clear();

            //send response
            boost::asio::async_write(socket_,
                boost::asio::buffer(http_response, http_response.size()), 
                boost::bind(&Session::handle_write, 
                    this,
                    boost::asio::placeholders::error));
        }
    }
    else if ((boost::asio::error::eof == error) || (boost::asio::error::connection_reset == error)){ //disconnect
        BOOST_LOG_TRIVIAL(debug) << "Disconnect due to end of file reached or connection reset";
        delete this;
    }
    else {
        BOOST_LOG_TRIVIAL(trace) << data_;
        BOOST_LOG_TRIVIAL(error) << "Error in handle read";
        delete this;
    }
}

//private
void Session::handle_write(const boost::system::error_code& error) {
    if (!error) {
      socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&Session::handle_read, 
            this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
    else {
        BOOST_LOG_TRIVIAL(error) << "Error in handle write";
        delete this;
    }
}
