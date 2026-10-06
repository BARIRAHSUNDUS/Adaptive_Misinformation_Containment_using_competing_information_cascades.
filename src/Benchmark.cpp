#include "Benchmark.h"
#include "SimulationEngine.h"
#include "Strategies.h"
#include <cmath>
#include <iomanip>
#include <memory>
#include <random>

namespace {
double mean(const std::vector<double>& v) {
    double sum = 0;
    for (double x : v) sum += x;
    return v.empty() ? 0 : sum / v.size();
}
double stddev(const std::vector<double>& v) {
    if (v.size() < 2) return 0;
    double m = mean(v), sum = 0;
    for (double x : v) sum += (x - m) * (x - m);
    return std::sqrt(sum / (v.size() - 1));
}
// the source is either the one given in the config or a random user
int pickSource(const SimulationConfig& cfg, int n, std::mt19937& rng) {
    if (n <= 0) return -1;
    if (cfg.source >= 0 && cfg.source < n) return cfg.source;
    std::uniform_int_distribution<int> d(0, n - 1);
    return d(rng);
}
}

void runSingle(const SimulationConfig& cfg, int strategyId, std::ostream& os) {
    std::mt19937 rng((uint32_t)cfg.seed);
    SocialGraph g = SocialGraph::generateRandom(cfg.users, cfg.edgeProb, rng);
    int source = pickSource(cfg, g.numUsers(), rng);

    std::unique_ptr<InterventionStrategy> strategy;
    if (strategyId == 1) strategy.reset(new RandomStrategy());
    else if (strategyId == 2) strategy.reset(new HighestDegreeStrategy());
    else if (strategyId == 3) strategy.reset(new MarginalGainStrategy(cfg.mcTrials));
    SimulationEngine engine(g, cfg, strategy.get());

    os << "===============================\nMISINFORMATION SIMULATOR\n===============================\n"
       << "Users: " << g.numUsers() << "  Edges: " << g.numEdges()
       << "  Components: " << g.countComponents() << "  Isolated: " << g.countIsolated() << "\n"
       << "Source: User " << source << "  Ticks: " << cfg.ticks << "  Warm-up: " << cfg.warmup
       << "  Budget: " << cfg.budget << "  Seed: " << cfg.seed << "\n"
       << "Strategy: " << (strategy ? strategy->name() : std::string("No Intervention")) << "\n"
       << "Intervention schedule:";
    if (!strategy || engine.schedule().empty()) os << " none";
    else for (int t : engine.schedule()) os << " Tick " << t;
    os << "\n\n";
    if (g.numUsers() <= 30) { g.displayGraph(os); os << "\n"; }

    SimulationResult r = engine.run(source, cfg.seed, &os);
    os << "\n-------------------------------\nRESULTS\n-------------------------------\n"
       << "Misinformation reach:   " << r.misinformationReach << "\n"
       << "Final misinformed:      " << r.finalMisinformed << "\n"
       << "Fact-check reach:       " << r.factCheckReach << " (direct " << r.directCorrections
       << ", propagated " << r.propagatedCorrections << ")\n"
       << "Interventions used:     " << r.interventionsUsed << "\n"
       << "Terminated at tick:     " << r.terminationTick << "\n"
       << "Runtime:                " << std::fixed << std::setprecision(3) << r.runtimeMs << " ms\n";
}

// Runs many experiments. In each one ALL strategies get the same graph, source and seed,
// so the comparison is fair.
void runBenchmark(const SimulationConfig& cfg, std::ostream& os) {
    RandomStrategy randomStrategy;
    HighestDegreeStrategy degreeStrategy;
    MarginalGainStrategy marginalStrategy(cfg.mcTrials);
    InterventionStrategy* strategies[4] = {nullptr, &randomStrategy, &degreeStrategy, &marginalStrategy};
    const char* names[4] = {"No Intervention", "Random", "Highest Degree", "Marginal Gain"};

    std::vector<double> reach[4], factReach[4], prevented[4], runtime[4];
    int marginalBeatsRandom = 0, marginalBeatsDegree = 0;
    double totalEdges = 0;

    for (int e = 0; e < cfg.runs; e++) {
        uint64_t seed = cfg.seed + e;                  // experiment seed: 1001, 1002, ...
        std::mt19937 rng((uint32_t)seed);
        SocialGraph g = SocialGraph::generateRandom(cfg.users, cfg.edgeProb, rng);
        int source = pickSource(cfg, g.numUsers(), rng);
        totalEdges += g.numEdges();

        SimulationResult res[4];
        for (int s = 0; s < 4; s++) {
            SimulationEngine engine(g, cfg, strategies[s]);
            res[s] = engine.run(source, seed);
            reach[s].push_back(res[s].misinformationReach);
            factReach[s].push_back(res[s].factCheckReach);
            runtime[s].push_back(res[s].runtimeMs);
            // "prevented" = how many fewer users were misinformed than with no intervention
            prevented[s].push_back(res[0].misinformationReach - res[s].misinformationReach);
        }
        if (res[3].misinformationReach < res[1].misinformationReach) marginalBeatsRandom++;
        if (res[3].misinformationReach < res[2].misinformationReach) marginalBeatsDegree++;
    }

    os << "===============================\nBENCHMARK (" << cfg.runs << " experiments, seeds "
       << cfg.seed << ".." << cfg.seed + cfg.runs - 1 << ")\n===============================\n"
       << "Users=" << cfg.users << " edgeP=" << cfg.edgeProb << " misP=" << cfg.misProb
       << " factP=" << cfg.factProb << " ticks=" << cfg.ticks << " warmup=" << cfg.warmup
       << " budget=" << cfg.budget << " MC trials=" << cfg.mcTrials << "\n"
       << "Avg edges per graph: " << std::fixed << std::setprecision(1) << totalEdges / cfg.runs << "\n\n";
    os << std::left << std::setw(17) << "Strategy" << std::right << std::setw(10) << "AvgReach"
       << std::setw(9) << "StdDev" << std::setw(11) << "Prevented"
       << std::setw(10) << "FactReach" << std::setw(12) << "Runtime(ms)" << "\n"
       << std::string(69, '-') << "\n";
    for (int s = 0; s < 4; s++)
        os << std::left << std::setw(17) << names[s] << std::right << std::fixed << std::setprecision(2)
           << std::setw(10) << mean(reach[s]) << std::setw(9) << stddev(reach[s])
           << std::setw(11) << mean(prevented[s]) << std::setw(10) << mean(factReach[s])
           << std::setw(12) << std::setprecision(3) << mean(runtime[s]) << "\n";
    os << std::string(69, '-') << "\n"
       << "Marginal Gain strictly better than Random in " << marginalBeatsRandom << "/" << cfg.runs
       << " experiments, than Highest Degree in " << marginalBeatsDegree << "/" << cfg.runs << ".\n"
       << "Prevented = (no-intervention reach - strategy reach), averaged over experiments.\n";
}
