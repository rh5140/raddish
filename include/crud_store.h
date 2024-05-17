#ifndef CRUD_STORE
#define CRUD_STORE

#include <optional>
#include <vector>
#include <string>

// IMPORTANT(!)
// Note to Drew: you implement this guy for the real CRUDStore
// the fake one will inherit this one, so it can be used in it's place ;)
// I will add an implementation file for now just to allow this to exist

// these methods do not throw exceptions
// they signal errors through return values
class CRUDStore {
public:
    CRUDStore(std::string data_path) : data_path_(data_path) { }
public:
    // return id of new entity or -1 if error
    int create(std::string entity_type, std::string post_body) { return -1; }
    // reads relevant entity's id's value tnd gives back or std::nullopt if none
    std::optional<std::string> retrieve(std::string entity_type, int id) { return std::nullopt; }
    // writes the put_body to the entity's id's value and gives indicator of success
    bool update(std::string entity_type, int id, std::string put_body) { return false; }
    // deletes the entity's id and associated value and indicates if successful
    // if file non-exist, returns 'false'
    bool del(std::string entity_type, int id) { return false; }
    // return vector of the ids for the given entity
    // empty if none
    std::vector<int> list(std::string entity_type) { return {}; }
private:
    std::string data_path_;
};

#endif
