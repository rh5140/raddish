#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string>
#include <boost/log/trivial.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include "session.h"
#include "request_dispatcher.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>


using namespace std;



Session::Session(boost::asio::io_service& io_service, ConfigInfo& config_info) : socket_(io_service) {
    config_info_ = config_info;
}

tcp::socket& Session::socket() {
    return socket_;
}

void Session::start() {
    http::async_read(
    socket_,
    buffer_,
    req_,
    boost::bind(&Session::handle_read, 
        this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
    /*
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&Session::handle_read, 
            this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
            */
}

// Setter for buf_ (made for testing)
void Session::set_buf(std::string buf) {
    for (char c : buf) {
        buf_.push_back(c);
    }
}

//public




http::response<http::string_body> Session::create_response(){
    BOOST_LOG_TRIVIAL(debug) << "http req object:\n" << req_;
    BOOST_LOG_TRIVIAL(debug) << "http req object method:\n" << req_.method_string();

    /*
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
    */
    std::string client_addr, host_addr;
    try {
        tcp::endpoint client = socket().remote_endpoint();
        tcp::endpoint host = socket().local_endpoint();
        client_addr = client.address().to_string() + ":" + to_string(client.port());
        host_addr = host.address().to_string() + ":" + to_string(host.port());
    } 
    catch (const boost::system::system_error& e) {
        BOOST_LOG_TRIVIAL(error) << "Sockets do not exist";
    }
    RequestDispatcher* dispatcher = new RequestDispatcher();
    return dispatcher->dispatch_request(req_, config_info_, client_addr, host_addr);
}

//private
void Session::handle_read(const boost::system::error_code& error, size_t bytes_transferred) {
    if (!error) {
        BOOST_LOG_TRIVIAL(debug) << "Reading in data";

        //note - new async_read always reads in the entire message, so we don't need recursive calls anymore.

        BOOST_LOG_TRIVIAL(debug) << "Creating Response";
        //create response
        res_ = create_response();

        //clear buffer for next
        buf_.clear();

        //TODO: make this in create response

        res_.prepare_payload();

        http::async_write(
            socket_,
            std::move(res_),
            boost::bind(&Session::handle_write, 
            this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
    else if((boost::beast::http::error::partial_message == error)){
        BOOST_LOG_TRIVIAL(debug) << "Partial Read";
    }
    else if ((boost::beast::http::error::end_of_stream == error) || (boost::asio::error::eof == error)){ //disconnect
        BOOST_LOG_TRIVIAL(debug) << "Disconnect due to end of file";
        delete this;
    }
    else if (boost::asio::error::connection_reset == error){ //disconnect
        BOOST_LOG_TRIVIAL(debug) << "Disconnect due to connection reset";
        delete this;
    }
    else {
        BOOST_LOG_TRIVIAL(trace) << data_;
        BOOST_LOG_TRIVIAL(trace) << "Error: " << error.message(); 
        BOOST_LOG_TRIVIAL(error) << "Error in handle read";
        delete this;
    }
}

//private
void Session::handle_write(const boost::system::error_code& error, size_t bytes_transferred) {
    if (!error) {
        http::async_read(
            socket_,
            buffer_,
            req_,
            boost::bind(&Session::handle_read, 
                this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }
    else {
        BOOST_LOG_TRIVIAL(trace) << error.message();
        BOOST_LOG_TRIVIAL(error) << "Error in handle write";
        delete this;
    }
}
