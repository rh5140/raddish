#include <boost/log/trivial.hpp>
#include <string>
#include <map>
#include "request_handler_factory.h"
#include "info.h"

//this prevents the static initialization issue where the order isn't fixed
//creates map on first call, after that just returns the same map as from before.
std::map<std::string, CreateRequestHandler>& RequestHandlerFactory::get_map(){
    static std::map<std::string, CreateRequestHandler>* map = new std::map<std::string,  CreateRequestHandler>;
    return *map;
}

//called by each handler to assign themselves
bool RequestHandlerFactory::register_handler(const std::string name, CreateRequestHandler factory) {
    BOOST_LOG_TRIVIAL(debug) << "Handler created : " << name;
    get_map()[name] = factory;
    return true; //can do error checking if needed
}

//given the name of a handler, returns that factory.
//defaults to a random factory if the name doesn't exist to prevent crashes.
CreateRequestHandler RequestHandlerFactory::get_factory(std::string name){
    if(get_map().count(name) == 0){
        BOOST_LOG_TRIVIAL(fatal) << "FATAL - NO HANDLER FOUND FOR KEY " + name + ". Check if the static init bool is set up properly or fix config.";
        exit(-1);
    }
    return get_map()[name];
}


