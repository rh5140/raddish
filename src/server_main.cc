//
// async_tcp_echo_server.cpp
// ~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <map>
#include <csignal>
#include "server.h"
#include "logger.h"

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

    boost::asio::io_service io_service;

    using namespace std; // For atoi.

    NginxConfigParser parser;
    NginxConfig out_config;  

    Logger logs;

    signal(SIGINT, signalHandler); // CNTRL + C handler

    bool success = parser.Parse(argv[1], &out_config);
    if(!success){
      return 1;
    }

    if(!parser.GetServerSettings(&out_config)){
      BOOST_LOG_TRIVIAL(error) << "Failed to parse config file";
      return 1;
    }

    // Pass entire struct...
    ConfigInfo config_info = parser.GetConfigInfo();
    
    server s(io_service, config_info);
    BOOST_LOG_TRIVIAL(info) << "Raddish Online!";

    io_service.run();
  }
  catch (std::exception& e) {
    BOOST_LOG_TRIVIAL(fatal) << "Raddish Down (Exception) : " << e.what();
  }

  return 0;
}

