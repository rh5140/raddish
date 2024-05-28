# Contributor Documentation

## How the source code is laid out

### General overview
- include - folder for header files
    - most are self explanatory
    - info.h to hold various structs used throughout
- src - folder for source files
- static_files - folder containing static files that can be requested from the webserver
    - subfolders for applications, images, and text
- tests - folder for unit and integration tests
    - requests - contains partial requests, URL is filled in by the integration script
    - responses - contains corresponding responses in RegEx
    - other subfolders for files used in tests
- gcloud_server_config vs server_config - former is used during deployment (Docker image), the other is used locally
- other files/folders - self explanatory

### Source code

#### config_parser_main.cc
- main function for the config parser - used when calling it separately from server

#### config_parser.cc
- reads in a configuration file
- parses for port number (port) and paths mapped to different handlers (location blocks)
- returns struct containing parsed info
    - port number
    - mapping from location to handler (location_to_handler)
    - mapping from location to another mapping of directive to argument (location_to_directives)
        - this assumes that every directive only has one argument

#### logger.cc
- for logging purposes
- defines and creates a file sink (rotation, formatting, etc)
- creates a console sink (primarily formatting)

#### server_main.cc
- main function for starting webserver
- runs parser on provided config, and passes parsed config to server

#### server.cc
- starts a new session

#### session.cc
- listens for requests
- upon receiving data, calls the request dispatcher for response
- sends response to client

#### request_dispatcher.cc
- using given request's path, matches it to a handler using config info
- gets corresponding factory
- uses factory to create handler to handle request
- returns response

#### request_handler_factory.cc
- contains mapping of handler name to handler factories
- mapping pairs added by handlers themselves 
- returns factory for handler when provided handler name


#### request_handler.cc
- base class of request handlers
- method for initializating response
- logging method - logs both request and response 


#### echo_request_handler.cc
- sets request as body
- sets 200 OK status and content type to be text
- returns response

#### file_request_handler.cc
- checks if file path exists - if so, tries to read in file as binary and add it to body
- sets header with appropriate content type, depending on file extension
- returns response

#### not_found_request_handler.cc
- generally configured to "/" in the configuration - catches all the unconfigured paths
- sets 404 Not Found status 
- returns response

#### Configuration side note:
- configuration should match format as specified in [Assignment 6](https://www.cs130.org/assignments/6/#config-file-format)


## How to build, test, and run the code

### Local
- `mkdir build` if no build directory exists
- `cd build`
- `cmake ..`
- `make`
- `bin/webserver ../server_config` to start server locally

#### Tests (in build directory)
- `cmake .. -DCMAKE_BUILD_TYPE=Release` (`cmake ..` OK if you didn't run coverage first)
- `make`
- `make test` to run tests

#### Coverage (in build coverage directory)
- `mkdir build_coverage` if no build coverage directory exists
- `cmake .. -DCMAKE_BUILD_TYPE=Coverage`
- `make coverage`
- To see line coverage, check the `build/reports` for `index.html` and Open with Live Server (may require downloading extension)

### Docker
- `docker build -f docker/base.Dockerfile -t raddish:base .`
- `docker build -f docker/Dockerfile -t my_image .`
    - If you want to clear the build cache, `docker build --no-cache -f docker/Dockerfile -t my_image`
- `docker run --rm -p 127.0.0.1:80:80 --name my_run my_image:latest`

## How to add a request handler
- Write your request handler as a child of the base class RequestHandler
    - its constructor should call the parent constructor with the passed in request handler data
    - it should implement the handle request method
- Write the init method for the factory - which just returns a new instance of itself
    - `RequestHandler* [HandlerName]::init(const RequestHandlerData& request_handler_data) {
    return new [HandlerName](request_handler_data); 
}`
- Register the handler with the factory map
    - `bool [HandlerName]::registered_ = RequestHandlerFactory::register_handler("[HandlerName]", [HandlerName]::init);`
- Link your handler in CMakeList.txt
    - *Note that ordering of the arguments matter! If A depends on B, A should be earlier than B in the list.*
    - Add library: `add_library([lib_name] OBJECT src/[handler_file_name])`
    - Add your library to the interface (used for linked multiple handlers at once): `target_link_libraries(all_handlers_lib INTERFACE $<TARGET_OBJECTS:[lib_name]> ...)`
    - Add and link test executable: `add_executable([test_name] tests/[test_file_name])
target_link_libraries([test_name] $<TARGET_OBJECTS:[lib_name]> handler_factory_lib handler_lib gtest_main Boost::log)`
    - Add test to gtest: `gtest_discover_tests([test_name] WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests)`
    - Add source and test libraries to coverage test: `generate_coverage_report(TARGETS ... [lib_name] ... TESTS ... [test_name] ...) `


### Example Handler
```
#include "not_found_request_handler.h"
#include "request_handler_factory.h"
namespace beast = boost::beast;  
namespace http = beast::http;    

// calls parent constructor
// can be used to set handler-specific variables - e.g. the root for file handler.
// all data needed should be added to the RequestHandlerData struct contained in info.h.
NotFoundHandler::NotFoundHandler(const RequestHandlerData& request_handler_data) : RequestHandler(request_handler_data){
}

// factory function
// Defined as a static function in the header - added to registry map on startup.
RequestHandler* NotFoundHandler::init(const RequestHandlerData& request_handler_data) {
    return new NotFoundHandler(request_handler_data); 
}


// add self-init to registry on startup. 
// defined as a static bool in the header file. 
bool NotFoundHandler::registered_ = RequestHandlerFactory::register_handler("NotFoundHandler", NotFoundHandler::init);

// implementation of handle_request
http::response<http::string_body> NotFoundHandler::handle_request(const http::request<http::string_body>& request) {
    //superclass function - creates a res object, and initializes it as a 404 not found.
    init_response(request);
    //set vars
    res_.result(http::status::not_found); //technically already done in init_response, but done here for clarity. 
    //log
    log_request(request, res_, "Path not configured", "NotFoundHandler");
    return res_;
}
```
### Example Header
```
#ifndef NOT_FOUND_REQUEST_HANDLER
#define NOT_FOUND_REQUEST_HANDLER

#include "request_handler.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
    
class NotFoundHandler : public RequestHandler {
    public:
        NotFoundHandler(const RequestHandlerData& request_handler_data); //constructor
        http::response<http::string_body> handle_request(const http::request<http::string_body>& request);
        static RequestHandler* init(const RequestHandlerData& request_handler_data); //factory
        static bool registered_; //used to register
};

#endif
```

# CRUD Handler Readme

## Handler Overview

The CRUD handler provides capabilities as required by assignment 7, spec shown [here](https://www.cs130.org/assignments/7/).

One should also note before looking at the operations that follow, that we assume the CRUD handler to be setup and have a specified valid `data_path`, or directory to be used internally by the persistent key-value store as implemented by the `CRUD store`. This is enforced by the handler's construction and initialization.

Despite the name of CRUD where one would expect 4 operations, the handler supports 5 distinct operations. Additionally, we define an operation as those that follow as supported by the CRUD handler, and an op code, or `${OP}`, as one of the HTTP verbs such as GET or POST.

#### Supported Operations

* Create: `POST /api/Shoes` -> which creates a new key-value store and writes the POST body to `data_path/Shoes/${ID}`, and then returns the ID of a newly created shoe entity
* Retrieve: `GET /api/Shoes/1` -> returns the data at the key of 1 in the entity Shoes by internally reading `data_path/Shoes/1`
* Update: `PUT /api/Shoes/5` -> updates the data for shoe 5 with the PUT body, by writing to `data_path/Shoes/5`. If shoe at ID 5 already existing, update the value. If shoe at 5 has not already been created, then we create a shoe at 5 and put the PUT body there, allowing for creation of entity values with a pre-determined ID.
* Delete: `DELETE /api/Shoes/1` -> delete shoe 1 from the system by interally removing the file `data_path/Shoes/1`
* List: `GET /api/Shoes` -> returns a JSON list of valid IDs for the given entity, in this case all IDs currently in use for Shoes, internally located at `data_path/Shoes/`

#### Mapping of Op Codes to Operations
* `GET` -> retrieval of a single item if ID specified *__OR__* a list of all valid IDs if no ID specified
* `POST` -> creation of a single item using the POST body as input to the value
* `PUT` -> update of data at existing ID, or creation with data at specific non-existing ID
* `DELETE` -> deletion of item at specified ID

#### Request Format
 All requests delivered to the CRUD handler API are expected to be in the format `${OP} ${API}/${Entity}/${ID}?`, where each of the following are defined as follows:
* `${OP}` is one of the operations specified above
* `${API}` is the location of the server's CRUD handler, such as `/api/` or other defined by           server config
* `${Entity}` is any simply-defined string name of an entity type (with some limitations noted below)
* `${ID}` is the ID if of the item in question if required by the operation

## More Info & Edge Cases

Edge cases will be specified per the 5 distinct operations and bad requests in general. Bad requests at the fault of the user will receive `400 Bad Request`. If the internals of the CRUD store fail for any reason not due to request input, then we will return `500 Internal Server Error` since this is not the fault of the user.

### General Info On Bad Requests 
* `${Entity}` can be any word with upper or lowercase letters, but cannot contain any other characters. This is defined by the Regex `[A-Za-z]+` and is our interpretation of a simply-defined string word. 
* `${ID}` can be any number of digits so long as there is one if required, but this is limited internally by its implementation and storage as an `int`, so be wary of overflow which will lead to undefined behavior
* If a request does not match our required format we return `400 Bad Request` with a body of `Invalid request format for CRUD API`. For example, if an operation requires an ID but one is not provided, then that is a `400`. Vice-versa, if an ID should not be specified and one is specified, that is a `400`.

### Create
* Uses an op code of `POST`
* IDs start at 1 and count upwards. 
* When creating a new item, the new ID will be `max value of the currently used IDs + 1`
* `POST` with an ID specified is a bad request
* `POST` with invalid JSON in the POST body is a bad request
* If the `${Entity}` subdirectory does not already exist (perhaps because there were no entries previously to this entity type), then one will be created and operation continues as normal

### Retrieve
* Uses an op code of `GET`
* If no ID is specified, we assume List operation (discussed below)
* If an invalid ID is specified, we return `404 File Not Found`
* Since IDs start at 1, `GET` with an ID of 0 or no ID will be interpreted as a `List` operation, since internally an ID of 0 (or no ID given) is used to signal a `List` operation

### Update
* Uses an op code of `PUT`
* If the ID is 0, bad request
* If the JSON in the POST body is invalid, bad request
* If the ID is in use already, update the value at that ID
* If the ID is not in use already, create a new item with that ID and the POST body as the value
* If ID not in use already and `${Entity}` also does not exist, create the `${Entity}` store and proceed as if everything is fine and ID not in use, so create a new item with that ID

### Delete
* Uses an op code of `DELETE`
* If no ID is specified or the ID specified is 0, bad request
* If the file exists at that ID, delete it
* If the file does not exist to delete, return `404 File Not Found`

### List
* Shares an op code of `GET` with retrieve
* Unspecified or ID of 0 is routed to the list functionality
* The list of valid IDs returned will be sorted in ascending order
* If there are no valid IDs, then we give back an empty list of IDs with `200`
* If the entity type does not exist, give back an empty list of IDs with `200`
* The list implementation ignores non-integer filenames and only returns those with valid IDs (integers only). These files would only end up here if the server admin interfered and put them there.


