#include "GuiExport.h"
#include "Benchmark.h"
#include "SimulationEngine.h"
#include "Strategies.h"
#include "ViewerTemplate.h"
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <memory>
#include <sstream>

namespace {
// JSON string literal. '/' is escaped too, so the text can never contain "</script>".
std::string jsonString(const std::string& s) {
    std::string o = "\"";
    for (unsigned char c : s) {
        switch (c) {
            case '"': o += "\\\""; break;
            case '\\': o += "\\\\"; break;
            case '/': o += "\\/"; break;
            case '\n': o += "\\n"; break;
            case '\r': o += "\\r"; break;
            case '\t': o += "\\t"; break;
            default:
                if (c < 0x20) { char b[8]; std::snprintf(b, sizeof b, "\\u%04x", c); o += b; }
                else o += (char)c;
        }
    }
    return o + "\"";
}
std::string number(double v) { std::ostringstream o; o << std::setprecision(6) << v; return o.str(); }

void writeTransmissions(std::ostream& o, const std::vector<Transmission>& v) {
    o << "[";
    for (size_t i = 0; i < v.size(); i++) o << (i ? "," : "") << "[" << v[i].user << "," << v[i].from << "]";
    o << "]";
}

void writeTrace(std::ostream& o, const RunTrace& tr) {
    o << "[";
    for (size_t t = 0; t < tr.ticks.size(); t++) {
        const TickTrace& tt = tr.ticks[t];
        o << (t ? "," : "") << "{\"mis\":"; writeTransmissions(o, tt.misinformed);
        o << ",\"fact\":"; writeTransmissions(o, tt.corrected);
        o << ",\"picks\":[";
        for (size_t i = 0; i < tt.interventions.size(); i++) {
            const InterventionTrace& it = tt.interventions[i];
            o << (i ? "," : "") << "{\"u\":" << it.user << ",\"c\":" << it.candidates << ",\"s\":[";
            for (size_t j = 0; j < it.top.size(); j++) o << (j ? "," : "") << "[" << it.top[j].first << "," << number(it.top[j].second) << "]";
            o << "]}";
        }
        o << "],\"skipped\":" << (tt.skipped ? "true" : "false") << "}";
    }
    o << "]";
}
}

std::string buildViewerJson(const SimulationConfig& cfg, const std::string& benchmarkText) {
    // One scenario, drawn exactly like a single run: probabilities, graph and source come from cfg.seed.
    SimulationConfig ec = randomizedConfig(cfg, cfg.seed);
    std::mt19937 rng((uint32_t)cfg.seed);
    SocialGraph g = buildGraph(ec, rng);
    int source = chooseSource(ec, g.numUsers(), rng);

    RandomStrategy randomStrategy;
    HighestDegreeStrategy degreeStrategy;
    MarginalGainStrategy marginalStrategy(ec.mcTrials);
    InterventionStrategy* strategies[4] = {nullptr, &randomStrategy, &degreeStrategy, &marginalStrategy};
    const char* names[4] = {"No Intervention", "Random", "Highest Degree", "Marginal Gain"};

    std::ostringstream o;
    o << "{\"meta\":{\"users\":" << g.numUsers() << ",\"ticks\":" << ec.ticks << ",\"warmup\":" << ec.warmup
      << ",\"budget\":" << ec.budget << ",\"seed\":" << ec.seed << ",\"source\":" << source
      << ",\"graph\":" << jsonString(ec.graph == GraphType::ScaleFree ? "scale-free" : "random")
      << ",\"misP\":" << number(ec.misProb) << ",\"factP\":" << number(ec.factProb) << ",\"edgeP\":" << number(ec.edgeProb)
      << "},\"edges\":[";
    bool first = true;
    for (int u = 0; u < g.numUsers(); u++)
        for (int v : g.getNeighbours(u))
            if (u < v) { o << (first ? "" : ",") << "[" << u << "," << v << "]"; first = false; }
    o << "],\"runs\":[";
    for (int s = 0; s < 4; s++) {
        SimulationEngine engine(g, ec, strategies[s]);
        RunTrace trace;
        SimulationResult r = engine.run(source, cfg.seed, nullptr, &trace);
        o << (s ? "," : "") << "{\"name\":" << jsonString(names[s]) << ",\"summary\":{\"reach\":" << r.misinformationReach
          << ",\"finalMis\":" << r.finalMisinformed << ",\"fact\":" << r.factCheckReach << ",\"direct\":" << r.directCorrections
          << ",\"propagated\":" << r.propagatedCorrections << ",\"used\":" << r.interventionsUsed << "},\"ticks\":";
        writeTrace(o, trace);
        o << "}";
    }
    o << "],\"bench\":" << jsonString(benchmarkText) << "}";
    return o.str();
}

std::string viewerHtmlForServer(const SimulationConfig& d) {
    std::string html = viewerTemplate();
    const std::string marker = "/*__DEFAULTS__*/null";
    size_t at = html.find(marker);
    if (at == std::string::npos) return html;
    std::ostringstream o;
    o << "{\"users\":" << d.users << ",\"ticks\":" << d.ticks << ",\"budget\":" << d.budget
      << ",\"graph\":" << (d.graph == GraphType::ScaleFree ? "\"ba\"" : "\"er\"") << ",\"seed\":" << d.seed << ",\"runs\":" << d.runs << ",\"warmup\":" << d.warmup << "}";
    html.replace(at, marker.size(), o.str());
    return html;
}

bool openInBrowser(const std::string& path) {
#ifdef _WIN32
    std::string cmd = "start \"\" \"" + path + "\"";
#elif defined(__APPLE__)
    std::string cmd = "open \"" + path + "\" >/dev/null 2>&1";
#else
    std::string cmd = "xdg-open \"" + path + "\" >/dev/null 2>&1";
#endif
    return std::system(cmd.c_str()) == 0;
}