#include "MyHTTPRequestHandler.hpp"
#include <iostream>

void MyHTTPRequestHandler::handleRequest(
    Poco::Net::HTTPServerRequest &request,
    Poco::Net::HTTPServerResponse &response) {
  

    response.setContentType("application/octet-stream");
    std::ostream &ostr = response.send();
}
