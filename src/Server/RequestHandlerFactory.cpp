#include "RequestHandlerFactory.hpp"
#include "MyHTTPRequestHandler.hpp"
#include "WebSocketRequestHandler.hpp"
#include <iostream>

int RequestHandlerFactory::max_linked_id = 0;

Poco::Net::HTTPRequestHandler *RequestHandlerFactory::createRequestHandler(
        const Poco::Net::HTTPServerRequest &request) {
    auto &uri = request.getURI();
    if (uri == "/render") {
        std::cout << "create WebSocketRequestHandler" << std::endl;
        WebSocketRequestHandler *n = new WebSocketRequestHandler();
        n->user_id = ++max_linked_id;
        n->neuron_pool->graphs = graph;
        return n;
    }
    std::cout << "create MyHTTPRequestHandler" << std::endl;
    return new MyHTTPRequestHandler();
}
