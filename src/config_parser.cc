// An nginx config file parser.
//
// See:
//   http://wiki.nginx.org/Configuration
//   http://blog.martinfjordvald.com/2010/07/nginx-primer/
//
// How Nginx does it:
//   http://lxr.nginx.org/source/src/core/ngx_conf_file.c
#include <cstddef>
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
#include "info.h"

using namespace std; // NOT JUST FOR COUT 


//Note: this is for the NginxConfig class, it was given combined so I left it in the same .cc
std::string NginxConfig::ToString(int depth) {
  std::string serialized_config;
  for (const auto& statement : statements_) {
    serialized_config.append(statement->ToString(depth));
  }
  return serialized_config;
}

//and this is for the statement class
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

//TODO: once changes are merged I'm going to rename these functions
bool NginxConfigParser::GetServerSettingsInner(){

    NginxConfig* server_config = &internal_config_; //set to internal config

    //init
    int port_num = -1;
    std::string root = "";
    // for temporarily storing locations without root overwriting 
    vector<std::string> default_root_locs;
    std::map<std::string, std::string> static_file_locations = std::map<std::string,std::string>();
    std::vector<std::string> echo_locations = std::vector<std::string>();
    std::string arg_token = "";

    //iterate through the config to extract data
    for (int i = 0; i < server_config->statements_.size(); i++) {
      std::vector<std::string> tokens = (*server_config->statements_[i]).tokens_;
      //iterate through the config lines to find each argument
      for(int j = 0; j < tokens.size(); j++){
        arg_token = tokens[j]; //since we check this a lot, just store it.
        //need to check this so j+1 doesn't segfault, not doing this in main loop in case someone needs to check the last thing.
        if(j < tokens.size() - 1){
          //handles port
          if(arg_token == "listen"){
            port_num = stoi(tokens[j + 1]);
            BOOST_LOG_TRIVIAL(info) << "Port Number found : " << (port_num);
          }
          // handles root
          else if(arg_token == "root"){
            root = tokens[j + 1];
            BOOST_LOG_TRIVIAL(info) << "Default root found: " << root;
          }
          //can add more args as needed here, following the pattern for ports

          //keep this one last as it's more complex
          // handles location
          else if(arg_token == "location"){
            std::string log_output = "";
            std::string key = tokens[j + 1];
            std::string type = tokens[j + 2];
            log_output = log_output + "Location: " + key + ", ";
            NginxConfig location_block = (*(*server_config->statements_[i]).child_block_);
            
            if (type == "static") {
              for (int l = 0; l < location_block.statements_.size(); l++){
                for (int m = 0; m < (*location_block.statements_[l]).tokens_.size(); m++){
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
        }
      }
    }

    //correctness checking
    if((port_num) < 0 || (port_num) > 65353){ //65353 is the default max range for port
      return false;
    }
    //add more checks here as needed

    // add the locations without explicit root to the map
    for (const std::string& key : default_root_locs){
      static_file_locations[key] = root;
    }
    
    // Add port number and locations to struct
    config_info.static_file_locations = static_file_locations;
    config_info.echo_locations = echo_locations;
    config_info.port_num = port_num;
    //add more info here as needed

    return true;

}


//This technically wastes effort as we do the i loop redundantly
//we could use vectors or something to only run this once
//however this only really matters if we have a ridiculously long config, so it's probably not worth implementing
//as i is realistically going to be like 5 at most
bool NginxConfigParser::ExtractConfigLayer(NginxConfig* config, std::string target){
    for(int i = 0; i < (*config).statements_.size(); i++){ 
      //parse outermost config to find the target config
      for(int j = 0; j < (*(*config).statements_[i]).tokens_.size(); j++){
          if((*(*config).statements_[i]).tokens_[j] == target){ //found config
          BOOST_LOG_TRIVIAL(info) << (target + " Config Found");
          internal_config_  = (*(*(*config).statements_[i]).child_block_); //workaround because the nginx config thing uses unique pointers and .release kept segfaulting.
          return true; //when we find it, we're all good to return
        }
      }
    }
  BOOST_LOG_TRIVIAL(info) << (target + " Config Not Found");
  return false;
}


bool NginxConfigParser::GetServerSettings(NginxConfig* config){

    //find the specific config we need
    if(!ExtractConfigLayer(config, "server")){
      return false;
    }
    //call the respective config handler
    if(!GetServerSettingsInner()){
      return false;
    }


    /*
    //new config method: just copy these two commands.
    //first do extractconfiglayer(config, "string_to_search")
    //then, call your custom function to deal with the extracted info.
    if(!ExtractConfigLayer(config, "foo")){
      return false;
    }
    if(!GetFooSettingsInner()){
      return false;
    }
    */
    return true;
}

int NginxConfigParser::GetPortNum() {
  return config_info.port_num;
}

ConfigInfo NginxConfigParser::GetConfigInfo() {
  return config_info;
}