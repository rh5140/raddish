#include "game_request_handler.h"
#include "request_handler_factory.h"
#include "nlohmann/json.hpp"
#include <cctype>
#include <sqlite3.h>
#include <vector>
#include <boost/log/trivial.hpp>

using json = nlohmann::json;

namespace beast = boost::beast;  
namespace http = beast::http;    

// calls parent constructor
// can be used to set handler-specific variables - e.g. the root for file handler.
// all data needed should be added to the RequestHandlerData struct contained in info.h.
GameRequestHandler::GameRequestHandler(const RequestHandlerData& request_handler_data) : RequestHandler(request_handler_data), data_path_(request_handler_data.data_path) {
}

// factory function
// defined as a static function in the header - added to registry map on startup.
RequestHandler* GameRequestHandler::init(const RequestHandlerData& request_handler_data) {
    return new GameRequestHandler(request_handler_data); 
}

// add self-init to registry on startup. 
// defined as a static bool in the header file.
bool GameRequestHandler::registered_ = RequestHandlerFactory::register_handler("GameRequestHandler", GameRequestHandler::init);

// implementation of handle_request
http::response<http::string_body> GameRequestHandler::handle_request(const http::request<http::string_body>& request) {
    init_response(request);

    std::string string_body = request.body();

    auto optBody = validate_json(string_body);

    // set as default status code
    res_.result(http::status::bad_request);

    if (!optBody.has_value()) {
        res_.body() = "Invalid JSON\n";
        log_request(request, res_, "Game request handled");
        return res_;
    }

    auto body = optBody.value();

    bool success = false;

    // TODO using binding to prevent injection!
    try {
        if (request.method_string() == "GET") {
            if (!(body.contains("username") && body.contains("password"))) {
                res_.body() = "Must have username and password\n";
                log_request(request, res_, "Game request handled");
                return res_;
            }
            auto name_ptr = body.find("username");
            auto pass_ptr = body.find("password");
            success = get_values(*name_ptr, *pass_ptr);

            if (!success) {
                res_.body() = "Unable to find offline user and/or username/password mismatch\n";
                log_request(request, res_, "Game request handled");
                return res_;
            }

            // fill out response
            json json_res_body = json{
                {"radish_num", game_data_.radish_num},
                {"session_id", game_data_.session_id},
                {"upgrades", game_data_.upgrades}
            };

            res_.set(http::field::content_type, "application/json");
            res_.body() = json_res_body.dump();
        }
        else if (request.method_string() == "PUT"){ // update function
            if (!(body.contains("username") && body.contains("session_id") && body.contains("radish_num") && body.contains("upgrades"))) {
                res_.body() = "Must have username, session id, radish number, and upgrades\n";
                log_request(request, res_, "Game request handled");
                return res_;
            }
            auto name_ptr = body.find("username");
            auto sess_id_ptr = body.find("session_id");
            auto rad_num_ptr = body.find("radish_num");
            auto upgrade_ptr = body.find("upgrades");
            success = update_values(*name_ptr, *sess_id_ptr, *rad_num_ptr, *upgrade_ptr);
            if (!success) {
                res_.body() = "Update failed\n";  
                log_request(request, res_, "Game request handled");
                return res_;
            }
            res_.body() = "Changes successful!\n";
        }
        else if (request.method_string() == "POST") {
            if (!(body.contains("username") && body.contains("password"))) {
                res_.body() = "Must have username and password\n";
                log_request(request, res_, "Game request handled");
                return res_;
            }
            auto name_ptr = body.find("username");
            auto pass_ptr = body.find("password");
            success = add_user(*name_ptr, *pass_ptr);

            if (!success) {
                res_.body() = "Unable to add user\n";  
                log_request(request, res_, "Game request handled");
                return res_;
            }

            res_.body() = "User successfully added\n";
        }
        else {
            log_request(request, res_, "Game request handled");
            res_.body() = "Invalid Method\n";
            return res_;
        }
    }
    catch(...){
        res_.result(http::status::unprocessable_entity);
        res_.body() = "Error with database\n";
        log_request(request, res_, "Game request handled");
        return res_;
    }

    //set vars
    res_.result(http::status::ok);  

    //log
    log_request(request, res_, "Game request handled");
    return res_;
}

std::optional<json> GameRequestHandler::validate_json(const std::string& json) {
    try {
        return json::parse(json);
    } catch (const json::parse_error&) {
        return std::nullopt;
    }
}

/**
Creates a user with the given password.

Adds the username and password pair to the database.

@param username given name for the user
@param password provided password
@return true if successfully added, false otherwise
*/

bool GameRequestHandler::add_user(std::string username, std::string password) {

    if (user_exists(username)) {
        return false;
    }

    // ** TODO hash the password, use binding to pass it along as unsigned char* **
    // unsigned char salt[salt_num_bytes_];
    // create_salt(salt);
    // password.append((char*)salt, salt_num_bytes_); 
    // unsigned char hashed_pass[hashed_pass_bytes_];
    // hash_password(password, hashed_pass);

    // temp - remember to also change password to hashed_pass
    std::string salt = "temporary";

    std::string sql_string = "INSERT OR IGNORE INTO users (user_id, username, hashed_pass, salt) VALUES (NULL, '" + username + "', '" + password + "', '" + salt + "'); ";

    return run_sql(sql_string);
}

/**
Gets game values of a user.

Given a username, either retrieves info pertaining to it or empty struct if invalid (e.g. user is online).
The info is stored in the private variable game_data_.

@param username The user that we should be retrieving information about
@return true if successful, false if not 
*/
bool GameRequestHandler::get_values(std::string username, std::string password) {

    if (is_online(username) || !user_exists(username)) {
        return false;
    }

    // ** TODO: hash the pass, will be bound to statement before execution **
    // std::string sql_string = "SELECT salt FROM users WHERE username='" + username + "'; ";
    // run_sql(sql_string);
    // password.append((char *)salt_, salt_num_bytes_);
    // unsigned char* hashed_pass = hash_password(password);

    // remember to change password to hashed_pass
    std::string sql_string = 
        "BEGIN EXCLUSIVE TRANSACTION; "
        "SELECT radish_num FROM users WHERE username='" + username + "' AND hashed_pass='" + password + "'; " 
        "SELECT upgrade_type, upgrade_num FROM upgrades JOIN users ON users.user_id=upgrades.user_id WHERE username='" + username + "' AND hashed_pass='" + password + "'; " 
        "INSERT OR IGNORE INTO sessions (user_id) VALUES ((SELECT user_id FROM users WHERE username = '" + username + "' AND hashed_pass='" + password + "')); " 
        "SELECT session_id FROM sessions JOIN users ON sessions.user_id=users.user_id WHERE username='" + username + "' AND hashed_pass='" + password + "'; "
        "COMMIT;";

    return run_sql(sql_string);
}

/** 
Helper function to determine whether a user exists 

@param username
@return true if user exists, false otherwise
*/
bool GameRequestHandler::user_exists(std::string username) {
    std::string sql_string = "SELECT * FROM users WHERE username='" + username + "'; ";

    game_data_.radish_num = -1; // clear out just in case

    bool success = run_sql(sql_string);

    return game_data_.radish_num != -1;
}

/**
Helper function to determine whether a user is online before "logging in" or "logging out"

@param username Inputted name
@return true if user is online, false otherwise
*/
bool GameRequestHandler::is_online(std::string username) {

    std::string sql_string = "SELECT session_id FROM sessions JOIN users ON sessions.user_id=users.user_id WHERE username='" + username + "'; ";

    game_data_.session_id = ""; // clear out just in case

    bool success = run_sql(sql_string);

    if (game_data_.session_id != "") {
        return true;
    }

    return false;
}

/**
Stores game values of a user.

Attempts to update given values in the DB - returns boolean that shows whether it was successful.

@param username The user we are setting the values for
@param session_id The session ID associated with the user, used to check whether callee is the same as the logged in user
@param radish_num The number of radishes (currency) the user currently has
@param upgrades A mapping of upgrade types to number of that specific upgrade
*/
bool GameRequestHandler::update_values(std::string username, std::string session_id, int radish_num, std::map<std::string, int> upgrades){
    
    // check whether user is online first - the session_id is then stored
    // we can then use it to match against given session_id
    if (!is_online(username) || game_data_.session_id != session_id) {
        return false;
    }

    std::string sql_string = "BEGIN EXCLUSIVE TRANSACTION; ";
    sql_string += "UPDATE users SET radish_num="+std::to_string(radish_num)+" WHERE username='"+username+"' AND EXISTS (SELECT 1 FROM sessions JOIN users ON sessions.user_id=users.user_id WHERE username='"+username+"' AND session_id='"+session_id+"'); ";
    for (const auto& upgrade : upgrades) {
        std::string upgrade_type = upgrade.first;
        std::string upgrade_num = std::to_string(upgrade.second);
        sql_string += "INSERT OR REPLACE INTO upgrades (user_id, upgrade_type, upgrade_num) VALUES ((SELECT users.user_id FROM sessions JOIN users ON sessions.user_id=users.user_id WHERE username='"+username+"' AND session_id='"+session_id+"'), '"+upgrade_type+"', "+upgrade_num+"); ";
    }
    sql_string += 
        "DELETE FROM sessions WHERE session_id='"+session_id+"' AND user_id=(SELECT user_id FROM users WHERE username='"+username+"');"
        "COMMIT;";

    return run_sql(sql_string);
}

/**
Runs input SQL 
*/
// example of binding: https://stackoverflow.com/questions/34179449/sqlite-inserting-blob-bind-or-column-index-out-of-range/34179660#34179660
// param should be array of strings
// add users - text, blob, blob 
// get values - text, blob multiple times
// is online - just text
// update values - unique for each - gotta write them separately :(
bool GameRequestHandler::run_sql(std::string sql_string){
    sqlite3* db;
    char *zErrMsg = 0;

    int rc = sqlite3_open(data_path_.data(), &db);
    if (rc) {
        BOOST_LOG_TRIVIAL(error) << "Can't open database: " << sqlite3_errmsg(db);
        return false;
    } 
    else {
        BOOST_LOG_TRIVIAL(info) << "Opened database successfully";
    }

    char *sql = sql_string.data();
    rc = sqlite3_exec(db, sql, callback, (void*)&game_data_, &zErrMsg);

    if (rc != SQLITE_OK) {
        BOOST_LOG_TRIVIAL(error) << "SQL error: " << zErrMsg;
        sqlite3_free(zErrMsg);
        sqlite3_close(db);
        return false;
    } 
    else {
        BOOST_LOG_TRIVIAL(info) << "Operation done successfully";
    }

    sqlite3_close(db);
    return true;
}


/**
callback function called for every row returned
*/
static int callback(void *data, int argc, char **argv, char **azColName){
    int i;

    GameData* game_data = (GameData*) data;

    for(i = 0; i<argc; i++) {
        BOOST_LOG_TRIVIAL(trace) << azColName[i] << " " << argv[i];
        std::string col_name = azColName[i];
        if (col_name == "radish_num") {
            game_data->radish_num = atoi(argv[i]);
        }
        else if (col_name == "upgrade_type") {
            game_data->upgrades[argv[i]] = atoi(argv[i+1]);
            i++;
        }
        else if (col_name == "session_id") {
            game_data->session_id = argv[i];
        }
    }
    return 0;
}

// using this for the functions below: https://stackoverflow.com/questions/10273414/library-for-passwords-salt-hash-in-c

// TODO: in progress 
void GameRequestHandler::create_salt(unsigned char* salt) {

    // from # include <openssl/rand.h>
    // RAND_bytes(salt, salt_num_bytes_);

}

// TODO in progress
//  # include <openssl/sha.h>
void GameRequestHandler::hash_password(std::string salted_pass, unsigned char* hashed_pass) {
    // SHA256_CTX context;

    // SHA256_Init(&context);
    // SHA256_Update(&context, (unsigned char*)salted_password, length);
    // SHA256_Final(hashed_pass, &context);
}