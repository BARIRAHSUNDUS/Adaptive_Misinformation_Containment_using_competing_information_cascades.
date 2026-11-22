#ifndef WEBSERVER_H
#define WEBSERVER_H
#include <map>
#include <ostream>
#include <string>
#include "Config.h"

struct HttpResponse {
    int status = 200;
    std::string contentType = "text/plain; charset=utf-8";
    std::string body;
};


std::map<std::string, std::string> parseQuery(const std::string& query);

HttpResponse handleHttpRequest(const std::string& method, const std::string& target, const SimulationConfig& defaults);
std::string formatHttpResponse(const HttpResponse& r);        

int serveForever(const SimulationConfig& defaults, int port, bool openBrowser, std::ostream& log);
#endif
