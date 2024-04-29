#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/core.hpp>
#include <iostream>
#include "logger.h"

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

logger::logger(bool isTest) {
    // lets timesstamp and threadID values 
    logging::add_common_attributes();

    // names the file - logs/2024-04-27_#####.log
    std::string file_name = "logs/%Y-%m-%d_%S.log"; // uses seconds bc larger number is harder to clash and thus rewrite
    if(isTest) { // so when remove from tests it does not accidentally rewrite actual logs
        file_name = "t_logs/%Y-%m-%d_%3N.log";
    }

    // creates the file sink
    logging::add_file_log (
        keywords::target_file_name = file_name, 
        keywords::rotation_size = 10000000, // 10MB = 10 * 10^6B 
        keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
        keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%] %Message%"
    );
    
    // creates the console sink
    logging::add_console_log(
        std::cout, 
        keywords::format = "[%TimeStamp%] [%ThreadID%] [%Severity%] %Message%"
    );

    // filters out both trace/debug messages (if we do use), so less clutter on log
    logging::core::get()->set_filter(
        logging::trivial::severity >= logging::trivial::info
    );
}

/*
# How to use
- #include <boost/log/trivial.hpp>
- BOOST_LOG_TRIVIAL(severity_of_error) << error_msg;

# Format of the logs 
[time] [thread] [severity] msg
*/ 


/* Code that I got from ChatGPT for this function bc the actual page no longer exists -_-
logging::formatter console_formatter = logging:expressions::format("[%TimeStamp%] [%ThreadID%] [%Severity%] %Message%");
logging::formatter test_formatter = logging:expressions::format("[%Severity%] %Message%");

logging::register_simple_formatter_factory<logging::trivial::severity_level, char>(
    "ConsoleFormatter", 
    keywords::format = console_formatter);
*/
