// An nginx config file parser.
//
// See:
//   http://wiki.nginx.org/Configuration
//   http://blog.martinfjordvald.com/2010/07/nginx-primer/
//
// How Nginx does it:
//   http://lxr.nginx.org/source/src/core/ngx_conf_file.c
#include "config_parser.h"

#include <fstream>
#include <map>
#include <stack>
#include <string>
#include <vector>
#include <boost/log/trivial.hpp>

using namespace std; 
namespace sev_lvl = boost::log::trivial;

//init for logs
NginxConfigParser::NginxConfigParser() : process_("Config Parser"), severity_(sev_lvl::info) {
  lg_.add_attribute("Process", process_);
  lg_.add_attribute("Severity", severity_);
};

//Note: this is for the NginxConfig class, it was given combined so I left it in the same .cc
string NginxConfig::ToString(int depth) {
  string serialized_config;
  for (const auto& statement : statements_) {
    serialized_config.append(statement->ToString(depth));
  }
  return serialized_config;
}

//and this is for the statement class
string NginxConfigStatement::ToString(int depth) {
  string serialized_statement;
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


NginxConfigParser::TokenType NginxConfigParser::parse_token(istream* input,
                                                           string* value) {
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

bool NginxConfigParser::parse(istream* config_file, NginxConfig* config) {
  stack<NginxConfig*> config_stack;
  config_stack.push(config);
  TokenType last_token_type = TOKEN_TYPE_START;
  TokenType token_type;
  int indent_count = 0;
  while (true) {
    string token;
    token_type = parse_token(config_file, &token);
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
        severity_.set(sev_lvl::error);
        BOOST_LOG(lg_) << "Mismatched amount of start/end blocks";
        return false;
      }
      return true;
    } else {
      // Error. Unknown token.
      break;
    }
    last_token_type = token_type;
  }
  severity_.set(sev_lvl::error);
  BOOST_LOG(lg_) << "Bad transition from " << token_type_as_string(last_token_type) << " to " << token_type_as_string(token_type);
  return false;
}
bool NginxConfigParser::parse(const char* file_name, NginxConfig* config) {
  ifstream config_file;
  config_file.open(file_name);
  if (!config_file.good()) {
    severity_.set(sev_lvl::error);
    BOOST_LOG(lg_) << "Failed to open config file: " << file_name;
    return false;
  }
  const bool return_value =
      parse(dynamic_cast<istream*>(&config_file), config);
  config_file.close();
  return return_value;
}

//TODO: once changes are merged I'm going to rename these functions
bool NginxConfigParser::get_server_settings_inner(){

    NginxConfig* server_config = &internal_config_; //set to internal config

    //init
    int port_num = -1;
    vector<string> seen_locations; 
    map<string, string> location_to_handler = map<string,string>();
    map<string, map<string, string>> location_to_directives = map<string, map<string, string>>();
    string arg_token = "";

    //iterate through the config to extract data
    for (int i = 0; i < server_config->statements_.size(); i++) {
      vector<string> tokens = (*server_config->statements_[i]).tokens_;
      //iterate through the config lines to find each argument
      for(int j = 0; j < tokens.size(); j++){
        arg_token = tokens[j]; //since we check this a lot, just store it.
        //need to check this so j+1 doesn't segfault, not doing this in main loop in case someone needs to check the last thing.
        if(j < tokens.size() - 1){
          //handles port
          if(arg_token == "port"){
            port_num = stoi(tokens[j + 1]);        
            severity_.set(sev_lvl::info);
            BOOST_LOG(lg_) << "Port Number found : " << (port_num);
          }
          //can add more args as needed here, following the pattern for ports

          //keep this one last as it's more complex
          // handles location
          else if(arg_token == "location"){
            string log_output = "";
            string location = tokens[j + 1][0]=='"' || tokens[j + 1][0]=='\'' ? tokens[j+1].substr(1, tokens[j+1].length()-2) : tokens[j+1];
            string handler_type = tokens[j + 2];
            log_output = log_output + "Location: " + location + ", ";

            if (hasKey(seen_locations, location)) { // exit if duplicate path
              severity_.set(sev_lvl::error);
              BOOST_LOG(lg_) << "Duplicate path found : " << location;
              return false;
            }
          
            location_to_handler[location] = handler_type;
            NginxConfig location_block = (*(*server_config->statements_[i]).child_block_);
            
            for (int l = 0; l < location_block.statements_.size(); l++){
              vector<string> loc_tokens = (*location_block.statements_[l]).tokens_;
              // below assumes that every directive only has one parameter
              for (int m = 0; m < loc_tokens.size(); m+=2){
                // if location has not been added yet, add a new map object
                if (location_to_directives.find(location) == location_to_directives.end()) {
                  location_to_directives[location] = map<string, string>();
                }
                location_to_directives[location][loc_tokens[m]] = loc_tokens[m+1][0]=='"' || loc_tokens[m+1][0]=='\'' ? loc_tokens[m+1].substr(1, loc_tokens[m+1].length()-2) : loc_tokens[m+1];
                log_output += " directive: " + loc_tokens[m] + ", parameter: " + location_to_directives[location][loc_tokens[m]];
              }
            }
            severity_.set(sev_lvl::info);
            BOOST_LOG(lg_) << log_output;

            // add current location to the seen locations
            // remove extra slashes for consistency
            string remove_slash = location;
            while (remove_slash[remove_slash.length()-1] == '/') {
              remove_slash = remove_slash.substr(0, location.length()-1);
            }
            seen_locations.push_back(remove_slash);
          }
        }
      }
    }

    //correctness checking
    if((port_num) < 0 || (port_num) > 65353){ //65353 is the default max range for port
      severity_.set(sev_lvl::error);
      BOOST_LOG(lg_) << "Invalid port number given";
      return false;
    }
    //add more checks here as needed

    
    // Add port number and locations to struct
    config_info_.location_to_handler = location_to_handler;
    config_info_.location_to_directives = location_to_directives;
    config_info_.port_num = port_num;
    //add more info here as needed

    return true;

}

bool NginxConfigParser::hasKey(vector<string> seen_locations, string key) {
  // remove excess slashes at end
  while (key[key.length()-1] == '/') {
    key = key.substr(0, key.length()-1);
  }
  return find(seen_locations.begin(), seen_locations.end(), key) != seen_locations.end();
}


// Extracts inner part of config from any external layers (e.g. server block or http block)
// Currently unused
// bool NginxConfigParser::extract_config_layer(NginxConfig* config, string target){
//     for(int i = 0; i < (*config).statements_.size(); i++){ 
//       //parse outermost config to find the target config
//       for(int j = 0; j < (*(*config).statements_[i]).tokens_.size(); j++){
//           if((*(*config).statements_[i]).tokens_[j] == target){ //found config
//           severity_.set(sev_lvl::info);
//           BOOST_LOG(lg_) << (target + " Config Found");
//           internal_config_  = (*(*(*config).statements_[i]).child_block_); //workaround because the nginx config thing uses unique pointers and .release kept segfaulting.
//           return true; //when we find it, we're all good to return
//         }
//       }
//     }
//     severity_.set(sev_lvl::error);
//     BOOST_LOG(lg_) << (target + " Config Not Found");
//   return false;
// }


bool NginxConfigParser::get_config_settings(NginxConfig* config){

    // find the specific config we need - used if config is within http or server block
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
