
#include "request_handler_factory.h"
#include "info.h"
#include <cstdint>
#include <sstream>
#include <string>
#include <iostream>

#include <boost/log/trivial.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/log/trivial.hpp>
#include <sys/types.h>


namespace beast = boost::beast;  
namespace http = beast::http;   

using CreateRequestHandler = RequestHandler*(*)(const RequestHandlerData&);

//std::map<std::string,  RequestHandler*(*)(http::request<http::string_body>, RequestHandlerData )> RequestHandlerFactory::map_ = {};

std::map<std::string, CreateRequestHandler>& RequestHandlerFactory::get_map(){
    static std::map<std::string, CreateRequestHandler>* map = new std::map<std::string,  CreateRequestHandler>;
    return *map;
}


bool RequestHandlerFactory::register_handler(const std::string name, CreateRequestHandler factory) {
    BOOST_LOG_TRIVIAL(debug) << "handler name: " << name;
    get_map()[name] = factory;
    return true; //can do error checking if needed
}

CreateRequestHandler RequestHandlerFactory::get_factory(std::string name){
    if(get_map().count(name) == 0){
        BOOST_LOG_TRIVIAL(warning) << "No handler found for key - defaulting to random one";
        return get_map().begin()->second; //just throws out the first one in the list.
    }
    return get_map()[name];
}


/*
RequestHandler* RequestHandlerFactory::create_request_handler(std::string name, http::request<http::string_body> request, RequestHandlerData request_handler_data){
    BOOST_LOG_TRIVIAL(debug) << "handler name: " << name;
    BOOST_LOG_TRIVIAL(debug) << get_map().size();
    if(get_map().count(name) == 0){
        BOOST_LOG_TRIVIAL(warning) << "No handler found for key - defaulting to random one";
        return get_map().begin()->second(request, request_handler_data); //just throws out the first one in the list.
    }

    return get_map()[name](request, request_handler_data);
}
*/
