#include "gtest/gtest.h"
#include "info.h"
#include "request_handler.h"
#include "crud_store.h"
#include "crud_request_handler.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <optional>

// this implements all CRUDStore operations in an in-memory map
// in addition, it provides a facility to falsely cause a creation error
// in order to exercise all possible execution paths
class FakeCRUDStore : public CRUDStore {
public:
  FakeCRUDStore() : CRUDStore({}) { }
  int create(std::string entity, std::string content) override {
    if (create_failure)
      return -1;
    entities.insert({entity, {}});
    auto ids = entities.at(entity);
    ids[++id] = content;
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
    try {
      entities.at(entity).at(id) = content;
    } catch (const std::out_of_range&) {
      return false;
    }
    return true;
  }
  bool del(std::string entity, int id) override {
    try {
      return entities.at(entity).erase(id);
    } catch (const std::out_of_range&) {
      return false;
    }
  }
  void cause_create_failure() {
    create_failure = true;
  }
private:
  int id = 0;
  bool create_failure = false;
  std::unordered_map<std::string, std::unordered_map<int, std::string>> entities;
};

// we wanna give request handler data
// this is a location_path, root, data_path, and addr_info
// we follow the style from the filerequesttest to set this up
class CRUDHandlerTest : public testing::Test {
protected:
  std::unique_ptr<RequestHandler> request_handler;
  // non-owning ptr
  FakeCRUDStore* store;
  void SetUp() override {
    RequestHandlerData request_handler_data;
    // we will prefix requests with location_path    
    request_handler_data.location_path = "/api/";
    request_handler_data.addr_info = { "host:8080", "client:8080" };

    // keep around a reference to the store to be able to impose create failure
    auto store = std::make_unique<FakeCRUDStore>();
    this->store = store.get();
    request_handler = std::make_unique<CRUDRequestHandler>(request_handler_data,
                                                           std::move(store));
  }
};



