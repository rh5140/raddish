#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <cstddef>
#include <iostream>
#include "logger.h"

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

logger::logger(bool isTest) {
    this->isTest = isTest;
    init_logs();
}

void logger::init_logs() {
    // lets timesstamp and threadID values 
    logging::add_common_attributes();

    // names the file - logs/2024-04-27_#####.log
      std::string file_name = "logs/%Y-%m-%d_%H-%M-%S-%f.log"; // uses time to make sure in order
    if(this->isTest) { // so when remove from tests it does not accidentally rewrite actual logs
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

time_t logger::force_log_rotation() {
    logging::core::get()->flush();
    logging::core::get()->remove_all_sinks();
    time_t log_time = std::time(nullptr);
    this->init_logs();
    return log_time;
}

/*
# How to use
- #include <boost/log/trivial.hpp>
- BOOST_LOG_TRIVIAL(severity_of_error) << error_msg;

# Format of the logs 
[time] [thread] [severity] msg


# Types
BOOST_LOG_TRIVIAL(trace) << "A trace severity message";
BOOST_LOG_TRIVIAL(debug) << "A debug severity message";
BOOST_LOG_TRIVIAL(info) << "An informational severity message";
BOOST_LOG_TRIVIAL(warning) << "A warning severity message";
BOOST_LOG_TRIVIAL(error) << "An error severity message";
BOOST_LOG_TRIVIAL(fatal) << "A fatal severity message";
*/ 


/* Code that I got from ChatGPT for this function bc the actual page no longer exists -_-
logging::formatter console_formatter = logging:expressions::format("[%TimeStamp%] [%ThreadID%] [%Severity%] %Message%");
logging::formatter test_formatter = logging:expressions::format("[%Severity%] %Message%");

logging::register_simple_formatter_factory<logging::trivial::severity_level, char>(
    "ConsoleFormatter", 
    keywords::format = console_formatter);
*/
