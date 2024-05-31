#include <gtest/gtest.h>
#include "game_request_handler.h"
#include "nlohmann/json.hpp"
#include <sqlite3.h>
#include <map>

class GameHandlerTest : public testing::Test {
protected:
    GameRequestHandler* handler;
    RequestHandlerData request_handler_data;
    sqlite3* db;
    

    void SetUp() override {
        char *zErrMsg = 0;
        // open DB in memory - shared between setup and individual tests
        int rc = sqlite3_open("file::memory:?cache=shared", &db);
        std::string sql_string = 
        "CREATE TABLE sessions ("
            "user_id INTEGER PRIMARY KEY,"
            "session_id CHAR(16) NOT NULL DEFAULT (lower(hex(randomblob(16)))),"
            "FOREIGN KEY (user_id) REFERENCES users (user_id)"
        ");"
        "CREATE TABLE upgrades ("
            "user_id INTEGER, "
            "upgrade_type VARCHAR(255),"
            "upgrade_num INTEGER NOT NULL DEFAULT 0,"
            "FOREIGN KEY (user_id) REFERENCES user (user_id), PRIMARY KEY (user_id, upgrade_type)"
        ");"
        "CREATE TABLE users ("
            "user_id INTEGER PRIMARY KEY,"
            "username TEXT UNIQUE NOT NULL,"
            "hashed_pass BLOB NOT NULL,"
            "salt BLOB NOT NULL,"
            "radish_num INTEGER NOT NULL DEFAULT 0"
        ");";
        char *sql = sql_string.data();
        // create the tables
        rc = sqlite3_exec(db, sql, nullptr, nullptr, nullptr);

        request_handler_data.data_path = "file::memory:?cache=shared";
        handler = new GameRequestHandler(request_handler_data);
    }

    void TearDown() override {
        delete handler;
        sqlite3_close(db);
    }

    http::request<http::string_body> construct_request(http::verb verb, std::string body) {
        http::request<http::string_body> req(verb, "/clicker_game/data", 11);
        req.set(http::field::content_type, "application/json");
        req.body() = body;
        req.prepare_payload();
        return req;
    }
};

TEST_F(GameHandlerTest, TestFactory) {
  EXPECT_NO_THROW(GameRequestHandler::init(request_handler_data));
}

// general flow
// make account, get data, edit data/log out, get data again, log out
TEST_F(GameHandlerTest, BasicGameTest) {
    auto request = construct_request(http::verb::post, "{ \"username\": \"testuser1\",  \"password\": \"testpass1\"}");
    auto response = handler->handle_request(request);
    EXPECT_EQ(response.result(), http::status::ok);
    EXPECT_EQ(response.body(), "User successfully added\n");

    request = construct_request(http::verb::get, "{ \"username\": \"testuser1\",  \"password\": \"testpass1\"}");
    response = handler->handle_request(request);
    EXPECT_EQ(response.result(), http::status::ok);
    json json_body = json::parse(response.body());
    EXPECT_EQ(json_body.at("radish_num"), 0);
    EXPECT_EQ(json_body.at("upgrades").empty(), true);

    request = construct_request(http::verb::put, "{ \"username\": \"testuser1\",  \"session_id\": \"" + (std::string)json_body.at("session_id") + "\", \"radish_num\": 7, \"upgrades\": {\"upgrade1\": 2, \"upgrade2\": 15} }");
    response = handler->handle_request(request);
    EXPECT_EQ(response.result(), http::status::ok);
    EXPECT_EQ(response.body(), "Changes successful!\n");

    request = construct_request(http::verb::get, "{ \"username\": \"testuser1\",  \"password\": \"testpass1\"}");
    response = handler->handle_request(request);
    EXPECT_EQ(response.result(), http::status::ok);
    json_body = json::parse(response.body());
    EXPECT_EQ(json_body.at("radish_num"), 7);
    EXPECT_EQ(json_body.at("upgrades").at("upgrade1"), 2);
    EXPECT_EQ(json_body.at("upgrades").at("upgrade2"), 15);

    request = construct_request(http::verb::put, "{ \"username\": \"testuser1\",  \"session_id\": \"" + (std::string)json_body.at("session_id") + "\", \"radish_num\": 7, \"upgrades\": {\"upgrade1\": 2, \"upgrade2\": 15} }");
    response = handler->handle_request(request);
    EXPECT_EQ(response.result(), http::status::ok);
    EXPECT_EQ(response.body(), "Changes successful!\n");
}

TEST_F(GameHandlerTest, DuplicateUser) {
    auto request = construct_request(http::verb::post, "{ \"username\": \"testuser1\",  \"password\": \"testpass1\"}");
    auto response = handler->handle_request(request);
    EXPECT_EQ(response.result(), http::status::ok);
    EXPECT_EQ(response.body(), "User successfully added\n");

    request = construct_request(http::verb::post, "{ \"username\": \"testuser1\",  \"password\": \"anotherpass\"}");
    response = handler->handle_request(request);
    EXPECT_EQ(response.result(), http::status::bad_request);
    EXPECT_EQ(response.body(), "Unable to add user\n");
}

// logging in to a logged in acc
TEST_F(GameHandlerTest, DoubleLogin) {
    auto request = construct_request(http::verb::post, "{ \"username\": \"testuser1\",  \"password\": \"testpass1\"}");
    auto response = handler->handle_request(request);
    EXPECT_EQ(response.result(), http::status::ok);
    EXPECT_EQ(response.body(), "User successfully added\n");

    request = construct_request(http::verb::get, "{ \"username\": \"testuser1\",  \"password\": \"testpass1\"}");
    response = handler->handle_request(request);
    EXPECT_EQ(response.result(), http::status::ok);
    json json_body = json::parse(response.body());
    EXPECT_EQ(json_body.at("radish_num"), 0);
    EXPECT_EQ(json_body.at("upgrades").empty(), true);

    request = construct_request(http::verb::get, "{ \"username\": \"testuser1\",  \"password\": \"testpass1\"}");
    response = handler->handle_request(request);
    EXPECT_EQ(response.result(), http::status::bad_request);
    EXPECT_EQ(response.body(), "Unable to find offline user and/or username/password mismatch\n");
}

// logging in to an user that DNE
TEST_F(GameHandlerTest, NonexistentLogin) {
    auto request = construct_request(http::verb::get, "{ \"username\": \"USERDNE\",  \"password\": \"testpass1\"}");
    auto response = handler->handle_request(request);
    EXPECT_EQ(response.result(), http::status::bad_request);
    EXPECT_EQ(response.body(), "Unable to find offline user and/or username/password mismatch\n");
}

// logging out with the wrong session ID
TEST_F(GameHandlerTest, WrongSessionLogout) {
    auto request = construct_request(http::verb::post, "{ \"username\": \"testuser1\",  \"password\": \"testpass1\"}");
    auto response = handler->handle_request(request);
    EXPECT_EQ(response.result(), http::status::ok);
    EXPECT_EQ(response.body(), "User successfully added\n");

    request = construct_request(http::verb::put, "{ \"username\": \"testuser1\",  \"session_id\": \"WRONGSESSIONID\", \"radish_num\": 7, \"upgrades\": {\"upgrade1\": 2, \"upgrade2\": 15} }");
    response = handler->handle_request(request);
    EXPECT_EQ(response.result(), http::status::bad_request);
    EXPECT_EQ(response.body(), "Update failed\n");
}

// logging out from a nonexistent user
TEST_F(GameHandlerTest, NonexistentLogout) {
    auto request = construct_request(http::verb::put, "{ \"username\": \"DNEUser\",  \"session_id\": \"xxx\", \"radish_num\": 7, \"upgrades\": {\"upgrade1\": 2, \"upgrade2\": 15} }");
    auto response = handler->handle_request(request);
    EXPECT_EQ(response.result(), http::status::bad_request);
    EXPECT_EQ(response.body(), "Update failed\n");
}

// logging out from an account that isn't online
TEST_F(GameHandlerTest, NotOnlineLogin) {
    auto request = construct_request(http::verb::post, "{ \"username\": \"testuser1\",  \"password\": \"testpass1\"}");
    auto response = handler->handle_request(request);
    EXPECT_EQ(response.result(), http::status::ok);
    EXPECT_EQ(response.body(), "User successfully added\n");

    request = construct_request(http::verb::get, "{ \"username\": \"testuser1\",  \"password\": \"testpass1\"}");
    response = handler->handle_request(request);
    EXPECT_EQ(response.result(), http::status::ok);

    json json_body = json::parse(response.body());
    request = construct_request(http::verb::put, "{ \"username\": \"testuser1\",  \"session_id\": \"" + (std::string)json_body.at("session_id") + "\", \"radish_num\": 7, \"upgrades\": {\"upgrade1\": 2, \"upgrade2\": 15} }");
    response = handler->handle_request(request);
    EXPECT_EQ(response.result(), http::status::ok);
    EXPECT_EQ(response.body(), "Changes successful!\n");

    request = construct_request(http::verb::put, "{ \"username\": \"testuser1\",  \"session_id\": \"" + (std::string)json_body.at("session_id") + "\", \"radish_num\": 7, \"upgrades\": {\"upgrade1\": 2, \"upgrade2\": 15} }");
    response = handler->handle_request(request);
    EXPECT_EQ(response.result(), http::status::bad_request);
    EXPECT_EQ(response.body(), "Update failed\n");
}

TEST_F(GameHandlerTest, InvalidJSON) {
    auto request = construct_request(http::verb::post, "{ \"username\": \"testuser1\",  \"password\": \"testpass1\"");
    auto response = handler->handle_request(request);
    EXPECT_EQ(response.result(), http::status::bad_request);
    EXPECT_EQ(response.body(), "Invalid JSON\n");

    request = construct_request(http::verb::get, "{ \"username\": \"testuser1\",  \"password\": \"testpass1\"");
    response = handler->handle_request(request);
    EXPECT_EQ(response.result(), http::status::bad_request);
    EXPECT_EQ(response.body(), "Invalid JSON\n");

    request = construct_request(http::verb::put, "{ \"username\": \"testuser1\",  \"session_id\": \"xxx\", \"radish_num\": 7, \"upgrades\": {\"upgrade1\": 2, \"upgrade2\": 15 }");
    response = handler->handle_request(request);
    EXPECT_EQ(response.result(), http::status::bad_request);
    EXPECT_EQ(response.body(), "Invalid JSON\n");
}

// e.g. instead of a string username, putting an int
TEST_F(GameHandlerTest, WrongTypes) {
    auto request = construct_request(http::verb::post, "{ \"username\": 1,  \"password\": \"testpass1\"}");
    auto response = handler->handle_request(request);
    EXPECT_EQ(response.result(), http::status::unprocessable_entity);
    EXPECT_EQ(response.body(), "Error with database\n");

    request = construct_request(http::verb::get, "{ \"username\": 1,  \"password\": \"testpass1\"}");
    response = handler->handle_request(request);
    EXPECT_EQ(response.result(), http::status::unprocessable_entity);
    EXPECT_EQ(response.body(), "Error with database\n");

    request = construct_request(http::verb::put, "{ \"username\": \"testuser1\",  \"session_id\": \"xxx\", \"radish_num\": 7, \"upgrades\": {\"upgrade1\": \"2\", \"upgrade2\": 15 }}");
    response = handler->handle_request(request);
    EXPECT_EQ(response.result(), http::status::unprocessable_entity);
    EXPECT_EQ(response.body(), "Error with database\n");
}

TEST_F(GameHandlerTest, MissingElements) {
    auto request = construct_request(http::verb::post, "{ \"password\": \"testpass1\"}");
    auto response = handler->handle_request(request);
    EXPECT_EQ(response.result(), http::status::bad_request);
    EXPECT_EQ(response.body(), "Must have username and password\n");

    request = construct_request(http::verb::get, "{ \"password\": \"testpass1\"}");
    response = handler->handle_request(request);
    EXPECT_EQ(response.result(), http::status::bad_request);
    EXPECT_EQ(response.body(), "Must have username and password\n");

    request = construct_request(http::verb::put, "{ \"username\": \"testuser1\",  \"session_id\": \"xxx\", \"radish_num\": 7 }");
    response = handler->handle_request(request);
    EXPECT_EQ(response.result(), http::status::bad_request);
    EXPECT_EQ(response.body(), "Must have username, session id, radish number, and upgrades\n");
}

TEST_F(GameHandlerTest, InvalidMethod) {
    auto request = construct_request(http::verb::patch, "{ \"password\": \"testpass1\"}");
    auto response = handler->handle_request(request);
    EXPECT_EQ(response.result(), http::status::bad_request);
    EXPECT_EQ(response.body(), "Invalid Method\n");
}

TEST_F(GameHandlerTest, NonexistentDB) {
    request_handler_data.data_path = ":memory:"; // new transient DB in memory - not shared
    GameRequestHandler* new_handler = new GameRequestHandler(request_handler_data);
    auto request = construct_request(http::verb::post, "{ \"username\": \"testuser1\", \"password\": \"testpass1\"}");
    auto response = new_handler->handle_request(request);
    EXPECT_EQ(response.result(), http::status::bad_request);
    EXPECT_EQ(response.body(), "Unable to add user\n");
    delete new_handler;
}