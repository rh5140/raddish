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
    log_request(request, res_, "Path not configured");
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

