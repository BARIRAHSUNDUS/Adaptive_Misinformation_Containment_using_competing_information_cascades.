#ifndef WEBSERVER_H
#define WEBSERVER_H
#include <map>
#include <ostream>
#include <string>
#include "Config.h"

// A tiny local HTTP server (127.0.0.1 only) that serves the HTML GUI and runs the SAME C++ engine
// whenever the page asks for a simulation.
//   GET /                 the HTML page (gui/viewer_template.html) with the form filled with the defaults
//   GET /api/run?users=&ticks=&budget=&graph=er|ba&seed=&bench=0|1&runs=     -> JSON (what the page replays)
struct HttpResponse {
    int status = 200;
    std::string contentType = "text/plain; charset=utf-8";
    std::string body;
};

// "a=1&b=x%20y" -> {a:"1", b:"x y"}   (percent-decoding, '+' = space)
std::map<std::string, std::string> parseQuery(const std::string& query);
// Pure function (no sockets) so it can be unit-tested: method + target ("/api/run?users=50") -> response.
HttpResponse handleHttpRequest(const std::string& method, const std::string& target, const SimulationConfig& defaults);
std::string formatHttpResponse(const HttpResponse& r);        // status line + headers + body
// Binds to the first free port starting at `port`, optionally opens the browser, then serves until Ctrl+C.
// Returns 0 on a clean stop, 1 if no port could be opened.
int serveForever(const SimulationConfig& defaults, int port, bool openBrowser, std::ostream& log);
#endif