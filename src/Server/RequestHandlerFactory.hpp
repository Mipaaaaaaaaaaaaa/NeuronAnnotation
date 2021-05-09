#pragma once
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <AnnotationDS.hpp>

class RequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
private:
  static int max_linked_id;
  static NeuronGraph *neuronGraph;
public:
  Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest &request) override;
};
