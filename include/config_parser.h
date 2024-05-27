#ifndef CONFIG_PARSER
#define CONFIG_PARSER
// An nginx config file parser.

#include "info.h"

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <boost/log/attributes.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/logger.hpp>

namespace logging = boost::log;
namespace attrs = boost::log::attributes;
namespace sev_lvl = boost::log::trivial;

class NginxConfig;
// The parsed representation of a single config statement.
class NginxConfigStatement {
 public:
  std::string ToString(int depth);
  std::vector<std::string> tokens_;
  std::unique_ptr<NginxConfig> child_block_;
};
// The parsed representation of the entire config.
class NginxConfig {
 public:
  std::string ToString(int depth = 0);
  std::vector<std::shared_ptr<NginxConfigStatement>> statements_;
};
// The driver that parses a config file and generates an NginxConfig.
class NginxConfigParser {
 public:
  NginxConfigParser();
  // Take a opened config file or file name (respectively) and store the
  // parsed config in the provided NginxConfig out-param.  Returns true
  // iff the input config file is valid.
  bool parse(std::istream* config_file, NginxConfig* config);
  bool parse(const char* file_name, NginxConfig* config);
  bool get_config_settings(NginxConfig* config);
  const char* number_to_token_string(int n); //used for testing without exposing private methods
  std::map<std::string, std::string> get_static_file_locations();
  // std::vector<std::string> get_echo_locations();
  int get_port_num();
  ConfigInfo get_config_info();
 private:
  bool hasKey(std::vector<std::string> mapping, std::string key);
  // std::map<std::string, std::string> locations;
  ConfigInfo config_info_;
  enum TokenType {
    TOKEN_TYPE_START = 0,
    TOKEN_TYPE_NORMAL = 1,
    TOKEN_TYPE_START_BLOCK = 2,
    TOKEN_TYPE_END_BLOCK = 3,
    TOKEN_TYPE_COMMENT = 4,
    TOKEN_TYPE_STATEMENT_END = 5,
    TOKEN_TYPE_EOF = 6,
    TOKEN_TYPE_ERROR = 7
  };
  const char* token_type_as_string(TokenType type);
  enum TokenParserState {
    TOKEN_STATE_INITIAL_WHITESPACE = 0,
    TOKEN_STATE_SINGLE_QUOTE = 1,
    TOKEN_STATE_DOUBLE_QUOTE = 2,
    TOKEN_STATE_TOKEN_TYPE_COMMENT = 3,
    TOKEN_STATE_TOKEN_TYPE_NORMAL = 4
  };
  TokenType parse_token(std::istream* input, std::string* value);

  bool extract_config_layer(NginxConfig* config, std::string target);
  bool get_server_settings_inner();

  NginxConfig internal_config_; //used for extracting internal configs

  // for logger
  logging::sources::logger lg_;
  attrs::mutable_constant<std::string> process_;
  attrs::mutable_constant<sev_lvl::severity_level> severity_;
};

#endif