#include "logger.h"

#include <iostream>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

Logger::Logger() {
    this->is_test_ = false;
    init_logs();
}

// creates log sinks, sets rotation specs and format
void Logger::init_logs() {
    // lets timesstamp and threadID values exist
    logging::add_common_attributes();

    // names the file - logs/YYYY-MM-DD_hh-mm-ss-ffffff.log
    std::string file_name = "logs/%Y-%m-%d_%H-%M-%S-%f.log";
    if(this->is_test_) { 
        // Reason: to isolate unit test logs from actual so we dont accidentally mess actual up
        file_name = "t_logs/%Y-%m-%d_%H-%M-%S-%f.log";
    }

    // creates the file sink
    logging::add_file_log (
        keywords::target_file_name = file_name,  
        keywords::rotation_size = 10000000, // 10MB = 10 * 10^6B 
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
        keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%] %Message%",
        keywords::auto_flush = true
    );
    
    // creates the console sink
    logging::add_console_log(
        std::cout, 
        keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%] %Message%",
        keywords::auto_flush = true
    );
}

// remakes sinks to match right status
void Logger::set_test_status(bool is_test) {
    this->is_test_ = is_test;
    logging::core::get()->remove_all_sinks();
    this->init_logs();
}

// forces the logs to push out to sink 
void Logger::force_log_rotation() {
    logging::core::get()->flush();
    logging::core::get()->remove_all_sinks();
    this->init_logs();
}

/*
# How to use
- #include <boost/log/trivial.hpp>
- BOOST_LOG_TRIVIAL(severity_of_error) << error_msg;

# Format of general logs 
[time] [thread] [severity] msg

# Format of request logs (not enforced here)
[time] [thread] [severity] code - msg - client: ip  host : path  response: "_ _ _"

# Types
trace : for printing data (response, var values, etc)
debug : for knowing server's state/process is in when running
info : information thats we want to know (config, request, etc)
warning : something goes wrong but we dont need to worry / error that we expected to happen (bad requests, etc)
error : something goes wrong that we dont want to happen (something thats not supposed to happen)
fatal : error but it kills the server (exceptions, etc)
*/ 

