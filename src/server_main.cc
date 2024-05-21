//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <csignal>
#include <iostream>
#include "server.h"
#include "logger.h"
#include "config_parser.h"
#include "info.h"

using boost::asio::ip::tcp;

// Catching exception function 
void signalHandler(int signal) {
  BOOST_LOG_TRIVIAL(info) << "Raddish Down (Terminated)";
  std::exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
  try {
    if (argc != 2) {
      std::cerr << "Usage: async_tcp_echo_server <path/to/config>\n";
      return 1;
    }

    Logger logs;

    boost::asio::io_service io_service;

    using namespace std; // For atoi.

    NginxConfigParser parser;
    NginxConfig out_config;  

    signal(SIGINT, signalHandler); // CTRL + C handler

    // Loading in Configs
    BOOST_LOG_TRIVIAL(info) << "Getting the (con)figs - Start!";
    bool success = parser.parse(argv[1], &out_config);
    if(!success){
      BOOST_LOG_TRIVIAL(fatal) << "Oh no! Bad (con)figs were provided";
      return 1;
    }
    if(!parser.get_config_settings(&out_config)){
      BOOST_LOG_TRIVIAL(fatal) << "Failed to parse config file";
      return 1;
    }

    // Pass entire struct...
    ConfigInfo config_info = parser.get_config_info();
    
    Server s(io_service, config_info);
    BOOST_LOG_TRIVIAL(info) << "Raddish Online!";

    io_service.run();
  }
  catch (std::exception& e) {
    BOOST_LOG_TRIVIAL(fatal) << "Raddish Down (Exception) : " << e.what();
  }

  return 0;
}

