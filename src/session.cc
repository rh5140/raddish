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
}

// Setter for req, used for testing to simulate incoming requests.
// ONLY CALL THIS IN TESTING - OTHERWISE SHOULD ALWAYS BE SET BY HANDLE READ
void Session::set_req(http::request<http::string_body> req) {
    req_ = req;
}

//public

http::response<http::string_body> Session::create_response(){
    std::string client_addr, host_addr;
    try {
        tcp::endpoint client = socket().remote_endpoint();
        tcp::endpoint host = socket().local_endpoint();
        client_addr = client.address().to_string() + ":" + to_string(client.port());
        host_addr = host.address().to_string() + ":" + to_string(host.port());
    } 
    catch (const boost::system::system_error& e) { //if this fails it just means our logs are not going to have client info, so we can keep going.
        BOOST_LOG_TRIVIAL(error) << "Sockets do not exist";
    }

    RequestDispatcher* dispatcher = new RequestDispatcher();
    http::response<http::string_body> res =  dispatcher->dispatch_request(req_, config_info_, client_addr, host_addr);
    delete dispatcher;
    return res;
}

//privateSS
void Session::handle_read(const boost::system::error_code& error, size_t bytes_transferred) {
    if (!error) {
        //create response
        res_ = create_response();
        res_.prepare_payload();
        
        //write response to client
        http::async_write(
            socket_,
            std::move(res_),
            boost::bind(&Session::handle_write, 
            this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
    else if((boost::beast::http::error::partial_message == error)){ //should never happen, but good to have just in case.
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
        BOOST_LOG_TRIVIAL(error) << "Error in handle read : " << error.message();
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
