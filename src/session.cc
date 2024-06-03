#include "session.h"
#include "request_dispatcher.h"

#include <string>
#include <boost/bind.hpp>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>

using namespace std;

Session::Session(boost::asio::io_service& io_service, ConfigInfo& config_info) : socket_(io_service), process_("Unknown - Session"), severity_(sev_lvl::info) {
    config_info_ = config_info;
    lg_.add_attribute("Process", process_);
    lg_.add_attribute("Severity", severity_);
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
        tcp::endpoint client = socket().local_endpoint();
        tcp::endpoint host = socket().remote_endpoint();
        client_addr = client.address().to_string() + ":" + to_string(client.port());
        host_addr = host.address().to_string() + ":" + to_string(host.port());
    } 
    catch (const boost::system::system_error& e) { //if this fails it just means our logs are not going to have client info, so we can keep going.
        severity_.set(sev_lvl::error);
        BOOST_LOG(lg_) << "Sockets do not exist";
    }

    RequestDispatcher* dispatcher = new RequestDispatcher();
    http::response<http::string_body> res =  dispatcher->dispatch_request(req_, config_info_, client_addr, host_addr);
    req_.body() = "";
    delete dispatcher;
    return res;
}

//private
void Session::handle_read(const boost::system::error_code& error, size_t bytes_transferred) {
    if (!error) {
        process_.set(string(req_.method_string()) + " " + string(req_.target()) + " - Session");
        severity_.set(sev_lvl::debug);
        BOOST_LOG(lg_) << "Session open"; 

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
        severity_.set(sev_lvl::debug);
        BOOST_LOG(lg_) << "Partial Read"; 
    }
    else if ((boost::beast::http::error::end_of_stream == error) || (boost::asio::error::eof == error)){ //disconnect
        severity_.set(sev_lvl::debug);
        BOOST_LOG(lg_) << "Disconnect due to end of file";
        delete this;
    }
    else if (boost::asio::error::connection_reset == error){ //disconnect
        severity_.set(sev_lvl::debug);
        BOOST_LOG(lg_) << "Disconnect due to connection reset";
        delete this;
    }
    else {
        severity_.set(sev_lvl::error);
        BOOST_LOG(lg_) << "Error in handle read : " << error.message();
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
        severity_.set(sev_lvl::error);
        BOOST_LOG(lg_) << "Error in handle write : " << error.message();
        delete this;
    }
}
