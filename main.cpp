#include <iostream>
#include <sstream>
#include <string>
#include "Config.h"
#include "Benchmark.h"

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
    std::cout << "Usage: ./misinfo [--users N --edgeprob P --mis P --fact P --ticks T --warmup W\n"
                 "                  --budget B --source S --seed X --mc M --runs R\n"
                 "                  --strategy none|random|degree|marginal | --benchmark]\n"
                 "No arguments -> interactive mode.\n";
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
    else return true;
    return false;
}
}

int main(int argc, char** argv) {
    SimulationConfig cfg;
    int strategy = 3;               // 0 none, 1 random, 2 degree, 3 marginal
    bool benchmark = false;

    if (argc > 1) {
        try {
            for (int i = 1; i < argc; i++) {
                std::string key = argv[i];
                if (key == "--benchmark") { benchmark = true; continue; }
                if (key == "--help") { usage(); return 0; }
                if (i + 1 >= argc) { usage(); return 1; }
                std::string value = argv[++i];
                if (key == "--users") cfg.users = std::stoi(value);
                else if (key == "--edgeprob") cfg.edgeProb = std::stod(value);
                else if (key == "--mis") cfg.misProb = std::stod(value);
                else if (key == "--fact") cfg.factProb = std::stod(value);
                else if (key == "--ticks") cfg.ticks = std::stoi(value);
                else if (key == "--warmup") cfg.warmup = std::stoi(value);
                else if (key == "--budget") cfg.budget = std::stoi(value);
                else if (key == "--source") cfg.source = std::stoi(value);
                else if (key == "--seed") cfg.seed = std::stoull(value);
                else if (key == "--mc") cfg.mcTrials = std::stoi(value);
                else if (key == "--runs") cfg.runs = std::stoi(value);
                else if (key == "--strategy") { strategy = strategyFromName(value); if (strategy < 0) { usage(); return 1; } }
                else { usage(); return 1; }
            }
        } catch (const std::exception&) {          // stoi/stod failed: the value was not a number
            std::cerr << "Error: a numeric option got a value that is not a number\n";
            return 1;
        }
    } else {
        std::cout << "=== Misinformation vs Fact-Check Simulator ===\n(press Enter to accept the default)\n";
        cfg.users = ask("Number of users", cfg.users);
        cfg.edgeProb = ask("Connection probability", cfg.edgeProb);
        cfg.misProb = ask("Misinformation spread probability", cfg.misProb);
        cfg.factProb = ask("Fact-check spread probability", cfg.factProb);
        cfg.ticks = ask("Total ticks", cfg.ticks);
        cfg.warmup = ask("Warm-up ticks", cfg.warmup);
        cfg.budget = ask("Moderator budget", cfg.budget);
        cfg.source = ask("Source user (-1 = random)", cfg.source);
        cfg.seed = ask("Random seed", cfg.seed);
        cfg.mcTrials = ask("Monte Carlo trials", cfg.mcTrials);
        int mode = ask("Mode: 1=single run, 2=benchmark", 1);
        if (mode == 2) { cfg.runs = ask("Benchmark experiments", cfg.runs); benchmark = true; }
        else strategy = ask("Strategy 0=none 1=random 2=degree 3=marginal", 3);
    }

    if (!configIsValid(cfg)) return 1;
    if (strategy < 0 || strategy > 3) { std::cerr << "Error: strategy must be 0..3\n"; return 1; }

    if (benchmark) runBenchmark(cfg, std::cout);
    else runSingle(cfg, strategy, std::cout);
    return 0;
}
