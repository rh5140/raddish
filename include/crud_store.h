#ifndef CRUD_STORE
#define CRUD_STORE

#include <optional>
#include <vector>
#include <string>

// these methods do not throw exceptions
// they signal errors through return values

// PRECONDITION(!): the 'id' parameter must be non-negative
// internally, we do not check, but note -1 is used as a sentinel in 'create'
class CRUDStore {
public:
    CRUDStore(std::string data_path);
public:
    // return id of new entity or -1 if error
    virtual int create(std::string entity_type, std::string post_body);
    // reads relevant entity's id's value and gives back or std::nullopt if none
    virtual std::optional<std::string> retrieve(std::string entity_type, int id);
    // writes the put_body to the entity's id's value and gives indicator of success
    // if the entity or the id doesn't exist, this will fail
    virtual bool update(std::string entity_type, int id, std::string put_body);
    // deletes the entity's id and associated value and indicates if successful
    // if file non-exist, returns 'false'
    virtual bool del(std::string entity_type, int id);
    // return vector of the ids for the given entity
    // empty if none
    virtual std::vector<int> list(std::string entity_type);
private:
    std::string data_path_;
};

#endif
