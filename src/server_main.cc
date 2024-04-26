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
#include <map>
#include "server.h"
#include "config_parser.h"

using boost::asio::ip::tcp;

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

    bool success = parser.Parse(argv[1], &out_config);
    if(!success){
      return 1;
    }


    int port_num = -1;
    if(!parser.GetServerSettings(&out_config, &port_num)){
      std::cerr << "Failed to parse config" << std::endl;
      return 1;
    }
    


  
    // std::map<std::string, std::string> locations = parser.GetLocations();
    // TODO - send into server + session so that it can use it
    server s(io_service, port_num);
    cout << "Server Running!" << endl;

    io_service.run();
  }
  catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
