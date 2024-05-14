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
const char* NginxConfigParser::token_type_as_string(TokenType type) {
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
const char* NginxConfigParser::number_to_token_string(int n){
  switch(n){
    case 0:
      return token_type_as_string(TOKEN_TYPE_START);
    case 1:
      return token_type_as_string(TOKEN_TYPE_NORMAL);
    case 2:
      return token_type_as_string(TOKEN_TYPE_START_BLOCK);
    case 3:
      return token_type_as_string(TOKEN_TYPE_END_BLOCK);
    case 4:
      return token_type_as_string(TOKEN_TYPE_COMMENT);
    case 5:
      return token_type_as_string(TOKEN_TYPE_STATEMENT_END);
    case 6:
      return token_type_as_string(TOKEN_TYPE_EOF);
    default:
      return token_type_as_string(TOKEN_TYPE_ERROR);
  }
}


NginxConfigParser::TokenType NginxConfigParser::parse_token(std::istream* input,
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

bool NginxConfigParser::parse(std::istream* config_file, NginxConfig* config) {
  std::stack<NginxConfig*> config_stack;
  config_stack.push(config);
  TokenType last_token_type = TOKEN_TYPE_START;
  TokenType token_type;
  int indent_count = 0;
  while (true) {
    std::string token;
    token_type = parse_token(config_file, &token);
    //printf ("%s: %s\n", token_type_as_string(token_type), token.c_str());
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
  BOOST_LOG_TRIVIAL(warning) << "Bad transition from " << token_type_as_string(last_token_type) << " to " << token_type_as_string(token_type);
  return false;
}
bool NginxConfigParser::parse(const char* file_name, NginxConfig* config) {
  std::ifstream config_file;
  config_file.open(file_name);
  if (!config_file.good()) {
    BOOST_LOG_TRIVIAL(error) << "Failed to open config file: " << file_name;
    return false;
  }
  const bool return_value =
      parse(dynamic_cast<std::istream*>(&config_file), config);
  config_file.close();
  return return_value;
}

//TODO: once changes are merged I'm going to rename these functions
bool NginxConfigParser::get_server_settings_inner(){

    NginxConfig* server_config = &internal_config_; //set to internal config

    //init
    int port_num = -1;
    vector<std::string> seen_locations;
    std::map<std::string, std::string> location_to_handler = std::map<std::string,std::string>();
    std::map<std::string, std::string> location_to_root = std::map<std::string,std::string>();
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
          if(arg_token == "port"){
            port_num = stoi(tokens[j + 1]);
            BOOST_LOG_TRIVIAL(info) << "Port Number found : " << (port_num);
          }
          //can add more args as needed here, following the pattern for ports

          //keep this one last as it's more complex
          // handles location
          else if(arg_token == "location"){
            std::string log_output = "";
            std::string key = tokens[j + 1][0]=='"' || tokens[j + 1][0]=='\'' ? tokens[j+1].substr(1, tokens[j+1].length()-2) : tokens[j+1];
            std::string type = tokens[j + 2];
            log_output = log_output + "Location: " + key + ", ";
            std::string remove_slash = key;
          
            location_to_handler[key] = type;
            NginxConfig location_block = (*(*server_config->statements_[i]).child_block_);
            
            if (type == "FileRequestHandler") {
              for (int l = 0; l < location_block.statements_.size(); l++){
                std::vector<std::string> loc_tokens = (*location_block.statements_[l]).tokens_;
                for (int m = 0; m < loc_tokens.size(); m++){
                  if(loc_tokens[m] == "root"){
                    if (hasKey(seen_locations, key)) { // exit if duplicate path
                      BOOST_LOG_TRIVIAL(error) << key << " is a duplicate path, static file request handler";
                      return false;
                    }
                    // assumes parser has done job, only single quote around entire path
                    location_to_root[key] = loc_tokens[m+1][0]=='"' || loc_tokens[m+1][0]=='\'' ? loc_tokens[m+1].substr(1, loc_tokens[m+1].length()-2) : loc_tokens[m+1];
                    log_output = log_output + "root: " + location_to_root[key] + " for serving static files";
                  }
                }
              }
            }
            else if (type == "EchoRequestHandler") {
              if (hasKey(seen_locations, key)) { // make sure echo locations don't overlap either
                BOOST_LOG_TRIVIAL(error) << key << " is a duplicate path, echo request handler";
                return false;
              }
              log_output = log_output + "for echoing";
            }
            BOOST_LOG_TRIVIAL(info) << log_output;

            // add key to the seen locations
            while (remove_slash[remove_slash.length()-1] == '/') {
              remove_slash = remove_slash.substr(0, key.length()-1);
            }
            seen_locations.push_back(remove_slash);
          }
        }
      }
    }

    //correctness checking
    if((port_num) < 0 || (port_num) > 65353){ //65353 is the default max range for port
      BOOST_LOG_TRIVIAL(error) << "invalid port number";
      return false;
    }
    //add more checks here as needed

    
    // Add port number and locations to struct
    config_info_.location_to_handler = location_to_handler;
    config_info_.location_to_root = location_to_root;
    config_info_.port_num = port_num;
    //add more info here as needed

    return true;

}

bool NginxConfigParser::hasKey(std::vector<std::string> seen_locations, std::string key) {
  // remove excess slashes at end
  while (key[key.length()-1] == '/') {
    key = key.substr(0, key.length()-1);
  }
  return std::find(seen_locations.begin(), seen_locations.end(), key) != seen_locations.end();
}


//This technically wastes effort as we do the i loop redundantly
//we could use vectors or something to only run this once
//however this only really matters if we have a ridiculously long config, so it's probably not worth implementing
//as i is realistically going to be like 5 at most
bool NginxConfigParser::extract_config_layer(NginxConfig* config, std::string target){
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


bool NginxConfigParser::get_config_settings(NginxConfig* config){

    //find the specific config we need
    // if(!extract_config_layer(config, "server")){
    //   return false;
    // }
    
    // remove below if extracting config layer
    internal_config_ = *config;
    
    //call the respective config handler
    if(!get_server_settings_inner()){
      return false;
    }


    /*
    //new config method: just copy these two commands.
    //first do extract_config_layer(config, "string_to_search")
    //then, call your custom function to deal with the extracted info.
    if(!extract_config_layer(config, "foo")){
      return false;
    }
    if(!GetFooSettingsInner()){
      return false;
    }
    */
    return true;
}

int NginxConfigParser::get_port_num() {
  return config_info_.port_num;
}

ConfigInfo NginxConfigParser::get_config_info() {
  return config_info_;
}