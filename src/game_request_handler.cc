#include "game_request_handler.h"
#include "request_handler_factory.h"
#include "nlohmann/json.hpp"
#include <cctype>
#include <sqlite3.h>
#include <stdexcept>
#include <vector>
#include <boost/log/trivial.hpp>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <sstream>

using json = nlohmann::json;

namespace beast = boost::beast;  
namespace http = beast::http;    

// calls parent constructor
// can be used to set handler-specific variables - e.g. the root for file handler.
// all data needed should be added to the RequestHandlerData struct contained in info.h.
GameRequestHandler::GameRequestHandler(const RequestHandlerData& request_handler_data) : RequestHandler(request_handler_data), data_path_(request_handler_data.data_path) {
    handler_name_ = "GameRequestHandler";
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
        log_request(request, res_, "JSON could not be validated in for the clicker game");
        return res_;
    }

    auto body = optBody.value();

    bool success = false;

    try {
        if (request.method_string() == "PUT"){ // update function
            if (!(body.contains("username") && body.contains("session_id") && body.contains("radish_num") && body.contains("upgrades"))) {
                res_.body() = "Must have username, session id, radish number, and upgrades\n";
                log_request(request, res_, "Missing one of the fields to update or logout");
                return res_;
            }
            auto name_ptr = body.find("username");
            auto sess_id_ptr = body.find("session_id");
            auto rad_num_ptr = body.find("radish_num");
            auto upgrade_ptr = body.find("upgrades");
            success = update_values(*name_ptr, *sess_id_ptr, *rad_num_ptr, *upgrade_ptr);
            if (!success) {
                res_.body() = "Update failed\n";  
                log_request(request, res_, "Unable to update database - user may be offline or nonexistent");
                return res_;
            }
            res_.result(http::status::ok);  
            res_.body() = "Changes successful!\n";
            log_request(request, res_, "Logout successful - changes successfully made");
        }
        else if (request.method_string() == "POST") {
            if (!(body.contains("action") && body.contains("username") && body.contains("password"))) {
                res_.body() = "Must have username and password\n";
                log_request(request, res_, "Missing one of the fields for creating user or logging in");
                return res_;
            }
            auto action_ptr = body.find("action");
            auto name_ptr = body.find("username");
            auto pass_ptr = body.find("password");

            if (*action_ptr=="create") {
                success = add_user(*name_ptr, *pass_ptr);

                if (!success) {
                    res_.body() = "Unable to add user\n";  
                    log_request(request, res_, "User cannot be added - may already exist");
                    return res_;
                }

                res_.result(http::status::ok);  
                res_.body() = "User successfully added\n";
                log_request(request, res_, "User successfully added");
            }
            else if (*action_ptr=="login") {

                success = get_values(*name_ptr, *pass_ptr);

                if (!success) {
                    res_.body() = "Unable to find offline user and/or username/password mismatch\n";
                    log_request(request, res_, "Unable to log in - user may be online or not exist, or the password may be wrong");
                    return res_;
                }

                // fill out response
                json json_res_body = json{
                    {"radish_num", game_data_.radish_num},
                    {"session_id", game_data_.session_id},
                    {"upgrades", game_data_.upgrades}
                };
                res_.result(http::status::ok);  
                res_.set(http::field::content_type, "application/json");
                res_.body() = json_res_body.dump();
                log_request(request, res_, "Successfully logged in");
            }
            else {
                res_.body() = "Invalid Action\n";
                log_request(request, res_, "Given action is not create or login - try again");
                return res_;
            }
        }
        else {
            res_.body() = "Invalid Method\n";
            log_request(request, res_, "Method is not supported - only PUT and POST are supported");
            return res_;
        }
    }
    // thrown by database errors
    catch(const std::runtime_error &e) {
        res_.result(http::status::internal_server_error);
        res_.body() = e.what();
        log_request(request, res_, "Unrecoverable error happened - database issue");
        return res_;
    }
    catch(const std::exception &e){
        res_.result(http::status::unprocessable_entity);
        res_.body() = e.what();
        log_request(request, res_, "Wrong types given");
        return res_;
    } 

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

    // create the salt, then salt and hash the password
    unsigned char salt[salt_num_bytes_] = {0};
    create_salt(salt);
    unsigned char salted_password[password.length()+salt_num_bytes_];
    std::copy(password.cbegin(), password.cend(), salted_password);
    memcpy(salted_password + password.length(), salt, salt_num_bytes_);
    unsigned char hashed_pass[hashed_pass_bytes_] = {0};
    hash_password(salted_password, hashed_pass, password.length()+salt_num_bytes_);

    char const* const command = "INSERT OR IGNORE INTO users (user_id, username, hashed_pass, salt) VALUES (NULL, ?, ?, ?); ";

    sqlite3* db;

    int rc = sqlite3_open(data_path_.data(), &db);
    if (rc) {
        throw std::runtime_error("Error while opening database");
    }

    sqlite3_stmt* stmt = NULL;
    rc = sqlite3_prepare_v2(db, command, -1, &stmt, NULL);
    if (rc != SQLITE_OK ) {
        throw std::runtime_error("Error while preparing statement");
    }
    rc = sqlite3_bind_text(stmt, 1, username.c_str(), username.length(), SQLITE_STATIC);
    check_rc(rc);
    rc = sqlite3_bind_blob(stmt, 2, hashed_pass, hashed_pass_bytes_, SQLITE_STATIC);
    check_rc(rc);
    rc = sqlite3_bind_blob(stmt, 3, salt, salt_num_bytes_, SQLITE_STATIC);
    check_rc(rc);
    severity_.set(sev_lvl::trace);
    BOOST_LOG(lg_) << "SQLITE3 binded stmt: " <<sqlite3_expanded_sql(stmt);
    rc = sqlite3_step(stmt);
    check_rc(rc);
    rc = sqlite3_finalize(stmt);
    check_rc(rc);

    sqlite3_close(db);
    check_rc(rc);

    return true;
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

    sqlite3* db;

    int rc = sqlite3_open(data_path_.data(), &db);
    if (rc) {
        throw std::runtime_error("Error while opening database");
    }

    sqlite3_stmt* stmt = NULL;

    unsigned char* salt;
    int len;
    char const* const new_command = "SELECT salt FROM users WHERE username=?; ";
    rc = sqlite3_prepare_v2(db, new_command, -1, &stmt, NULL);
    if (rc != SQLITE_OK ) {
        throw std::runtime_error("Error while preparing statement");
    }
    rc = sqlite3_bind_text(stmt, 1, username.c_str(), username.length(), SQLITE_STATIC);
    check_rc(rc);
    severity_.set(sev_lvl::trace);
    BOOST_LOG(lg_) << "SQLITE3 binded stmt: " <<sqlite3_expanded_sql(stmt);
    rc = sqlite3_step(stmt);
    check_rc(rc);
    if (rc == SQLITE_ROW) {
        len = sqlite3_column_bytes(stmt, 0);
        salt = (unsigned char *)malloc(len);
        memcpy(salt, sqlite3_column_blob(stmt, 0), len);
    }
    rc = sqlite3_finalize(stmt);
    check_rc(rc);

    // salt then hash the password
    unsigned char salted_password[password.length()+salt_num_bytes_];
    std::copy(password.cbegin(), password.cend(), salted_password);
    memcpy(salted_password + password.length(), salt, salt_num_bytes_);
    unsigned char hashed_pass[hashed_pass_bytes_] = {0};
    hash_password(salted_password, hashed_pass, password.length()+salt_num_bytes_);

    char const* const commands[4] = {"SELECT radish_num FROM users WHERE username=? AND hashed_pass=?; ",
        "SELECT upgrade_type, upgrade_num FROM upgrades JOIN users ON users.user_id=upgrades.user_id WHERE username=? AND hashed_pass=?; ", 
        "INSERT OR IGNORE INTO sessions (user_id) VALUES ((SELECT user_id FROM users WHERE username =? AND hashed_pass=?)); ", 
        "SELECT session_id FROM sessions JOIN users ON sessions.user_id=users.user_id WHERE username=? AND hashed_pass=?; "};

    rc = sqlite3_exec(db, "BEGIN EXCLUSIVE TRANSACTION; ", NULL, NULL, NULL);
    check_rc(rc);

    for (int i = 0; i<4; i++) {
        rc = sqlite3_prepare_v2(db, commands[i], -1, &stmt, NULL);
        if (rc != SQLITE_OK ) {
            throw std::runtime_error("Error while preparing statement");
        }
        rc = sqlite3_bind_text(stmt, 1, username.c_str(), username.length(), SQLITE_STATIC);
        check_rc(rc);
        rc = sqlite3_bind_blob(stmt, 2, hashed_pass, hashed_pass_bytes_, SQLITE_STATIC);
        check_rc(rc);
        severity_.set(sev_lvl::trace);
        BOOST_LOG(lg_) << "SQLITE3 binded stmt: " <<sqlite3_expanded_sql(stmt);
        rc = sqlite3_step(stmt);
        check_rc(rc);
        while (rc == SQLITE_ROW) {
            std::string col_name = sqlite3_column_name(stmt, 0);
            if (col_name == "radish_num") {
                game_data_.radish_num = sqlite3_column_int(stmt, 0);
            }
            else if (col_name == "session_id") {
                game_data_.session_id = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
            }
            else if (col_name == "upgrade_type") {
                std::string type = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
                game_data_.upgrades[type] = sqlite3_column_int(stmt, 1);
            }
            rc = sqlite3_step(stmt);
            check_rc(rc);
        }
        rc = sqlite3_finalize(stmt);
        check_rc(rc);
    }

    rc = sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
    check_rc(rc);

    delete salt;
    rc = sqlite3_close(db);
    check_rc(rc);
    return true;
}

/** 
Helper function to determine whether a user exists 

@param username
@return true if user exists, false otherwise
*/
bool GameRequestHandler::user_exists(std::string username) {
    char const* const command = "SELECT * FROM users WHERE username=?; ";

    sqlite3* db;

    int rc = sqlite3_open(data_path_.data(), &db);
    if (rc) {
        throw std::runtime_error("Error while opening database");
    }
    bool has_user = false;
    sqlite3_stmt* stmt = NULL;
    rc = sqlite3_prepare_v2(db, command, -1, &stmt, NULL);
    if (rc != SQLITE_OK ) {
        throw std::runtime_error("Error while preparing statement");
    }
    rc = sqlite3_bind_text(stmt, 1, username.c_str(), username.length(), SQLITE_STATIC);
    check_rc(rc);
    severity_.set(sev_lvl::trace);
    BOOST_LOG(lg_) << "SQLITE3 binded stmt: " <<sqlite3_expanded_sql(stmt);
    rc = sqlite3_step(stmt);
    check_rc(rc);
    if (rc == SQLITE_ROW) {
        has_user = true;
    }
    rc = sqlite3_finalize(stmt);
    check_rc(rc);

    rc = sqlite3_close(db);
    check_rc(rc);

    return has_user;
}

/**
Helper function to determine whether a user is online before "logging in" or "logging out"

@param username Inputted name
@return true if user is online, false otherwise
*/
bool GameRequestHandler::is_online(std::string username) {
    char const* const command = "SELECT session_id FROM sessions JOIN users ON sessions.user_id=users.user_id WHERE username=?; ";

    sqlite3* db;

    int rc = sqlite3_open(data_path_.data(), &db);
    if (rc) {
        throw std::runtime_error("Error while opening database");
    }

    sqlite3_stmt* stmt = NULL;
    bool online = false;
    rc = sqlite3_prepare_v2(db, command, -1, &stmt, NULL);
    if (rc != SQLITE_OK ) {
        throw std::runtime_error("Error while preparing statement");
    }
    rc = sqlite3_bind_text(stmt, 1, username.c_str(), username.length(), SQLITE_STATIC);
    check_rc(rc);
    severity_.set(sev_lvl::trace);
    BOOST_LOG(lg_) << "SQLITE3 binded stmt: " << sqlite3_expanded_sql(stmt);
    rc = sqlite3_step(stmt);
    check_rc(rc);
    if (rc == SQLITE_ROW) {
        online = true;
    }
    rc = sqlite3_finalize(stmt);
    check_rc(rc);

    rc = sqlite3_close(db);
    check_rc(rc);

    return online;
}

/**
Helper function to check whether session ID matches for given user

@param username Inputted name
@param session_id the id provided by the request
@return whether session id associated with the user matches
*/
bool GameRequestHandler::has_session_id(std::string username, std::string session_id) {
    char const* const command = "SELECT session_id FROM sessions JOIN users ON sessions.user_id=users.user_id WHERE username=? AND session_id=?; ";

    sqlite3* db;

    int rc = sqlite3_open(data_path_.data(), &db);
    if (rc) {
        throw std::runtime_error("Error while opening database");
    }

    sqlite3_stmt* stmt = NULL;
    bool has_session_id = false;
    rc = sqlite3_prepare_v2(db, command, -1, &stmt, NULL);
    if (rc != SQLITE_OK ) {
        throw std::runtime_error("Error while preparing statement");
    }
    rc = sqlite3_bind_text(stmt, 1, username.c_str(), username.length(), SQLITE_STATIC);
    check_rc(rc);
    rc = sqlite3_bind_text(stmt, 2, session_id.c_str(), 16, SQLITE_STATIC);
    check_rc(rc);
    severity_.set(sev_lvl::trace);
    BOOST_LOG(lg_) << "SQLITE3 binded stmt: " << sqlite3_expanded_sql(stmt);
    rc = sqlite3_step(stmt);
    check_rc(rc);
    if (rc == SQLITE_ROW) {
        has_session_id = true;
    }
    rc = sqlite3_finalize(stmt);
    check_rc(rc);

    rc = sqlite3_close(db);
    check_rc(rc);

    return has_session_id;
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
    if (!is_online(username) || has_session_id(username, session_id)) {
        return false;
    }

    sqlite3* db;

    int rc = sqlite3_open(data_path_.data(), &db);
    if (rc) {
        throw std::runtime_error("Error while opening database");
    }

    std::vector<std::string> commands;
    std::vector<std::pair<std::string, int>> upgrades_list;
    // radish num, username, username, session id
    commands.push_back("UPDATE users SET radish_num=? WHERE username=? AND EXISTS (SELECT 1 FROM sessions JOIN users ON sessions.user_id=users.user_id WHERE username=? AND session_id=?); ");
    for (const auto& upgrade : upgrades) {
        std::string upgrade_type = upgrade.first;
        int upgrade_num = upgrade.second;
        // username, session_id, upgrade type, upgrade num
        commands.push_back("INSERT OR REPLACE INTO upgrades (user_id, upgrade_type, upgrade_num) VALUES ((SELECT users.user_id FROM sessions JOIN users ON sessions.user_id=users.user_id WHERE username=? AND session_id=?), ?, ?); ");
        upgrades_list.push_back(make_pair(upgrade_type, upgrade_num));
    }
    // session id, username
    commands.push_back("DELETE FROM sessions WHERE session_id=? AND user_id=(SELECT user_id FROM users WHERE username=?);");

    sqlite3_stmt* stmt = NULL;

    rc = sqlite3_exec(db, "BEGIN EXCLUSIVE TRANSACTION; ", NULL, NULL, NULL);
    check_rc(rc);

    for (int i = 0; i<commands.size(); i++) {
        char const* const command = commands[i].c_str();
        rc = sqlite3_prepare_v2(db, command, -1, &stmt, NULL);
        if (rc != SQLITE_OK ) {
            throw std::runtime_error("Error while preparing statement");
        }

        if (i == 0) {
            rc = sqlite3_bind_int(stmt, 1, radish_num);
            check_rc(rc);
            rc = sqlite3_bind_text(stmt, 2, username.c_str(), username.length(), SQLITE_STATIC);
            check_rc(rc);
            rc = sqlite3_bind_text(stmt, 3, username.c_str(), username.length(), SQLITE_STATIC);
            check_rc(rc);
            rc = sqlite3_bind_text(stmt, 4, session_id.c_str(), session_id.length(), SQLITE_STATIC);
            check_rc(rc);
        }
        else if (i == commands.size()-1) {
            rc = sqlite3_bind_text(stmt, 1, session_id.c_str(), session_id.length(), SQLITE_STATIC);
            check_rc(rc);
            rc = sqlite3_bind_text(stmt, 2, username.c_str(), username.length(), SQLITE_STATIC);
            check_rc(rc);
        }
        else {
            std::string upgrade_type = upgrades_list[i-1].first;
            int upgrade_num = upgrades_list[i-1].second;
            rc = sqlite3_bind_text(stmt, 1, username.c_str(), username.length(), SQLITE_STATIC);
            check_rc(rc);
            rc = sqlite3_bind_text(stmt, 2, session_id.c_str(), session_id.length(), SQLITE_STATIC);
            check_rc(rc);
            rc = sqlite3_bind_text(stmt, 3, upgrade_type.c_str(), upgrade_type.length(), SQLITE_STATIC);
            check_rc(rc);
            rc = sqlite3_bind_int(stmt, 4, upgrade_num);
            check_rc(rc);
        }

        severity_.set(sev_lvl::trace);
        BOOST_LOG(lg_) << "SQLITE3 binded stmt: " << sqlite3_expanded_sql(stmt);
        rc = sqlite3_step(stmt);
        check_rc(rc);
        rc = sqlite3_finalize(stmt);
        check_rc(rc);
    }

    rc = sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL);
    check_rc(rc);

    rc = sqlite3_close(db);
    check_rc(rc);
    return true;
}

void GameRequestHandler::create_salt(unsigned char* salt) {
    RAND_bytes(salt, salt_num_bytes_);
}

void GameRequestHandler::hash_password(unsigned char* salted_pass, unsigned char* hashed_pass, int len) {
    SHA256(salted_pass, len, hashed_pass);
}

void GameRequestHandler::check_rc(int rc) {
    if (rc != SQLITE_OK && rc != SQLITE_ROW && rc != SQLITE_DONE) {
        throw std::runtime_error("SQLITE ERROR " + std::to_string(rc));
    }
}