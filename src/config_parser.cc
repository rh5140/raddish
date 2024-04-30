// An nginx config file parser.
//
// See:
//   http://wiki.nginx.org/Configuration
//   http://blog.martinfjordvald.com/2010/07/nginx-primer/
//
// How Nginx does it:
//   http://lxr.nginx.org/source/src/core/ngx_conf_file.c
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <stack>
#include <string>
#include <vector>
#include <map>
#include <boost/log/trivial.hpp>
#include "config_parser.h"

using namespace std; // NOT JUST FOR COUT 

std::string NginxConfig::ToString(int depth) {
  std::string serialized_config;
  for (const auto& statement : statements_) {
    serialized_config.append(statement->ToString(depth));
  }
  return serialized_config;
}
std::string NginxConfigStatement::ToString(int depth) {
  std::string serialized_statement;
  for (int i = 0; i < depth; ++i) {
    serialized_statement.append("  ");
  }
  for (unsigned int i = 0; i < tokens_.size(); ++i) {
    if (i != 0) {
      serialized_statement.append(" ");
    }
    serialized_statement.append(tokens_[i]);
  }
  if (child_block_.get() != nullptr) {
    serialized_statement.append(" {\n");
    serialized_statement.append(child_block_->ToString(depth + 1));
    for (int i = 0; i < depth; ++i) {
      serialized_statement.append("  ");
    }
    serialized_statement.append("}");
  } else {
    serialized_statement.append(";");
  }
  serialized_statement.append("\n");
  return serialized_statement;
}
const char* NginxConfigParser::TokenTypeAsString(TokenType type) {
  switch (type) {
    case TOKEN_TYPE_START:         return "TOKEN_TYPE_START";
    case TOKEN_TYPE_NORMAL:        return "TOKEN_TYPE_NORMAL";
    case TOKEN_TYPE_START_BLOCK:   return "TOKEN_TYPE_START_BLOCK";
    case TOKEN_TYPE_END_BLOCK:     return "TOKEN_TYPE_END_BLOCK";
    case TOKEN_TYPE_COMMENT:       return "TOKEN_TYPE_COMMENT";
    case TOKEN_TYPE_STATEMENT_END: return "TOKEN_TYPE_STATEMENT_END";
    case TOKEN_TYPE_EOF:           return "TOKEN_TYPE_EOF";
    default:                       return "TOKEN_TYPE_ERROR";
  }
}

//used for testing the above function publicly
const char* NginxConfigParser::NumberToTokenString(int n){
  switch(n){
    case 0:
      return TokenTypeAsString(TOKEN_TYPE_START);
    case 1:
      return TokenTypeAsString(TOKEN_TYPE_NORMAL);
    case 2:
      return TokenTypeAsString(TOKEN_TYPE_START_BLOCK);
    case 3:
      return TokenTypeAsString(TOKEN_TYPE_END_BLOCK);
    case 4:
      return TokenTypeAsString(TOKEN_TYPE_COMMENT);
    case 5:
      return TokenTypeAsString(TOKEN_TYPE_STATEMENT_END);
    case 6:
      return TokenTypeAsString(TOKEN_TYPE_EOF);
    default:
      return TokenTypeAsString(TOKEN_TYPE_ERROR);
  }
}


NginxConfigParser::TokenType NginxConfigParser::ParseToken(std::istream* input,
                                                           std::string* value) {
  TokenParserState state = TOKEN_STATE_INITIAL_WHITESPACE;
  while (input->good()) {
    const char c = input->get();
    if (!input->good()) {
      break;
    }
    switch (state) {
      case TOKEN_STATE_INITIAL_WHITESPACE:
        switch (c) {
          case '{':
            *value = c;
            return TOKEN_TYPE_START_BLOCK;
          case '}':
            *value = c;
            return TOKEN_TYPE_END_BLOCK;
          case '#':
            *value = c;
            state = TOKEN_STATE_TOKEN_TYPE_COMMENT;
            continue;
          case '"':
            *value = c;
            state = TOKEN_STATE_DOUBLE_QUOTE;
            continue;
          case '\'':
            *value = c;
            state = TOKEN_STATE_SINGLE_QUOTE;
            continue;
          case ';':
            *value = c;
            return TOKEN_TYPE_STATEMENT_END;
          case ' ':
          case '\t':
          case '\n':
          case '\r':
            continue;
          default:
            *value += c;
            state = TOKEN_STATE_TOKEN_TYPE_NORMAL;
            continue;
        }
      case TOKEN_STATE_SINGLE_QUOTE:
        *value += c;
        if (c == '\'') {
          return TOKEN_TYPE_NORMAL;
        }
        continue;
      case TOKEN_STATE_DOUBLE_QUOTE:
        *value += c;
        if (c == '"') {
          return TOKEN_TYPE_NORMAL;
        }
        continue;
      case TOKEN_STATE_TOKEN_TYPE_COMMENT:
        if (c == '\n' || c == '\r') {
          return TOKEN_TYPE_COMMENT;
        }
        *value += c;
        continue;
      case TOKEN_STATE_TOKEN_TYPE_NORMAL:
        if (c == ' ' || c == '\t' || c == '\n' || c == '\t' ||
            c == ';' || c == '{' || c == '}') {
          input->unget();
          return TOKEN_TYPE_NORMAL;
        }
        *value += c;
        continue;
    }
  }
  // If we get here, we reached the end of the file.
  if (state == TOKEN_STATE_SINGLE_QUOTE ||
      state == TOKEN_STATE_DOUBLE_QUOTE) {
    return TOKEN_TYPE_ERROR;
  }
  return TOKEN_TYPE_EOF;
}
bool NginxConfigParser::Parse(std::istream* config_file, NginxConfig* config) {
  std::stack<NginxConfig*> config_stack;
  config_stack.push(config);
  TokenType last_token_type = TOKEN_TYPE_START;
  TokenType token_type;
  int indent_count = 0;
  while (true) {
    std::string token;
    token_type = ParseToken(config_file, &token);
    //printf ("%s: %s\n", TokenTypeAsString(token_type), token.c_str());
    if (token_type == TOKEN_TYPE_ERROR) {
      break;
    }
    if (token_type == TOKEN_TYPE_COMMENT) {
      // Skip comments.
      continue;
    }
    if (token_type == TOKEN_TYPE_START) {
      // Error.
      break;
    } else if (token_type == TOKEN_TYPE_NORMAL) {
      if (last_token_type == TOKEN_TYPE_START ||
          last_token_type == TOKEN_TYPE_STATEMENT_END ||
          last_token_type == TOKEN_TYPE_START_BLOCK ||
          last_token_type == TOKEN_TYPE_END_BLOCK ||
          last_token_type == TOKEN_TYPE_NORMAL) {
        if (last_token_type != TOKEN_TYPE_NORMAL) {
          config_stack.top()->statements_.emplace_back(
              new NginxConfigStatement);
        }
        config_stack.top()->statements_.back().get()->tokens_.push_back(
            token);
      } else {
        // Error.
        break;
      }
    } else if (token_type == TOKEN_TYPE_STATEMENT_END) {
      if (last_token_type != TOKEN_TYPE_NORMAL) {
        // Error.
        break;
      }
    } else if (token_type == TOKEN_TYPE_START_BLOCK) {
      if (last_token_type != TOKEN_TYPE_NORMAL) {
        // Error.
        break;
      }
      indent_count++;
      NginxConfig* const new_config = new NginxConfig;
      config_stack.top()->statements_.back().get()->child_block_.reset(
          new_config);
      config_stack.push(new_config);
    } else if (token_type == TOKEN_TYPE_END_BLOCK) {
      if (last_token_type != TOKEN_TYPE_STATEMENT_END && 
      last_token_type != TOKEN_TYPE_START_BLOCK &&
      last_token_type != TOKEN_TYPE_END_BLOCK) {
        // Error.
        break;
      }
      indent_count--;
      config_stack.pop();
    } else if (token_type == TOKEN_TYPE_EOF) {
      if (last_token_type != TOKEN_TYPE_STATEMENT_END &&
          last_token_type != TOKEN_TYPE_END_BLOCK &&
          last_token_type != TOKEN_TYPE_START) {
        // Error.
        break;
      }
      //handle {} mismatch
      if(indent_count != 0){
        return false;
      }
      return true;
    } else {
      // Error. Unknown token.
      break;
    }
    last_token_type = token_type;
  }
  BOOST_LOG_TRIVIAL(warning) << "Bad transition from " << TokenTypeAsString(last_token_type) << " to " << TokenTypeAsString(token_type);
  return false;
}
bool NginxConfigParser::Parse(const char* file_name, NginxConfig* config) {
  std::ifstream config_file;
  config_file.open(file_name);
  if (!config_file.good()) {
    BOOST_LOG_TRIVIAL(error) << "Failed to open config file: " << file_name;
    return false;
  }
  const bool return_value =
      Parse(dynamic_cast<std::istream*>(&config_file), config);
  config_file.close();
  return return_value;
}

bool NginxConfigParser::GetServerSettings(NginxConfig* config){
    //config
    //behold my n^3 function and weep
    //I'm not sure if this should be made a real function - easier to test but this will eventually return a ton of things.
    //outermost config

    //init
    int port_num = -1;
    std::string root = "";
    // for temporarily storing locations without root overwriting 
    vector<std::string> default_root_locs;
    std::map<std::string, std::string> static_file_locations = std::map<std::string,std::string>();
    std::vector<std::string> echo_locations = std::vector<std::string>();



    for(int i = 0; i < (*config).statements_.size(); i++){ 
      //parse outermost config to find the server config
      for(int j = 0; j < (*(*config).statements_[i]).tokens_.size(); j++){
        //locate server config so we can find the server args
        if((*(*config).statements_[i]).tokens_[j] == "server"){
          BOOST_LOG_TRIVIAL(info) << "Server Config Found";
          NginxConfig server_config = (*(*(*config).statements_[i]).child_block_);
          for (int z = 0; z<server_config.statements_.size(); z++) {
          //iterate through server config to find each argument for server starting
            for(int k = 0; k < (*server_config.statements_[z]).tokens_.size(); k++){
              //handles port
              if((*server_config.statements_[z]).tokens_[k] == "listen" && k <= (*server_config.statements_[z]).tokens_.size()){
                (port_num) = stoi((*server_config.statements_[z]).tokens_[k + 1]);
                BOOST_LOG_TRIVIAL(info) << "Port Number found : " << (port_num);
              }

              // handles root
              if((*server_config.statements_[z]).tokens_[k] == "root" && k <= (*server_config.statements_[z]).tokens_.size()){
                root = (*server_config.statements_[z]).tokens_[k + 1];
                BOOST_LOG_TRIVIAL(info) << "Default root found: " << root;
              }

              // handles location
              if((*server_config.statements_[z]).tokens_[k] == "location" && k <= (*server_config.statements_[z]).tokens_.size()){
                std::string log_output = "";
                
                std::string key = (*server_config.statements_[z]).tokens_[k + 1];
                std::string type = (*server_config.statements_[z]).tokens_[k + 2];
                log_output = log_output + "Location: " + key + ", ";
                NginxConfig location_block = (*(*server_config.statements_[z]).child_block_);
                
                if (type == "static") {
                  for (int l = 0; l<location_block.statements_.size(); l++){
                    for (int m = 0; m<(*location_block.statements_[l]).tokens_.size(); m++){
                      if((*location_block.statements_[l]).tokens_[m] == "root" && m <= (*location_block.statements_[l]).tokens_.size()){
                        static_file_locations[key] = (*location_block.statements_[l]).tokens_[m + 1];
                        log_output = log_output + "root: " + static_file_locations[key] + " for serving static files";
                      }
                    }
                  }
                  if (static_file_locations.find(key) == static_file_locations.end()) {
                    log_output = log_output + "default root for serving static files";
                    default_root_locs.push_back(key);
                  }
                }
                else if (type == "echo") {
                  echo_locations.push_back(key);
                  log_output = log_output + "for echoing";
                }

                BOOST_LOG_TRIVIAL(info) << log_output;
              }
              //can add more args as needed here, following the pattern for ports
            }
          }
        }
      }
    }

    // add the locations without explicit root to the map
    for (const std::string& key : default_root_locs){
      static_file_locations[key] = root;
    }
    
    if((port_num) < 0 || (port_num) > 65353){ //65353 is the default max range for port
      return false;
    }

    // Add port number and locations to struct
    config_info.static_file_locations = static_file_locations;
    config_info.echo_locations = echo_locations;
    config_info.port_num = port_num;

    return true;
}

int NginxConfigParser::GetPortNum() {
  return config_info.port_num;
}

ConfigInfo NginxConfigParser::GetConfigInfo() {
  return config_info;
}