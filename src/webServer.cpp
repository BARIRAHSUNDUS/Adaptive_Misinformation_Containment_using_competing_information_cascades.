#include "WebServer.h"
#include "Benchmark.h"
#include "GuiExport.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <sstream>

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  typedef SOCKET sock_t;
  static void closeSocket(sock_t s) { closesocket(s); }
  static const sock_t BAD_SOCKET = INVALID_SOCKET;
  #ifdef _MSC_VER
    #pragma comment(lib, "ws2_32.lib")
  #endif
#else
  #include <arpa/inet.h>
  #include <netinet/in.h>
  #include <sys/socket.h>
  #include <sys/time.h>
  #include <unistd.h>
  typedef int sock_t;
  static void closeSocket(sock_t s) { close(s); }
  static const sock_t BAD_SOCKET = -1;
#endif

namespace {
int hexValue(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}
std::string urlDecode(const std::string& s) {
    std::string o;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '+') o += ' ';
        else if (s[i] == '%' && i + 2 < s.size() && hexValue(s[i + 1]) >= 0 && hexValue(s[i + 2]) >= 0) {
            o += (char)(hexValue(s[i + 1]) * 16 + hexValue(s[i + 2])); i += 2;
        } else o += s[i];
    }
    return o;
}

HttpResponse jsonError(int status, const std::string& message) {     // `message` is always one of our own fixed texts
    HttpResponse r; r.status = status; r.contentType = "application/json; charset=utf-8";
    r.body = "{\"error\":\"" + message + "\"}";
    return r;
}

// Whole-string integer in [lo, hi]. A missing key keeps `value` unchanged.
bool intParam(const std::map<std::string, std::string>& q, const char* key, long long lo, long long hi, long long& value) {
    auto it = q.find(key);
    if (it == q.end()) return true;
    const std::string& s = it->second;
    size_t i = (!s.empty() && s[0] == '-') ? 1 : 0;
    if (s.size() == i || s.size() > 15) return false;
    for (size_t k = i; k < s.size(); k++) if (!std::isdigit((unsigned char)s[k])) return false;
    long long v = std::atoll(s.c_str());
    if (v < lo || v > hi) return false;
    value = v;
    return true;
}
}

std::map<std::string, std::string> parseQuery(const std::string& query) {
    std::map<std::string, std::string> q;
    std::istringstream in(query);
    std::string part;
    while (std::getline(in, part, '&')) {
        if (part.empty()) continue;
        size_t eq = part.find('=');
        if (eq == std::string::npos) q[urlDecode(part)] = "";
        else q[urlDecode(part.substr(0, eq))] = urlDecode(part.substr(eq + 1));
    }
    return q;
}

HttpResponse handleHttpRequest(const std::string& method, const std::string& target, const SimulationConfig& defaults) {
    if (method != "GET") return jsonError(405, "only GET is supported");
    size_t qm = target.find('?');
    std::string path = target.substr(0, qm), query = qm == std::string::npos ? "" : target.substr(qm + 1);

    if (path == "/" || path == "/index.html") {
        HttpResponse r; r.contentType = "text/html; charset=utf-8"; r.body = viewerHtmlForServer(defaults); return r;
    }
    if (path == "/favicon.ico") { HttpResponse r; r.status = 204; return r; }
    if (path != "/api/run") return jsonError(404, "not found");

    // ---- /api/run: validate every parameter, then run the real engine
    std::map<std::string, std::string> q = parseQuery(query);
    long long users = defaults.users, ticks = defaults.ticks, budget = defaults.budget, warmup = defaults.warmup, seed = (long long)defaults.seed, runs = defaults.runs, bench = 1;
    if (!intParam(q, "users", 1, 500, users))       return jsonError(400, "users must be a whole number from 1 to 500");
    if (!intParam(q, "ticks", 1, 300, ticks))       return jsonError(400, "ticks must be a whole number from 1 to 300");
    if (!intParam(q, "budget", 0, 500, budget))     return jsonError(400, "budget must be a whole number from 0 to 500");
    if (!intParam(q, "warmup", 0, 299, warmup))     return jsonError(400, "warmup must be a whole number from 0 to 299");
    if (!intParam(q, "seed", 0, 1000000000000LL, seed)) return jsonError(400, "seed must be a whole number from 0 to 1000000000000");
    if (!intParam(q, "runs", 1, 100, runs))         return jsonError(400, "runs must be a whole number from 1 to 100");
    if (!intParam(q, "bench", 0, 1, bench))         return jsonError(400, "bench must be 0 or 1");
    SimulationConfig cfg = defaults;
    cfg.users = (int)users; cfg.ticks = (int)ticks; cfg.budget = (int)budget; cfg.warmup = (int)warmup; cfg.seed = (uint64_t)seed; cfg.runs = (int)runs;
    auto g = q.find("graph");
    if (g != q.end()) {
        if (g->second == "er") cfg.graph = GraphType::ErdosRenyi;
        else if (g->second == "ba") cfg.graph = GraphType::ScaleFree;
        else return jsonError(400, "graph must be er or ba");
    }
    if (cfg.source >= cfg.users) cfg.source = -1;
    if (cfg.users > 200) { cfg.mcTrials = std::min(cfg.mcTrials, 40); cfg.runs = std::min(cfg.runs, 10); }   // keep big networks responsive

    std::string benchText = "Benchmark not run (tick \"Compare all strategies\" to see which strategy is best over many scenarios).";
    if (bench == 1) { std::ostringstream b; runBenchmark(cfg, b); benchText = b.str(); }
    HttpResponse r; r.contentType = "application/json; charset=utf-8"; r.body = buildViewerJson(cfg, benchText);
    return r;
}

std::string formatHttpResponse(const HttpResponse& r) {
    const char* reason = r.status == 200 ? "OK" : r.status == 204 ? "No Content" : r.status == 400 ? "Bad Request"
                       : r.status == 404 ? "Not Found" : r.status == 405 ? "Method Not Allowed" : "Error";
    std::ostringstream o;
    o << "HTTP/1.1 " << r.status << " " << reason << "\r\n"
      << "Content-Type: " << r.contentType << "\r\n"
      << "Content-Length: " << r.body.size() << "\r\n"
      << "Cache-Control: no-store\r\nConnection: close\r\n\r\n" << r.body;
    return o.str();
}

int serveForever(const SimulationConfig& defaults, int port, bool openBrowser, std::ostream& log) {
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { log << "Error: cannot start Winsock\n"; return 1; }
#else
    std::signal(SIGPIPE, SIG_IGN);
#endif
    sock_t listener = BAD_SOCKET;
    int boundPort = -1;
    for (int p = port; p < port + 20 && p <= 65535; p++) {
        sock_t s = socket(AF_INET, SOCK_STREAM, 0);
        if (s == BAD_SOCKET) continue;
#ifndef _WIN32
        int yes = 1; setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof yes);
#endif
        sockaddr_in addr; std::memset(&addr, 0, sizeof addr);
        addr.sin_family = AF_INET; addr.sin_port = htons((unsigned short)p);
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);              // this computer only
        if (bind(s, (sockaddr*)&addr, sizeof addr) == 0 && listen(s, 8) == 0) { listener = s; boundPort = p; break; }
        closeSocket(s);
    }
    if (listener == BAD_SOCKET) { log << "Error: could not open a local port (tried " << port << "-" << port + 19 << ")\n"; return 1; }

    std::string url = "http://localhost:" + std::to_string(boundPort) + "/";
    log << "GUI running at " << url << "\nPress Ctrl+C to stop.\n" << std::flush;
    if (openBrowser && !openInBrowser(url)) log << "Could not open the browser automatically. Open " << url << " yourself.\n";

    while (true) {
        sock_t c = accept(listener, nullptr, nullptr);
        if (c == BAD_SOCKET) continue;
#ifdef _WIN32
        DWORD ms = 5000; setsockopt(c, SOL_SOCKET, SO_RCVTIMEO, (const char*)&ms, sizeof ms);
#else
        timeval tv; tv.tv_sec = 5; tv.tv_usec = 0; setsockopt(c, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
#endif
        std::string req; char buf[2048];
        while (req.find("\r\n\r\n") == std::string::npos && req.size() < 16384) {
            int got = (int)recv(c, buf, sizeof buf, 0);
            if (got <= 0) break;
            req.append(buf, (size_t)got);
        }
        HttpResponse resp;
        std::istringstream line(req.substr(0, req.find("\r\n")));
        std::string method, target, version;
        if (line >> method >> target >> version) resp = handleHttpRequest(method, target, defaults);
        else { resp.status = 400; resp.body = "bad request"; }
        std::string out = formatHttpResponse(resp);
        size_t sent = 0;
        while (sent < out.size()) {
            int w = (int)send(c, out.data() + sent, (int)std::min<size_t>(out.size() - sent, 65536), 0);
            if (w <= 0) break;
            sent += (size_t)w;
        }
        closeSocket(c);
    }
}