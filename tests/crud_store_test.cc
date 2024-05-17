#include "crud_store.h"
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

class CRUDStoreTest : public testing::Test {
protected:
    void SetUp() override {
        test_path = "./crud_store_test";
        fs::remove_all(test_path);
        fs::create_directory(test_path);
    }

    void TearDown() override {
        fs::remove_all(test_path);
    }

    std::string test_path;
};

TEST_F(CRUDStoreTest, FullSimpleCRUD) {
    CRUDStore store(test_path);

    std::string entity = "test_entity";
    std::string content = R"({"name": "test"})";
    int id = store.create(entity, content);
    ASSERT_NE(id, -1);

    auto retrieved = store.retrieve(entity, id);
    ASSERT_TRUE(retrieved.has_value());
    EXPECT_EQ(retrieved.value(), content);

    std::string new_content = R"({"name": "updated_test"})";
    bool updated = store.update(entity, id, new_content);
    ASSERT_TRUE(updated);

    retrieved = store.retrieve(entity, id);
    ASSERT_TRUE(retrieved.has_value());
    EXPECT_EQ(retrieved.value(), new_content);

    bool deleted = store.del(entity, id);
    ASSERT_TRUE(deleted);

    retrieved = store.retrieve(entity, id);
    EXPECT_FALSE(retrieved.has_value());
}

TEST_F(CRUDStoreTest, ListEntitiesTest) {
    CRUDStore store(test_path);

    std::string entity = "test_entity";
    store.create(entity, R"({"name": "test1"})");
    store.create(entity, R"({"name": "test2"})");

    auto ids = store.list(entity);
    EXPECT_EQ(ids.size(), 2);
    EXPECT_EQ(ids[0], 1);
    EXPECT_EQ(ids[1], 2);
}

TEST_F(CRUDStoreTest, NonExistentFile) {
    CRUDStore store(test_path);
    
    // check false on deleting bad entity
    bool deleted = store.del("test_entity", 1);
    EXPECT_FALSE(deleted);

    // check non-existent file on update
    int id = store.create("test_entity", R"({"name": "test2"})");
    ASSERT_NE(id, -1);
    ASSERT_TRUE(store.del("test_entity", id));
    bool updated = store.update("test_entity", id, R"({"name": "updated_test"})");
    EXPECT_FALSE(updated);
}

TEST_F(CRUDStoreTest, ListErrors) {
    CRUDStore store(test_path);

    // catch for bad entity names
    std::vector<int> ids = store.list("bad_entity_name");
    EXPECT_TRUE(ids.empty());

    // catch for bad file name under our entity, should be ignored by list
    fs::create_directory(test_path + "/test_entity");
     std::ofstream bad_file(test_path + "/test_entity/not_an_id");
    bad_file.close();
    ids = store.list("test_entity");
    EXPECT_TRUE(ids.empty());
}

TEST_F(CRUDStoreTest, ComplexCRUD) {
    CRUDStore store(test_path);

    std::string entity = "test_entity";
    std::string content = R"({"name": "test"})";
    int id = store.create(entity, content);
    ASSERT_EQ(id, 1);
    id = store.create(entity, content);
    ASSERT_EQ(id, 2);
    id = store.create(entity, content);
    ASSERT_EQ(id, 3);

    store.del(entity, 2);
    id = store.create(entity, content);
    ASSERT_EQ(id, 4);

    auto ids = store.list(entity);
    EXPECT_EQ(ids.size(), 3);
    EXPECT_EQ(ids[0], 1);
    EXPECT_EQ(ids[1], 3);
    EXPECT_EQ(ids[2], 4);

    store.del(entity, 3);
    store.del(entity, 4);
    id = store.create(entity, content);
    ASSERT_EQ(id, 2);
}