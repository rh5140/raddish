#include "logger.h"

#include <iostream>
#include <string>
#include <boost/log/attributes.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;

// Makes attr so we can use in format + logs
BOOST_LOG_ATTRIBUTE_KEYWORD(process, "Process", std::string)

Logger::Logger() {
    this->is_test_ = false;
    init_logs();
}

// creates log sinks, sets rotation specs and format
void Logger::init_logs() {
    // lets timesstamp and threadID values exist
    logging::add_common_attributes();
    logging::core::get()->add_global_attribute("Process", attrs::constant<std::string>("Unknown"));

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
        keywords::format = "[%TimeStamp%] [%ThreadID%] [%Process%] [%Severity%] %Message%",
        keywords::auto_flush = true
    );

    // creates the console sink
    logging::add_console_log(
        std::cout, 
        keywords::format = "[%TimeStamp%] [%ThreadID%] [%Process%] [%Severity%] %Message%",
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
## Libraries + namespace
#include <boost/log/attributes.hpp> // for attributes
#include <boost/log/trivial.hpp> // for severity lvls + log func
#include <boost/log/sources/logger.hpp> // for logger object

namespace logging = boost::log;
namespace attrs = boost::log::attributes;
namespace sev_lvl = boost::log::trivial;

## Declare the attributes
logging::sources::logger lg;
auto process = attrs::mutable_constant<std::string>(NAME);
auto severity = attrs::mutable_constant<sev_lvl::severity_level>(sev_lvl::LVL)
lg.add_attribute("Process", process);
lg_.add_attribute("Severity", severity);
-----------------
logging::sources::logger lg_;
attrs::mutable_constant<std::string> process_;
attrs::mutable_constant<sev_lvl::severity_level> severity_;

class_constructor() : process_(NAME), severity_(sev_lvl::info) {
    lg_.add_attribute("Process", process_);
    lg_.add_attribute("Severity", severity_);
}

## Changing attributes
ATTR.set(NEW_VAL);

# Make a log (RMB TO DECLARE ATTR FIRST)
BOOST_LOG(lg) << ERROR_MSG;

# Format of general logs 
[%time] [%thread] [%process] [%severity] %msg
- time : time when log was created
- thread : thread id 
- process : class that ran the log command
    - non-request logs: just class name
    - request logs: req_method req_path - class name (for knowing what req was ran)
- severity : type of log (trace, debug, info, warning, error, fatal)
- msg : message that we want to log

# Format of request logs
[%time] [%thread] [%process] [%severity] %res_code %msg - client: %client_ip, host: %host_ip, request: "%req_line"
- res_code : response code for request
- client_ip : ip of client that made request
- host_ip : ip of where request was made to
- req_line : first line of request holding method, path, and version

# Severity levels 
sev_lvl::trace : for printing data (response, var values, etc)
sev_lvl::debug : for knowing server's state/process is in when running
sev_lvl::info : information thats we want to know (config, request, etc) 
sev_lvl::warning : something goes wrong but we dont need to worry / error that we expected to happen (bad requests, etc)
sev_lvl::error : something goes wrong that we dont want to happen (file cant be used, etc)
sev_lvl::fatal : error but it kills the server (exceptions, etc)
*/ 

