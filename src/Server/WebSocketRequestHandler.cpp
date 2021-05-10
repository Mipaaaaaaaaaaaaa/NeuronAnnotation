#include "WebSocketRequestHandler.hpp"
#include <Camera.hpp>
#include <Image.hpp>
#include <TransferFunction.hpp>
#include <VolumeRenderer.hpp>
#include <Poco/Net/NetException.h>
#include <Poco/Net/WebSocket.h>
#include <Poco/Util/Application.h>
#include <seria/deserialize.hpp>
#include <iostream>
#include <AnnotationDS.hpp>
#include <ErrorMessage.hpp>
#include <SWCP.hpp>


using Poco::Util::Application;

void WebSocketRequestHandler::handleRequest(
        Poco::Net::HTTPServerRequest &request,
        Poco::Net::HTTPServerResponse &response) {
    using WebSocket = Poco::Net::WebSocket;

    Application &app = Application::instance();
    VolumeRenderer block_volume_renderer("BlockVolumeRenderer");
    // VolumeRenderer lines_renderer("LinesRender");
    std::cout<<"loading render backend..."<<std::endl;
#ifdef _WINDOWS
    block_volume_renderer.set_volume("D:/mouse_23389_29581_10296_9p2_lod3.h264");
#else
    block_volume_renderer.set_volume("/media/wyz/Workspace/mouse_23389_29581_10296_512_2_lod3/mouse_23389_29581_10296_9p2_lod3.h264");
#endif
    TransferFunction default_tf;
    default_tf.points.emplace_back(0);
    default_tf.colors.emplace_back(std::array<double ,4>{0.0,0.1,0.6,0.0});
    default_tf.points.emplace_back(30);
    default_tf.colors.emplace_back(std::array<double ,4>{0.25, 0.5, 1.0, 0.9});
    default_tf.points.emplace_back(64);
    default_tf.colors.emplace_back(std::array<double ,4>{0.75,0.75,0.75,0.9});
    default_tf.points.emplace_back(224);
    default_tf.colors.emplace_back(std::array<double ,4>{1.0,0.5,0.25,0.9});
    default_tf.points.emplace_back(225);
    default_tf.colors.emplace_back(std::array<double ,4>{0.6,0.1,0.0,1.0});
    block_volume_renderer.set_transferfunc(default_tf);
    
    if( neuron_pool->getSelectedLineIndex() == -1 ){
        neuron_pool->initSelectedLineIndex();
    }
    if( neuron_pool->getSelectedVertexIndex() == -1 ){
        neuron_pool->initSelectedVertexIndex();
    }

    try
    {
        char buffer[4096];
        int flags = 0;
        int len;

        WebSocket ws(request, response);
        auto one_hour = Poco::Timespan(0, 1, 0, 0, 0);
        ws.setReceiveTimeout(one_hour);
        rapidjson::Document document{};

        do{
            len=ws.receiveFrame(buffer,sizeof(buffer),flags);
            try
            {   
                document.Parse(buffer,len);
                
                if(document.HasParseError() || !document.IsObject())
                {
                    throw std::runtime_error("Parse error");
                }
                auto objects=document.GetObject();
                if(document.HasMember("camera"))
                {
                    auto values=objects["camera"].GetObject();
                    Camera camera;
                    seria::deserialize(camera,values);
                    block_volume_renderer.set_camera(camera);
                    //lines_renderer.set_camera(camera);
                }
                else if(document.HasMember("click"))
                {
                    auto values=objects["click"].GetObject();
                    QueryPoint query_point;
                    seria::deserialize(query_point,values);
                    block_volume_renderer.set_querypoint({query_point.x,query_point.y});
                    block_volume_renderer.render_frame();
                    auto query_res = block_volume_renderer.get_querypoint();
                    //进一步确定当前操作
                    if ( document.HasMember("tool") ){
                        rapidjson::Value& toolValue = document["tool"];
                        std::cout << toolValue.GetInt() << std::endl;
                        switch(Tools(toolValue.GetInt())){
                            case Drag:
                            break;
                            case Insert:
                                if( query_res[7] > 0.7f )
                                    neuron_pool->addVertex(query_res[0],query_res[1],query_res[3]);
                                else{
                                    printf("%lf Alpha is too low!\n",query_res[7]);
                                    ErrorMessage em("选择点透明度低，请重新选择");
                                    std::string str = em.ToJson();
                                    ws.sendFrame(str.c_str(),str.size(),WebSocket::FRAME_TEXT);
                                }
                            break;
                            case Cut:
                            break;
                            case Select:
                            break;
                            case Delete:
                            break;
                        }
                    }
                }
                else if(document.HasMember("modify")){
                    rapidjson::Value &modify_data = document["modify"];
                    if( modify_data.HasMember("selectedVertexIndex") && modify_data["selectedVertexIndex"].IsInt64() ){
                        neuron_pool->selectVertex(modify_data["selectedVertexIndex"].GetInt64());
                    }
                    if( modify_data.HasMember("selectedLineIndex") && modify_data["selectedLineIndex"].IsInt64() ){
                        neuron_pool->selectLine(modify_data["selectedLineIndex"].GetInt64());
                    }
                    // if( neuron_pool->modifyData(&modify_data) ){

                    // }
                }
                else if(document.HasMember("addline")){
                    if (neuron_pool->addLine()){
                        ErrorMessage em("修改成功","success");
                        std::string str = em.ToJson();
                        ws.sendFrame(str.c_str(),str.size(),WebSocket::FRAME_BINARY);
                    }else{
                        ErrorMessage em("修改失败");
                        std::string str = em.ToJson();
                        ws.sendFrame(str.c_str(),str.size(),WebSocket::FRAME_TEXT);
                    }

                }
                block_volume_renderer.render_frame();
                auto &image = block_volume_renderer.get_frame();
                std::cout<<image.width<<" "<<image.height<<std::endl;
                auto encoded = Image::encode(image, Image::Format::JPEG);
                ws.sendFrame(encoded.data.data(), encoded.data.size(),WebSocket::FRAME_BINARY);
                std::string structureInfo = neuron_pool->getLinestoJson();
                ws.sendFrame(structureInfo.c_str(),structureInfo.size(),WebSocket::FRAME_TEXT);
            }
            catch (std::exception& error)
            {
                ws.sendFrame(error.what(), std::strlen(error.what()),WebSocket::FRAME_TEXT);
            }
        }while(len>0 && (flags & WebSocket::FRAME_OP_BITMASK) !=WebSocket::FRAME_OP_CLOSE);

    }//try
    catch (Poco::Net::WebSocketException &exc)
    {
        app.logger().log(exc);
        switch (exc.code())
        {
            case WebSocket::WS_ERR_HANDSHAKE_UNSUPPORTED_VERSION:
                response.set("Sec-WebSocket-Version", WebSocket::WEBSOCKET_VERSION);
                // fallthrough
            case WebSocket::WS_ERR_NO_HANDSHAKE:
            case WebSocket::WS_ERR_HANDSHAKE_NO_VERSION:
            case WebSocket::WS_ERR_HANDSHAKE_NO_KEY:
                response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
                response.setContentLength(0);
                response.send();
                break;
        }//switch
    }//catch
}
