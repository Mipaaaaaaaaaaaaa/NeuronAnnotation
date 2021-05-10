#include "RequestHandlerFactory.hpp"
#include "MyHTTPRequestHandler.hpp"
#include "WebSocketRequestHandler.hpp"
#include <AnnotationDS.hpp>
#include <SWCP.hpp>
#include <iostream>

int RequestHandlerFactory::max_linked_id = 0;
NeuronGraph* RequestHandlerFactory::neuronGraph= new NeuronGraph("./test.swc");
map<int,NeuronPool*> RequestHandlerFactory::neuronPools;
map<string,int> RequestHandlerFactory::userList;

Poco::Net::HTTPRequestHandler *RequestHandlerFactory::createRequestHandler(
        const Poco::Net::HTTPServerRequest &request) {
    auto &uri = request.getURI();
    auto address =  request.clientAddress();
    auto host = address.host();
    if (uri == "/render") {
        std::cout << "create WebSocketRequestHandler" << std::endl;
        WebSocketRequestHandler *n = new WebSocketRequestHandler();
        if( userList.find(host.toString()) != userList.end() ){
            n->user_id = userList[host.toString()];
            n->neuron_pool = neuronPools[n->user_id];
        }else{
            n->user_id = ++max_linked_id;
            userList[host.toString()] = n->user_id;
            n->neuron_pool = new NeuronPool();
            n->neuron_pool->setGraph(neuronGraph);
            n->neuron_pool->setUserId(n->user_id);
            neuronPools[n->user_id] = n->neuron_pool;
        }
        return n;
    }
    std::cout << "create MyHTTPRequestHandler" << std::endl;
    return new MyHTTPRequestHandler();
}
