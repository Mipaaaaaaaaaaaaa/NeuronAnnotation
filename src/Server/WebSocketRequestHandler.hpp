#pragma once
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Data/AnnotationDS.hpp>

class WebSocketRequestHandler : public Poco::Net::HTTPRequestHandler {
public:
  int user_id;
  NeuronPool *neuron_pool;
  void handleRequest(Poco::Net::HTTPServerRequest &request,
                     Poco::Net::HTTPServerResponse &response) override;
};
