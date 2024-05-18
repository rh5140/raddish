#ifndef CRUD_STORE
#define CRUD_STORE

#include <optional>
#include <vector>
#include <string>

// these methods do not throw exceptions
// they signal errors through return values
class CRUDStore {
public:
    CRUDStore(std::string data_path);
public:
    // return id of new entity or -1 if error
    int create(std::string entity_type, std::string post_body);
    // reads relevant entity's id's value tnd gives back or std::nullopt if none
    std::optional<std::string> retrieve(std::string entity_type, int id);
    // writes the put_body to the entity's id's value and gives indicator of success
    bool update(std::string entity_type, int id, std::string put_body);
    // deletes the entity's id and associated value and indicates if successful
    // if file non-exist, returns 'false'
    bool del(std::string entity_type, int id);
    // return vector of the ids for the given entity
    // empty if none
    std::vector<int> list(std::string entity_type);
private:
    std::string data_path_;
};

#endif
