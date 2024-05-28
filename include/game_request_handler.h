#ifndef GAME_REQUEST_HANDLER
#define GAME_REQUEST_HANDLER

#include "request_handler.h"
#include <optional>
#include <map>
#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>

struct GameData {
    int radish_num;
    std::map<std::string, int> upgrades;
    std::string session_id;
};

static int callback(void *data, int argc, char **argv, char **azColName);
    
class GameRequestHandler : public RequestHandler {
    public:
        GameRequestHandler(const RequestHandlerData& request_handler_data);
        http::response<http::string_body> handle_request(const http::request<http::string_body>& request);
        static RequestHandler* init(const RequestHandlerData& request_handler_data);
        static bool registered_;
    private:
        std::optional<json> validate_json(const std::string& json);
        bool add_user(std::string username, std::string password);
        bool get_values(std::string username, std::string password);
        bool update_values(std::string username, std::string session_id, int radish_num, std::map<std::string, int> upgrades);
        bool is_online(std::string username);
        bool user_exists(std::string username);
        bool run_sql(std::string sql_string);
        void create_salt(unsigned char* salt);
        void hash_password(std::string salted_pass, unsigned char* hashed_pass);
        int salt_num_bytes_ = 32;
        int hashed_pass_bytes_ = 32;
        std::string data_path_;
        GameData game_data_;
        unsigned char* salt_;
};


#endif