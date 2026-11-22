#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "Config.h"
#include "Benchmark.h"
#include "WebServer.h"

namespace {
// Asks one question. Enter = default value, wrong input = ask again.
template <class T> T ask(const std::string& label, T def) {
    while (true) {
        std::cout << label << " [" << def << "]: ";
        std::string line;
        if (!std::getline(std::cin, line) || line.empty()) return def;
        std::istringstream ss(line);
        T value;
        if (ss >> value) return value;
        std::cout << "  Invalid input, try again.\n";
    }
}

int strategyFromName(const std::string& s) {
    if (s == "none") return 0;
    if (s == "random") return 1;
    if (s == "degree") return 2;
    if (s == "marginal") return 3;
    return -1;
}

void usage() {
    std::cout << "Usage: ./misinfo [--users N --ticks T --budget B --strategy none|random|degree|marginal]\n"
                 "       ./misinfo --benchmark | --sweep-budget [--budgets 0,1,2,3,5,10] | --conditions\n"
                 "       ./misinfo --timing [--delays 0,1,2,3,5,8]    (WHEN should the moderator start? the main question)\n"
                 "       ./misinfo --gui [--port 8080] [--no-open]     (HTML GUI in your browser; the C++ engine runs behind it)\n"
                 "Network:  --graph er|ba   --ba-m M (links per new user, scale-free only)\n"
                 "Advanced: --warmup W --source S --seed X --mc M --runs R\n"
                 "          --edgeprob P --mis P --fact P   (fix a probability; otherwise it is random per run)\n"
                 "No arguments -> interactive mode (asks only mode, users, ticks, budget, strategy).\n";
}

// "0,2,5,10" -> {0,2,5,10}. Returns false for an empty list or a bad number.
bool parseBudgets(const std::string& text, std::vector<int>& out) {
    out.clear();
    std::istringstream ss(text);
    std::string part;
    while (std::getline(ss, part, ',')) {
        try { size_t used = 0; int b = std::stoi(part, &used); if (used != part.size() || b < 0) return false; out.push_back(b); }
        catch (const std::exception&) { return false; }
    }
    return !out.empty();
}

// Basic sanity checks. Prints the problem and returns false if the settings make no sense.
bool configIsValid(const SimulationConfig& c) {
    if (c.users < 1)                              std::cerr << "Error: users must be at least 1\n";
    else if (c.edgeProb < 0 || c.edgeProb > 1)    std::cerr << "Error: connection probability must be between 0 and 1\n";
    else if (c.misProb < 0 || c.misProb > 1)      std::cerr << "Error: misinformation probability must be between 0 and 1\n";
    else if (c.factProb < 0 || c.factProb > 1)    std::cerr << "Error: fact-check probability must be between 0 and 1\n";
    else if (c.ticks < 1)                         std::cerr << "Error: ticks must be at least 1\n";
    else if (c.warmup < 0 || c.budget < 0)        std::cerr << "Error: warmup and budget cannot be negative\n";
    else if (c.source < -1 || c.source >= c.users) std::cerr << "Error: source must be -1 or a valid user id\n";
    else if (c.mcTrials < 1 || c.runs < 1)        std::cerr << "Error: mc and runs must be at least 1\n";
    else if (c.baM < 1)                           std::cerr << "Error: ba-m must be at least 1\n";
    else return true;
    return false;
}
}

int main(int argc, char** argv) {
    SimulationConfig cfg;
    int strategy = 3;               // 0 none, 1 random, 2 degree, 3 marginal
    bool benchmark = false, sweep = false, conditions = false, timing = false, gui = false, openBrowser = true;
    int port = 8080;
    std::vector<int> delays = {0, 1, 2, 3, 5, 8};        // moderator first acts at tick delay+1
    std::vector<int> budgets = {0, 1, 2, 3, 5, 10};      // small budgets first: a budget is only interesting while it is used up

    if (argc > 1) {
        try {
            for (int i = 1; i < argc; i++) {
                std::string key = argv[i];
                if (key == "--benchmark") { benchmark = true; continue; }
                if (key == "--sweep-budget") { sweep = true; continue; }
                if (key == "--conditions") { conditions = true; continue; }
                if (key == "--timing") { timing = true; continue; }
                if (key == "--gui") { gui = true; continue; }
                if (key == "--no-open") { openBrowser = false; continue; }
                if (key == "--help") { usage(); return 0; }
                if (i + 1 >= argc) { usage(); return 1; }
                std::string value = argv[++i];
                if (key == "--users") cfg.users = std::stoi(value);
                else if (key == "--edgeprob") { cfg.edgeProb = std::stod(value); cfg.randEdge = false; }
                else if (key == "--mis") { cfg.misProb = std::stod(value); cfg.randMis = false; }
                else if (key == "--fact") { cfg.factProb = std::stod(value); cfg.randFact = false; }
                else if (key == "--ticks") cfg.ticks = std::stoi(value);
                else if (key == "--warmup") cfg.warmup = std::stoi(value);
                else if (key == "--budget") cfg.budget = std::stoi(value);
                else if (key == "--source") cfg.source = std::stoi(value);
                else if (key == "--seed") cfg.seed = std::stoull(value);
                else if (key == "--mc") cfg.mcTrials = std::stoi(value);
                else if (key == "--runs") cfg.runs = std::stoi(value);
                else if (key == "--port") port = std::stoi(value);
                else if (key == "--ba-m") cfg.baM = std::stoi(value);
                else if (key == "--graph") {
                    if (value == "er") cfg.graph = GraphType::ErdosRenyi;
                    else if (value == "ba") cfg.graph = GraphType::ScaleFree;
                    else { usage(); return 1; }
                }
                else if (key == "--delays") { if (!parseBudgets(value, delays)) { std::cerr << "Error: --delays needs a list like 0,1,2,5\n"; return 1; } }
                else if (key == "--budgets") { if (!parseBudgets(value, budgets)) { std::cerr << "Error: --budgets needs a list like 0,2,5,10\n"; return 1; } }
                else if (key == "--strategy") { strategy = strategyFromName(value); if (strategy < 0) { usage(); return 1; } }
                else { usage(); return 1; }
            }
        } catch (const std::exception&) {          // stoi/stod failed: the value was not a number
            std::cerr << "Error: a numeric option got a value that is not a number\n";
            return 1;
        }
    } else {
        std::cout << "=== Misinformation vs Fact-Check Simulator ===\n(press Enter to accept the default)\n"
                     "Probabilities are random in every run. Other settings use tuned defaults (see --help).\n\n";
        int mode = ask("Mode: 1=single run, 2=compare all strategies (benchmark), 3=budget sweep, 4=HTML GUI (opens in browser)", 1);
        if (mode != 4) {                      // the GUI has its own form for these
            cfg.users = ask("Number of users", cfg.users);
            cfg.ticks = ask("Total ticks", cfg.ticks);
            if (mode == 1 || mode == 2) cfg.budget = ask("Moderator budget (users to fact-check)", cfg.budget);
        }
        if (mode == 2) benchmark = true;
        else if (mode == 3) sweep = true;
        else if (mode == 4) gui = true;
        else strategy = ask("Strategy 0=none 1=random 2=degree 3=marginal", 3);
    }

    if (!configIsValid(cfg)) return 1;
    if (strategy < 0 || strategy > 3) { std::cerr << "Error: strategy must be 0..3\n"; return 1; }

    if (gui) return serveForever(cfg, port, openBrowser, std::cout);
    else if (timing) runTimingStudy(cfg, delays, std::cout);
    else if (conditions) runConditionStudy(cfg, std::cout);
    else if (sweep) runBudgetSweep(cfg, budgets, std::cout);
    else if (benchmark) runBenchmark(cfg, std::cout);
    else runSingle(cfg, strategy, std::cout);
    return 0;
}
