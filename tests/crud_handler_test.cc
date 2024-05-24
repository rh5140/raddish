#include <gtest/gtest.h>
#include "request_handler.h"
#include "crud_store.h"
#include "crud_request_handler.h"

#include <algorithm>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <iostream>

namespace http = boost::beast::http;

// this implements all CRUDStore operations in an in-memory map
class FakeCRUDStore : public CRUDStore {
public:
  FakeCRUDStore() : CRUDStore({}) { }
  int create(std::string entity, std::string content) override {
    if (fake_error)
      return -1;
    entities.insert({entity, {}});
    auto& ids = entities.at(entity);

    auto id_list = list(entity);
    auto id = id_list.empty() ? 1 : id_list[id_list.size() - 1] + 1;
    ids[id] = content;
    return id;
  }
  std::optional<std::string> retrieve(std::string entity, int id) override {
    try {
      return entities.at(entity).at(id);
    } catch (const std::out_of_range&) {
      return std::nullopt;
    }
  }
  bool update(std::string entity, int id, std::string content) override {
    if (fake_error)
      return false;
    entities.insert({entity, {}});
    auto& ids = entities.at(entity);
    ids[id] = content;
    return true;
  }
  bool del(std::string entity, int id) override {
    try {
      return entities.at(entity).erase(id);
    } catch (const std::out_of_range&) {
      return false;
    }
  }
  virtual std::vector<int> list(std::string entity) {
    std::vector<int> l;
    try {
      auto& ids = entities.at(entity);
      for (const auto& [id, _] : ids)
        l.push_back(id);
      std::sort(l.begin(), l.end());
    } catch (const std::out_of_range&) {

    }
    return l;
  }
  void set_fake_error() {
    fake_error = true;
  }
private:
  bool fake_error = false;
  std::unordered_map<std::string, std::unordered_map<int, std::string>> entities;
};

// we wanna give request handler data
// this is a location_path, root, data_path, and addr_info
// we follow the style from the filerequesttest to set this up
class CRUDHandlerTest : public testing::Test {
protected:
  std::unique_ptr<RequestHandler> request_handler;
  RequestHandlerData request_handler_data;  
  // non-owning ptr
  FakeCRUDStore* store;
  void SetUp() override {
    // we will prefix requests with location_path    
    request_handler_data.location_path = "/api/";
    request_handler_data.addr_info = { "host:8080", "client:8080" };

    // keep around a reference to the store to be able to impose create failure
    auto store = std::make_unique<FakeCRUDStore>();
    this->store = store.get();
    request_handler = std::make_unique<CRUDRequestHandler>(request_handler_data,
                                                           std::move(store));
  }
  
  http::request<http::string_body> construct_request(http::verb verb, std::string path, std::string body) {
    http::request<http::string_body> req(verb, path, 11);
    req.set(http::field::content_type, "application/json");
    req.body() = body;
    req.prepare_payload();
    return req;
  }
};

TEST_F(CRUDHandlerTest, TestFactoryBadDataPath) {
  EXPECT_ANY_THROW(CRUDRequestHandler::init(request_handler_data));
}

TEST_F(CRUDHandlerTest, TestFactoryGoodDataPath) {
  request_handler_data.data_path = "./static_files";
  EXPECT_NO_THROW(CRUDRequestHandler::init(request_handler_data));
}

TEST_F(CRUDHandlerTest, CreateAndGetBack) {
  auto request = construct_request(http::verb::post, "/api/Shoes", "\"arteen\"");
  auto response = request_handler->handle_request(request);
  EXPECT_EQ(response.result(), http::status::ok);
  EXPECT_EQ(response.body(), "{\"id\": 1}");

  // make sure it's there
  request = construct_request(http::verb::get, "/api/Shoes/1", {});
  response = request_handler->handle_request(request);
  EXPECT_EQ(response.result(), http::status::ok);
  EXPECT_EQ(response.body(), "\"arteen\"");
}

TEST_F(CRUDHandlerTest, PutAndGetBack) {
  auto request = construct_request(http::verb::put, "/api/Shoes/78", "\"arteen\"");
  auto response = request_handler->handle_request(request);
  EXPECT_EQ(response.result(), http::status::ok);

  // make sure it's there
  request = construct_request(http::verb::get, "/api/Shoes/78", {});
  response = request_handler->handle_request(request);
  EXPECT_EQ(response.result(), http::status::ok);
  EXPECT_EQ(response.body(), "\"arteen\"");
}

TEST_F(CRUDHandlerTest, BadRequest) {
  auto request = construct_request(http::verb::put, "/api/Shoes/blah", "\"arteen\"");
  auto response = request_handler->handle_request(request);
  EXPECT_EQ(response.result(), http::status::bad_request);
}

TEST_F(CRUDHandlerTest, BadPOST) {
  auto request = construct_request(http::verb::post, "/api/Shoes/1", "\"arteen\"");
  auto response = request_handler->handle_request(request);
  EXPECT_EQ(response.result(), http::status::bad_request);
}

TEST_F(CRUDHandlerTest, BadPUT) {
  auto request = construct_request(http::verb::put, "/api/Shoes/", "\"arteen\"");
  auto response = request_handler->handle_request(request);
  EXPECT_EQ(response.result(), http::status::bad_request);
}

TEST_F(CRUDHandlerTest, BadDELETE) {
  auto request = construct_request(http::verb::delete_, "/api/Shoes/", "\"arteen\"");
  auto response = request_handler->handle_request(request);
  EXPECT_EQ(response.result(), http::status::bad_request);
}

TEST_F(CRUDHandlerTest, BadJSONPut) {
  auto request = construct_request(http::verb::put, "/api/Shoes/1", "arteen");
  auto response = request_handler->handle_request(request);
  EXPECT_EQ(response.result(), http::status::bad_request);
}

TEST_F(CRUDHandlerTest, DeleteNonExistent) {
  auto request = construct_request(http::verb::delete_, "/api/Shoes/1", "\"arteen\"");
  auto response = request_handler->handle_request(request);
  EXPECT_EQ(response.result(), http::status::not_found);
}

TEST_F(CRUDHandlerTest, BadVerb) {
  auto request = construct_request(http::verb::patch, "/api/Shoes/1", "\"arteen\"");
  auto response = request_handler->handle_request(request);
  EXPECT_EQ(response.result(), http::status::method_not_allowed);
}

TEST_F(CRUDHandlerTest, ListNothing) {
  auto request = construct_request(http::verb::get, "/api/Shoes/", {});
  auto response = request_handler->handle_request(request);
  EXPECT_EQ(response.result(), http::status::ok);
  EXPECT_EQ(response.body(), "[]");
}

TEST_F(CRUDHandlerTest, BadJSONPost) {
  auto request = construct_request(http::verb::post, "/api/Shoes/", "arteen");
  auto response = request_handler->handle_request(request);
  EXPECT_EQ(response.result(), http::status::bad_request);
}

TEST_F(CRUDHandlerTest, BadIDPost) {
  store->set_fake_error();
  auto request = construct_request(http::verb::post, "/api/Shoes/", "\"arteen\"");
  auto response = request_handler->handle_request(request);
  EXPECT_EQ(response.result(), http::status::internal_server_error);
}

TEST_F(CRUDHandlerTest, NonExistentIDGET) {
  auto request = construct_request(http::verb::get, "/api/Shoes/1", {});
  auto response = request_handler->handle_request(request);
  EXPECT_EQ(response.result(), http::status::not_found);
}

TEST_F(CRUDHandlerTest, PutFails) {
  store->set_fake_error();
  auto request = construct_request(http::verb::put, "/api/Shoes/1", "\"arteen\"");
  auto response = request_handler->handle_request(request);
  EXPECT_EQ(response.result(), http::status::internal_server_error);
}

TEST_F(CRUDHandlerTest, SuccessfulDelete) {
  auto request = construct_request(http::verb::put, "/api/Shoes/1", "\"arteen\"");
  auto response = request_handler->handle_request(request);
  EXPECT_EQ(response.result(), http::status::ok);

  request = construct_request(http::verb::delete_, "/api/Shoes/1", {});
  response = request_handler->handle_request(request);
  EXPECT_EQ(response.result(), http::status::ok);  
}
