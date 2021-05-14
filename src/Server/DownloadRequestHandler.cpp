#include "DownloadRequestHandler.hpp"
#include "WebSocketRequestHandler.hpp"
#include <Camera.hpp>
#include <Image.hpp>
#include <TransferFunction.hpp>
#include <VolumeRenderer.hpp>
#include <Poco/Net/NetException.h>
#include <Poco/Net/WebSocket.h>
#include <Poco/Exception.h>
#include <Poco/Util/Application.h>
#include <seria/deserialize.hpp>
#include <iostream>
#include <AnnotationDS.hpp>
#include <ErrorMessage.hpp>
#include <SWCP.hpp>
#include <DataBase.hpp>
#include <iostream>
using Poco::Util::Application;
using WebSocket = Poco::Net::WebSocket;

void DownloadRequestHandler::handleRequest(
    Poco::Net::HTTPServerRequest &request,
    Poco::Net::HTTPServerResponse &response) {
  
    Application &app = Application::instance();
    
    try
    {
        char buffer[4096];
        int flags = 0;
        int len;
        WebSocket ws(request,response);
        auto one_hour = Poco::Timespan(0, 1, 0, 0, 0);
        ws.setReceiveTimeout(one_hour);
        rapidjson::Document document{};

        do{
            len=ws.receiveFrame(buffer,sizeof(buffer),flags);
            try
            {   
                std::cout << std::string(buffer,len) << std::endl;
                std::istream &i = request.stream();

                //std::ostream &responseStream = response.send();
                std::shared_ptr<NeuronGraph> pG = neuron_pool->getGraph();
                std::string output = DataBase::getSWCFileStringFromTable(pG->tableName);
                std::string filePath = "./";
                std::string fileName = "ccecece";//pG->tableName;
                filePath.append(fileName);
                fileName.append(".swc");
                filePath.append(".swc");
                std::ofstream o(filePath);
                o.write(output.c_str(),output.size());
                o.close();
                response.set("Content-Disposition","attachment");
                response.set("filename",fileName);
                response.sendFile(filePath,"text");
                //ws.sendFrame(output.c_str(),output.size(),WebSocket::FRAME_BINARY);
            }
            catch (std::exception& error)
            {
                ws.sendFrame(error.what(), std::strlen(error.what()),WebSocket::FRAME_TEXT);
            }
            catch (Poco::FileNotFoundException& error){
                std::cout << "坏坏" << std::endl;
            }
            catch (Poco::OpenFileException& error){
                std::cout << "sad" << std::endl;
            }
        }while( len > 0 && (flags & WebSocket::FRAME_OP_BITMASK) !=WebSocket::FRAME_OP_CLOSE);
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
