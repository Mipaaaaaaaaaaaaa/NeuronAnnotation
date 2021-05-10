#pragma once
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <AnnotationDS.hpp>
#include <VolumeRenderer.hpp>
#include <Poco/Mutex.h>
class WebSocketRequestHandler : public Poco::Net::HTTPRequestHandler {
public:
  int user_id;
  NeuronPool *neuron_pool;
  VolumeRenderer *block_volume_renderer;
  Poco::Mutex *volume_render_lock;
  void handleRequest(Poco::Net::HTTPServerRequest &request,
                     Poco::Net::HTTPServerResponse &response) override;
};
