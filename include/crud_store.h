#ifndef CRUD_STORE
#define CRUD_STORE

#include <optional>
#include <string>
#include <vector>

// these methods do not throw exceptions
// they signal errors through return values

// PRECONDITION(!): the 'id' parameter must be non-negative
// internally, we do not check, but note -1 is used as a sentinel in 'create'
class CRUDStore {
public:
    CRUDStore(std::string data_path);
public:
    // return id of new entity or -1 if error
    // id of new entity will be 1 if no id's in system or max_id + 1
    // this will never stomp on an old id
    virtual int create(std::string entity_type, std::string post_body);
    // reads relevant entity's id's value and gives back
    // if entity or id doesn't exist, gives std::nullopt
    virtual std::optional<std::string> retrieve(std::string entity_type, int id);
    // writes the put_body to the entity's id's value and gives indicator of success
    // if doesn't exist, will create both the entity and the id
    virtual bool update(std::string entity_type, int id, std::string put_body);
    // deletes the entity's id and associated value and indicates if successful
    // this changes the max_id value in the system
    // if entity or id doesn't exist, returns false
    virtual bool del(std::string entity_type, int id);
    // return vector of the ids for the given entity
    // empty if entity does not exist or no ids for the entity
    // this vector will be sorted in ascending order
    virtual std::vector<int> list(std::string entity_type);
private:
    std::string data_path_;
};

#endif
