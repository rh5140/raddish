//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "server.h"
#include "config_parser.h"
#include "logger.h"
#include "info.h"

#include <csignal>
#include <iostream>
#include <thread>
#include <vector>

#include <boost/log/attributes.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/logger.hpp>

using boost::asio::ip::tcp;
namespace logging = boost::log;
namespace attrs = boost::log::attributes;
namespace sev_lvl = boost::log::trivial;

// Catching exception function 
void signalHandler(int signal) {
    logging::sources::logger lg;
    lg.add_attribute("Process", attrs::mutable_constant<std::string>("Server"));
    lg.add_attribute("Severity", attrs::mutable_constant<sev_lvl::severity_level>(sev_lvl::info));
    BOOST_LOG(lg) << "Raddish Down (Terminated)";
    std::exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
    Logger logs;
    logging::sources::logger lg;
    auto process = attrs::mutable_constant<std::string>("Server Startup");
    auto severity = attrs::mutable_constant<sev_lvl::severity_level>(sev_lvl::info);
    lg.add_attribute("Process", process);
    lg.add_attribute("Severity", severity);

    try {
        if (argc != 2) {
            std::cerr << "Usage: async_tcp_echo_server <path/to/config>\n";
            return 1;
        }

        boost::asio::io_service io_service;

        using namespace std; // For atoi.

        NginxConfigParser parser;
        NginxConfig out_config;  

        signal(SIGINT, signalHandler); // CTRL + C handler

        // Loading in Configs
        severity.set(sev_lvl::info);
        BOOST_LOG(lg) << "Getting the (con)figs - Start!";
        bool success = parser.parse(argv[1], &out_config);
        if(!success){
            severity.set(sev_lvl::fatal);
            BOOST_LOG(lg) << "Oh no! Bad (con)figs were provided";
            return 1;
        }
        if(!parser.get_config_settings(&out_config)){
            severity.set(sev_lvl::fatal);
            BOOST_LOG(lg) << "Failed to parse config file";
            return 1;
        }

        // Pass entire struct...
        ConfigInfo config_info = parser.get_config_info();
        
        Server s(io_service, config_info);
        process.set("Server");
        severity.set(sev_lvl::info);
        BOOST_LOG(lg) << "Raddish Online!";

        int threads = 2; // arbitrary number - could probably be specified in config

        //code below from https://www.boost.org/doc/libs/1_79_0/libs/beast/example/advanced/server/advanced_server.cpp
        std::vector<std::thread> v;
        v.reserve(threads - 1);
        for(auto i = threads - 1; i > 0; --i)
            v.emplace_back([&io_service] {
                io_service.run();
            });

        io_service.run();
    }
    catch (std::exception& e) {
        severity.set(sev_lvl::fatal);
        BOOST_LOG(lg) << "Raddish Down (Exception) : " << e.what();
    }

    return 0;
}

