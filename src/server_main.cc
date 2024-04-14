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


    //config
    //behold my n^3 function and weep
    //I'm not sure if this should be made a real function - easier to test but this will eventually return a ton of things.
    int port_num;
    //outermost config
    for(int i = 0; i < out_config.statements_.size(); i++){ 
      //parse outermost config to find the server config
      for(int j = 0; j < (*out_config.statements_[i]).tokens_.size(); j++){
        //locate server config so we can find the server args
        if((*out_config.statements_[i]).tokens_[j] == "server"){
          cout << "server config found" << endl;
          NginxConfig server_config = (*(*out_config.statements_[i]).child_block_);
          //iterate through server config to find each arguement for server starting
          for(int k = 0; k < (*server_config.statements_[0]).tokens_.size(); k++){
            //handles port
            if((*server_config.statements_[0]).tokens_[k] == "listen" && k <= (*server_config.statements_[0]).tokens_.size()){
              port_num = stoi((*server_config.statements_[0]).tokens_[k + 1]);
              cout << "port num found: " << port_num << endl;
            }
            //can add more args as needed here, following the pattern for ports
          }
        }
      }
    }

  

    server s(io_service, port_num);
    cout << "Server Running!" << endl;

    io_service.run();
  }
  catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
