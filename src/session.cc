#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <string>
#include <regex>
#include <boost/log/trivial.hpp>
#include "session.h"
#include "request_handler.h"

using namespace std;

session::session(boost::asio::io_service& io_service, ConfigInfo& config_info) : socket_(io_service) {
    config_info_ = config_info;
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

// Setter for buf_ (made for testing)
void session::set_buf(std::string buf) {
    for (char c : buf) {
        buf_.push_back(c);
    }
}

//public
std::string session::create_response(){
    std::string response = "";
    
    std::stringstream ss(buf_.data());
    std::string first_line;
    std::getline(ss, first_line,'\n'); // get the first line

    bool foundSpace = false;
    std::string file_path = "";

    // extract path
    for (int i = 0; i<first_line.length(); i++) {
        if (first_line[i] == ' ') 
            if (!foundSpace)
            {
                foundSpace = true;
                i++;
            }
            else 
                break;
        if (foundSpace) {
            file_path += first_line[i];
        }
    }
    // extract first element of path
    int idx_end;
    if (file_path[0] == '/') {
        for (int i = 1; i < file_path.length(); i++) {
            if (file_path[i] == '/') {
                idx_end = i;
                break;
            }
        }
    }
    std::string file_path_start = file_path.substr(0, idx_end + 1);
    // compare with list of locations parsed from config
    bool isStaticFilePath = false;
    bool isEchoPath = false;
    std::string root = "";
    for (auto const& x : config_info_.static_file_locations){
        if (x.first == file_path_start) {
            root = config_info_.static_file_locations[x.first];
            isStaticFilePath = true;
            break;
        }
    }
    for (auto const& x : config_info_.echo_locations){
        if (x == file_path) {
            isEchoPath = true;
            break;
        }
    }
    size_t total_data = buf_.size();

    if (isStaticFilePath) {
        file_request_handler* handler = new file_request_handler(root + file_path);
        response = handler->handle_request(buf_.data(), &total_data);
    }
    // TODO - will need to change if only specified paths will echo
    else { // assuming even non-echo paths will echo
        echo_request_handler* handler = new echo_request_handler();
        response = handler->handle_request(buf_.data(), &total_data);
    }

    return response;
}

//private
void session::handle_read(const boost::system::error_code& error, size_t bytes_transferred) {
    if (!error) {
        buf_.insert(buf_.end(), data_, data_ + bytes_transferred);
        if(bytes_transferred >= max_length){ //should never be greater but just in case...
            BOOST_LOG_TRIVIAL(error) << "Bytes transferred greater than max length";
            session::handle_write(error);
        }
        else{
            //create response
            std::string http_response = create_response();
            //clear buffer for next
            buf_.clear();
            //send response
            boost::asio::async_write(socket_,
                boost::asio::buffer(http_response, http_response.size()), 
                boost::bind(&session::handle_write, 
                    this,
                    boost::asio::placeholders::error));
        }
    }
    else if ((boost::asio::error::eof == error) || (boost::asio::error::connection_reset == error)){ //disconnect
        BOOST_LOG_TRIVIAL(error) << "Disconnect due to end of file reached or connection reset";
        delete this;
    }
    else {
        BOOST_LOG_TRIVIAL(trace) << data_;
        BOOST_LOG_TRIVIAL(error) << "Error in handle read";
        delete this;
    }
}

//private
void session::handle_write(const boost::system::error_code& error) {
    if (!error) {
      socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&session::handle_read, 
            this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
    else {
        BOOST_LOG_TRIVIAL(error) << "Error in handle write";
        delete this;
    }
}








    /*
    //I'm keeping this around for now in case we need to parse the body in later iterations of the webserver
    //iterate through data to find content length, line-by-line
    std::string headerRet = "";
    int content_length = -1; //just an init value       
    string content_flag = "Content-Length:"; 
    bool found_body = false; //looking for \n\n
    while(std::getline(ss, to,'\n')){
        if(!found_body){
            headerRet += to + "\n";
        }
        //bytes_read += to.length();
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
        else if(found_body && content_length == -1){ //no body
            return headerRet;
        }
        else if(found_body){
            //check to see if buffer overflows content_length
            if(ret.length() + to.length() + 1 <= content_length){
                ret += to + "\n"; //normal add
            }
            else{
                //if content-length = 10, ret.length = 8 and to.length = 3, then we get the substr of (0, (10-8)) or (0, 2)
                //i.e. start from 0 and get substr of length 2
                ret += to.substr(0, content_length - ret.length()); 
                return headerRet + ret; //we've reached content_length so return
            }
        }
    }
    //return headerRet + ret;
    */