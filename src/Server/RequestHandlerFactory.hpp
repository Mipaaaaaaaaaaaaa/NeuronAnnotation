#pragma once
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <AnnotationDS.hpp>
#include <VolumeRenderer.hpp>
#include <Poco/Mutex.h>

class RequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
private:
  static int max_linked_id;
  static NeuronGraph *neuronGraph;
  static map<string,int> userList;
  static map<int,NeuronPool*> neuronPools;
  static VolumeRenderer *block_volume_renderer;
  static bool isInited;
  static Poco::Mutex *volume_render_lock;
public:
  Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest &request) override;
  void initBlockVolumeRender();
};
