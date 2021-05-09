#include "RequestHandlerFactory.hpp"
#include "MyHTTPRequestHandler.hpp"
#include "WebSocketRequestHandler.hpp"
#include <iostream>

int RequestHandlerFactory::max_linked_id = 0;
NeuronGraph* RequestHandlerFactory::neuronGraph= new NeuronGraph("./test.swc");

Poco::Net::HTTPRequestHandler *RequestHandlerFactory::createRequestHandler(
        const Poco::Net::HTTPServerRequest &request) {
    auto &uri = request.getURI();
    if (uri == "/render") {
        std::cout << "create WebSocketRequestHandler" << std::endl;
        WebSocketRequestHandler *n = new WebSocketRequestHandler();
        n->user_id = ++max_linked_id;
        n->neuron_pool->setGraph(neuronGraph);
        n->neuron_pool->setUserId(n->user_id);
        return n;
    }
    std::cout << "create MyHTTPRequestHandler" << std::endl;
    return new MyHTTPRequestHandler();
}
