
#include "Benchmark.h"
#include "SimulationEngine.h"
#include "Strategies.h"
#include "Stats.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <memory>
#include <random>
#include <sstream>

namespace {
const int NS = 4;      // none, random, degree, marginal  (cheapest to most expensive, after "none")
const char* NAMES[NS] = {"No Intervention", "Random", "Highest Degree", "Marginal Gain"};
const char* WHY[NS] = {
    "",
    "picks an eligible user uniformly at random (it ignores both the network and the rumour)",
    "picks the best-connected user, because hubs pass the rumour (and the correction) to the most people",
    "simulates the future for every candidate and picks the one that saves the most users"};

std::string graphName(const SimulationConfig& cfg) {
    return cfg.graph == GraphType::ScaleFree ? "scale-free (m=" + std::to_string(cfg.baM) + ")" : "random";
}
std::string num(double v, int prec = 2) { std::ostringstream o; o << std::fixed << std::setprecision(prec) << v; return o.str(); }

// "0.37" for a fixed probability, "random 0.20-0.60 (avg 0.41)" for a randomized one
std::string probText(bool randomized, double fixedValue, double lo, double hi, double avg) {
    if (!randomized) return num(fixedValue);
    return "random " + num(lo) + "-" + num(hi) + " (avg " + num(avg) + ")";
}

struct Experiments {
    std::vector<double> reach[NS], factReach[NS], prevented[NS], runtime[NS], used[NS];
    double totalEdges = 0, sumEdgeP = 0, sumMisP = 0, sumFactP = 0;
};

// Runs cfg.runs experiments. In each one the probabilities are drawn once (if randomized) and ALL strategies
// get the same graph, source, probabilities and seed, so the comparison is fair and "prevented" is paired.
Experiments runExperiments(const SimulationConfig& cfg) {
    RandomStrategy randomStrategy;
    HighestDegreeStrategy degreeStrategy;
    MarginalGainStrategy marginalStrategy(cfg.mcTrials);
    InterventionStrategy* strategies[NS] = {nullptr, &randomStrategy, &degreeStrategy, &marginalStrategy};

    Experiments ex;
    for (int e = 0; e < cfg.runs; e++) {
        uint64_t seed = cfg.seed + e;                  // experiment seed: 1001, 1002, ...
        SimulationConfig ec = randomizedConfig(cfg, seed);
        std::mt19937 rng((uint32_t)seed);
        SocialGraph g = buildGraph(ec, rng);
        int source = chooseSource(ec, g.numUsers(), rng);
        ex.totalEdges += g.numEdges();
        ex.sumEdgeP += ec.edgeProb; ex.sumMisP += ec.misProb; ex.sumFactP += ec.factProb;

        SimulationResult res[NS];
        for (int s = 0; s < NS; s++) {
            SimulationEngine engine(g, ec, strategies[s]);
            res[s] = engine.run(source, seed);
            ex.reach[s].push_back(res[s].misinformationReach);
            ex.factReach[s].push_back(res[s].factCheckReach);
            ex.runtime[s].push_back(res[s].runtimeMs);
            ex.used[s].push_back(res[s].interventionsUsed);
            ex.prevented[s].push_back(res[0].misinformationReach - res[s].misinformationReach);
        }
    }
    return ex;
}

// Index (1..NS-1) of the strategy with the lowest average reach. Ties go to the cheaper strategy.
int bestStrategy(const Experiments& ex) {
    int best = 1;
    for (int s = 2; s < NS; s++) if (mean(ex.reach[s]) < mean(ex.reach[best])) best = s;
    return best;
}

// Paired comparison on the same experiments: how many more users does `better` save than `other`?
struct Paired { double diff, ci; int win, tie, loss; };
Paired compare(const Experiments& ex, int better, int other) {
    Paired p{0, 0, 0, 0, 0};
    std::vector<double> d;
    for (size_t i = 0; i < ex.reach[0].size(); i++) {
        double x = ex.reach[other][i] - ex.reach[better][i];     // > 0: `better` had the lower reach
        d.push_back(x);
        if (x > 0) p.win++; else if (x == 0) p.tie++; else p.loss++;
    }
    p.diff = mean(d); p.ci = ci95HalfWidth(d);
    return p;
}

void printVerdict(const SimulationConfig& cfg, const Experiments& ex, std::ostream& os) {
    const int best = bestStrategy(ex);
    const double none = mean(ex.reach[0]), bestReach = mean(ex.reach[best]);

    std::vector<int> order = {1, 2, 3};
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        double ma = mean(ex.reach[a]), mb = mean(ex.reach[b]);
        return ma != mb ? ma < mb : a < b; });

    os << "\n===============================\nVERDICT\n===============================\n";
    os << "Best strategy: " << NAMES[best] << "  (average reach " << num(bestReach, 1) << " of " << cfg.users
       << " users; " << num(none - bestReach, 1) << " fewer misinformed per run = "
       << num(none > 0 ? 100.0 * (none - bestReach) / none : 0, 0) << "% less spread than doing nothing)\n";
    os << "Why: it " << WHY[best] << ".\n";
    os << "Ranking (lowest reach first): ";
    for (size_t i = 0; i < order.size(); i++)
        os << (i ? "  >  " : "") << NAMES[order[i]] << " " << num(mean(ex.reach[order[i]]), 1);
    os << "\n";

    // which strategies can NOT be told apart from the best?
    std::vector<int> tied;
    for (int s = 1; s < NS; s++) {
        if (s == best) continue;
        Paired p = compare(ex, best, s);                         // best vs s
        if (p.diff - p.ci <= 0) tied.push_back(s);               // CI of (reach_s - reach_best) reaches 0 -> not clearly worse
    }
    os << "Evidence (same graph, source and probabilities for every strategy, " << cfg.runs << " experiments):\n";
    for (int s = 1; s < NS; s++) {
        if (s == best) continue;
        Paired p = compare(ex, best, s);
        os << "  vs " << std::left << std::setw(15) << NAMES[s] << std::right << "saves " << std::setw(5) << num(p.diff, 2)
           << " +/- " << num(p.ci, 2) << " more users per run   (win " << p.win << " / tie " << p.tie << " / loss " << p.loss << ")   "
           << (p.diff - p.ci > 0 ? "clearly better" : "not clearly better") << "\n";
    }
    if (tied.empty())
        os << "Conclusion: " << NAMES[best] << " is clearly better than every other strategy.\n";
    else {
        os << "Conclusion: " << NAMES[best] << " has the lowest average reach, but the gap to ";
        for (size_t i = 0; i < tied.size(); i++) os << (i ? ", " : "") << NAMES[tied[i]];
        os << " is within the statistical noise of this benchmark (more experiments with --runs would narrow it).\n";
        int cheapest = best;                                    // fastest strategy among the best and the tied ones
        for (int s : tied) if (mean(ex.runtime[s]) < mean(ex.runtime[cheapest])) cheapest = s;
        if (cheapest != best && mean(ex.runtime[best]) >= 2 * mean(ex.runtime[cheapest]))
            os << "Cost: " << NAMES[cheapest] << " is not clearly worse and about "
               << num(mean(ex.runtime[best]) / std::max(mean(ex.runtime[cheapest]), 1e-9), 0) << "x faster ("
               << num(mean(ex.runtime[cheapest]), 3) << " ms per run against " << num(mean(ex.runtime[best]), 3)
               << " ms for " << NAMES[best] << ").\n";
    }
    os << "Interventions actually used per run (budget " << cfg.budget << "): ";
    for (int s = 1; s < NS; s++) os << (s > 1 ? ", " : "") << NAMES[s] << " " << num(mean(ex.used[s]), 1);
    os << "\n";
}
}

SocialGraph buildGraph(const SimulationConfig& cfg, std::mt19937& rng) {
    if (cfg.graph == GraphType::ScaleFree) return SocialGraph::generateScaleFree(cfg.users, cfg.baM, rng);
    return SocialGraph::generateRandom(cfg.users, cfg.edgeProb, rng);
}

// the source is either the one given in the config or a random user
int chooseSource(const SimulationConfig& cfg, int n, std::mt19937& rng) {
    if (n <= 0) return -1;
    if (cfg.source >= 0 && cfg.source < n) return cfg.source;
    std::uniform_int_distribution<int> d(0, n - 1);
    return d(rng);
}

SimulationConfig randomizedConfig(const SimulationConfig& cfg, uint64_t seed) {
    SimulationConfig c = cfg;
    std::mt19937 rng((uint32_t)((seed * 2654435761ULL) ^ (seed >> 32) ^ 0x5bd1e995u));
    auto draw = [&](double lo, double hi) { std::uniform_real_distribution<double> d(lo, hi); return std::round(d(rng) * 100) / 100; };
    double e = draw(EDGE_PROB_MIN, EDGE_PROB_MAX), m = draw(MIS_PROB_MIN, MIS_PROB_MAX), f = draw(FACT_PROB_MIN, FACT_PROB_MAX);
    if (cfg.randEdge) c.edgeProb = e;
    if (cfg.randMis) c.misProb = m;
    if (cfg.randFact) c.factProb = f;
    return c;
}

void runSingle(const SimulationConfig& cfg, int strategyId, std::ostream& os) {
    SimulationConfig ec = randomizedConfig(cfg, cfg.seed);
    std::mt19937 rng((uint32_t)cfg.seed);
    SocialGraph g = buildGraph(ec, rng);
    int source = chooseSource(ec, g.numUsers(), rng);

    std::unique_ptr<InterventionStrategy> strategy;
    if (strategyId == 1) strategy.reset(new RandomStrategy());
    else if (strategyId == 2) strategy.reset(new HighestDegreeStrategy());
    else if (strategyId == 3) strategy.reset(new MarginalGainStrategy(ec.mcTrials));
    SimulationEngine engine(g, ec, strategy.get());

    os << "===============================\nMISINFORMATION SIMULATOR\n===============================\n"
       << "Users: " << g.numUsers() << "  Edges: " << g.numEdges()
       << "  Components: " << g.countComponents() << "  Isolated: " << g.countIsolated() << "\n"
       << "Graph: " << graphName(ec) << "\n"
       << "Probabilities" << (ec.randEdge || ec.randMis || ec.randFact ? " (random for this run)" : "") << ": connection "
       << num(ec.edgeProb) << ", misinformation spread " << num(ec.misProb) << ", fact-check spread " << num(ec.factProb) << "\n"
       << "Source: User " << source << "  Ticks: " << ec.ticks << "  Warm-up: " << ec.warmup
       << "  Budget: " << ec.budget << "  Seed: " << ec.seed << "\n"
              << "Strategy: " << (strategy ? strategy->name() : std::string("No Intervention")) << "\n"
       << "Interventions: " << (strategy ? "once per tick from tick " + std::to_string(ec.warmup + 1) + " until the budget is used up or the rumour stops spreading" : std::string("none")) << "\n";
    os << "\n\n";
    if (g.numUsers() <= 30) { g.displayGraph(os); os << "\n"; }

    SimulationResult r = engine.run(source, cfg.seed, &os);
    SimulationEngine baseline(g, ec, nullptr);
    SimulationResult b = baseline.run(source, cfg.seed);              // same run, nobody fact-checks
    os << "\n-------------------------------\nRESULTS\n-------------------------------\n"
       << "Misinformation reach:   " << r.misinformationReach << "\n"
       << "Final misinformed:      " << r.finalMisinformed << "\n"
       << "Fact-check reach:       " << r.factCheckReach << " (direct " << r.directCorrections
       << ", propagated " << r.propagatedCorrections << ")\n"
       << "Interventions used:     " << r.interventionsUsed << "\n"
       << "Terminated at tick:     " << r.terminationTick << "\n"
       << "Runtime:                " << std::fixed << std::setprecision(3) << r.runtimeMs << " ms\n";
    if (strategy)
        os << "Without any intervention the rumour would have reached " << b.misinformationReach << " users, so "
           << strategy->name() << " saved " << (b.misinformationReach - r.misinformationReach) << " users in this run.\n"
           << "(One run is random; use the benchmark to compare strategies.)\n";
}

void runBenchmark(const SimulationConfig& cfg, std::ostream& os) {
    Experiments ex = runExperiments(cfg);
    const double n = cfg.runs;

    os << "===============================\nBENCHMARK (" << cfg.runs << " experiments, seeds "
       << cfg.seed << ".." << cfg.seed + cfg.runs - 1 << ")\n===============================\n"
       << "Users=" << cfg.users << " graph=" << graphName(cfg) << " ticks=" << cfg.ticks << " warmup=" << cfg.warmup
       << " budget=" << cfg.budget << " MC trials=" << cfg.mcTrials << "\n"
       << "Moderator: one intervention per tick from tick " << cfg.warmup + 1 << " until the budget is used up or the rumour stops spreading\n"
       << "Probabilities are drawn again in every experiment (all strategies share them):\n"
       << "  misinformation spread: " << probText(cfg.randMis, cfg.misProb, MIS_PROB_MIN, MIS_PROB_MAX, ex.sumMisP / n) << "\n"
       << "  fact-check spread:     " << probText(cfg.randFact, cfg.factProb, FACT_PROB_MIN, FACT_PROB_MAX, ex.sumFactP / n) << "\n";
    if (cfg.graph == GraphType::ErdosRenyi)
        os << "  connection:            " << probText(cfg.randEdge, cfg.edgeProb, EDGE_PROB_MIN, EDGE_PROB_MAX, ex.sumEdgeP / n) << "\n";
    os << "Avg edges per graph: " << std::fixed << std::setprecision(1) << ex.totalEdges / n << "\n\n";
    const int best = bestStrategy(ex);
    os << std::left << std::setw(17) << "Strategy" << std::right << std::setw(10) << "AvgReach"
       << std::setw(9) << "StdDev" << std::setw(11) << "Prevented" << std::setw(9) << "+/-95%"
       << std::setw(10) << "FactReach" << std::setw(12) << "Runtime(ms)" << "\n"
       << std::string(78, '-') << "\n";
    for (int s = 0; s < NS; s++)
        os << std::left << std::setw(17) << NAMES[s] << std::right << std::fixed << std::setprecision(2)
           << std::setw(10) << mean(ex.reach[s]) << std::setw(9) << stddev(ex.reach[s])
           << std::setw(11) << mean(ex.prevented[s]) << std::setw(9) << ci95HalfWidth(ex.prevented[s])
           << std::setw(10) << mean(ex.factReach[s])
           << std::setw(12) << std::setprecision(3) << mean(ex.runtime[s]) << (s == best ? "  <-- best" : "") << "\n";
    os << std::string(78, '-') << "\n"
       << "AvgReach = users ever misinformed (lower is better). Prevented = no-intervention reach minus the\n"
       << "strategy's reach on the same experiment; +/-95% is the half-width of its 95% confidence interval.\n";
    printVerdict(cfg, ex, os);
}

void runBudgetSweep(const SimulationConfig& cfg, const std::vector<int>& budgets, std::ostream& os) {
    os << "===============================\nBUDGET SWEEP (" << cfg.runs << " experiments per budget)\n"
       << "===============================\n"
       << "Users=" << cfg.users << " graph=" << graphName(cfg) << " ticks=" << cfg.ticks << " warmup=" << cfg.warmup
       << " MC trials=" << cfg.mcTrials << "\n"
       << "Moderator: one intervention per tick from tick " << cfg.warmup + 1 << " until the budget is used up or the rumour stops spreading\n"
       << (cfg.randEdge || cfg.randMis || cfg.randFact
               ? "Probabilities not fixed on the command line are random per experiment (same draws for every budget and strategy).\n"
               : "Probabilities are fixed (same for every experiment).\n")
       << "Average misinformation reach (lower is better). 'Used' = interventions actually used per run (Random/Degree/Marginal).\n\n"
       << std::left << std::setw(8) << "Budget" << std::right;
    for (int s = 0; s < NS; s++) os << std::setw(17) << NAMES[s];
    os << std::setw(17) << "Best" << std::setw(17) << "Used" << "\n" << std::string(8 + 17 * (NS + 2), '-') << "\n";
    double prevUsed = -1;                                       // total interventions used at the previous budget
    bool budgetNeverBinds = false;
    for (int b : budgets) {
        SimulationConfig c = cfg;
        c.budget = b;
        Experiments ex = runExperiments(c);
        os << std::left << std::setw(8) << b << std::right << std::fixed << std::setprecision(2);
        for (int s = 0; s < NS; s++) os << std::setw(17) << mean(ex.reach[s]);
        os << std::setw(17) << (b == 0 ? "(no budget)" : NAMES[bestStrategy(ex)]);
        os << std::setw(17) << (num(mean(ex.used[1]), 1) + "/" + num(mean(ex.used[2]), 1) + "/" + num(mean(ex.used[3]), 1)) << "\n";
        // the budget no longer matters once the moderator uses (almost) fewer interventions than it is allowed
        double total = mean(ex.used[1]) + mean(ex.used[2]) + mean(ex.used[3]);
        if (b > 0 && prevUsed >= 0 && total - prevUsed < 0.05 && mean(ex.used[3]) < b - 0.5) budgetNeverBinds = true;
        prevUsed = total;
    }
    if (budgetNeverBinds)
        os << "\nNote: from some budget on, the rows repeat because the moderator stops on its own when the rumour has stopped\n"
              "spreading, so the extra budget is never used. Use smaller budgets or a faster rumour (--mis) to see the budget matter.\n";
}

// Step 3: the same three strategies under FIXED conditions (network type x fact-check speed), so every row is
// a clear question ("on a scale-free network with a slow fact-check, who wins?") instead of one random mix.
void runConditionStudy(const SimulationConfig& cfg, std::ostream& os) {
    struct Cond { const char* name; GraphType graph; double fact; };
    const Cond conds[] = {
        {"random network, slow fact-check",     GraphType::ErdosRenyi, 0.20},
        {"random network, fast fact-check",     GraphType::ErdosRenyi, 0.60},
        {"scale-free network, slow fact-check", GraphType::ScaleFree,  0.20},
        {"scale-free network, fast fact-check", GraphType::ScaleFree,  0.60}};
    const double MIS = 0.50, EDGE = 0.04;                       // EDGE 0.04 on 100 users ~ the same average degree as scale-free m=2

    os << "\nCONDITION STUDY  (" << cfg.runs << " experiments per condition, " << cfg.users << " users, " << cfg.ticks
       << " ticks, budget " << cfg.budget << ")\n"
       << "Numbers = average users who ever believed the rumour. LOWER IS BETTER.\n"
       << "Takeoff = how often the rumour really spread when nobody stopped it.\n\n"
       << std::left << std::setw(36) << "Condition" << std::right << std::setw(8) << "Takeoff"
       << std::setw(8) << "None" << std::setw(8) << "Random" << std::setw(8) << "Degree" << std::setw(10) << "Marginal"
       << "   Winner\n" << std::string(36 + 8 * 4 + 10 + 18, '-') << "\n";
    int wins[NS] = {0, 0, 0, 0}, clear[NS] = {0, 0, 0, 0}, nCond = 0;
    double sumReach[NS] = {0, 0, 0, 0}, sumTime[NS] = {0, 0, 0, 0};

    for (const Cond& cd : conds) {
        SimulationConfig c = cfg;
        c.graph = cd.graph; c.factProb = cd.fact; c.misProb = MIS; c.edgeProb = EDGE;
        c.randEdge = c.randMis = c.randFact = false;            // fixed conditions: nothing is drawn
        Experiments ex = runExperiments(c);

        int takeoff = 0;
        for (double r : ex.reach[0]) if (r >= 0.1 * c.users) takeoff++;
        const int best = bestStrategy(ex);
        int second = -1;                                        // runner-up = lowest reach among the others
        for (int s = 1; s < NS; s++)
            if (s != best && (second < 0 || mean(ex.reach[s]) < mean(ex.reach[second]))) second = s;
        Paired p = compare(ex, best, second);

        os << std::left << std::setw(36) << cd.name << std::right << std::fixed << std::setprecision(0)
           << std::setw(7) << 100.0 * takeoff / cfg.runs << "%" << std::setprecision(1);
        for (int s = 0; s < NS; s++) os << std::setw(s == 3 ? 10 : 8) << mean(ex.reach[s]);
        os << "   " << NAMES[best] << (p.diff - p.ci > 0 ? " *" : "") << "\n";
        wins[best]++; if (p.diff - p.ci > 0) clear[best]++; nCond++;
        for (int s = 0; s < NS; s++) { sumReach[s] += mean(ex.reach[s]); sumTime[s] += mean(ex.runtime[s]); }
    }
    os << "\n* = clearly better than the runner-up (95% confidence interval above zero).\n\n"
       << "IN SHORT\n";
    const int top = (int)(std::max_element(wins + 1, wins + NS) - wins);
    os << "- " << NAMES[top] << " had the lowest reach in " << wins[top] << " of " << nCond << " conditions"
       << " (clearly better in " << clear[top] << ").\n";
    os << "- Averaged over the conditions the rumour reached: nobody " << num(sumReach[0] / nCond, 1) << ", Random " << num(sumReach[1] / nCond, 1)
       << ", Highest Degree " << num(sumReach[2] / nCond, 1) << ", Marginal Gain " << num(sumReach[3] / nCond, 1) << " users.\n";
    os << "- Saving vs doing nothing: Marginal Gain " << num(100.0 * (1 - sumReach[3] / sumReach[0]), 0) << "%, Highest Degree "
       << num(100.0 * (1 - sumReach[2] / sumReach[0]), 0) << "%, Random " << num(100.0 * (1 - sumReach[1] / sumReach[0]), 0) << "%.\n";
    os << "- Marginal Gain vs Highest Degree: " << num((sumReach[2] - sumReach[3]) / nCond, 1) << " fewer users misinformed on average, but about "
       << num(sumTime[3] / std::max(sumTime[2], 1e-9), 0) << "x slower to compute.\n\n";
}

// The project's main question: WHEN should the moderator start, and does it matter more than WHICH users are chosen?
// For every start delay the same experiments (same graphs, sources, probabilities, seeds) are run, so the rows are comparable.
void runTimingStudy(const SimulationConfig& cfg, const std::vector<int>& delays, std::ostream& os) {
    SimulationConfig base = cfg;
    // Unless the user fixed them, use a setting where the rumour really takes off, otherwise timing cannot matter.
    const bool defaulted = base.randEdge || base.randMis || base.randFact;
    if (base.randEdge) { base.edgeProb = 0.04; base.randEdge = false; }
    if (base.randMis)  { base.misProb  = 0.50; base.randMis  = false; }
    if (base.randFact) { base.factProb = 0.40; base.randFact = false; }

    os << "\nTIMING STUDY: when should the moderator start?  (" << cfg.runs << " experiments per row, " << cfg.users << " users, "
       << cfg.ticks << " ticks, budget " << cfg.budget << ")\n"
       << "Rumour spread " << num(base.misProb) << ", fact-check spread " << num(base.factProb) << ", " << graphName(base) << " network"
       << (defaulted ? " (fixed values chosen so that the rumour takes off)" : "") << ".\n"
       << "Numbers = average users who ever believed the rumour. LOWER IS BETTER. Nobody fact-checks: see 'None'.\n\n"
       << std::left << std::setw(20) << "First fact-check at" << std::right << std::setw(8) << "None" << std::setw(8) << "Random"
       << std::setw(8) << "Degree" << std::setw(10) << "Marginal" << "   Users saved by the best\n"
       << std::string(20 + 8 * 3 + 10 + 27, '-') << "\n";

    struct Row { int delay; double reach[NS]; };
    std::vector<Row> rows;
    for (int d : delays) {
        SimulationConfig c = base; c.warmup = d;
        Experiments ex = runExperiments(c);
        Row r; r.delay = d;
        for (int s = 0; s < NS; s++) r.reach[s] = mean(ex.reach[s]);
        rows.push_back(r);
        double bestReach = std::min(r.reach[1], std::min(r.reach[2], r.reach[3]));
        os << std::left << std::setw(20) << ("tick " + std::to_string(d + 1)) << std::right << std::fixed << std::setprecision(1)
           << std::setw(8) << r.reach[0] << std::setw(8) << r.reach[1] << std::setw(8) << r.reach[2] << std::setw(10) << r.reach[3]
           << "   " << r.reach[0] - bestReach << "\n";
    }
    if (rows.size() < 2) return;

    const Row& first = rows.front(); const Row& last = rows.back();
    os << "\nIN SHORT\n";
    // 1) how much does each tick of delay cost?
    int ticksSpan = last.delay - first.delay;
    for (int s = 1; s < NS; s++)
        os << (s == 1 ? "- Every extra tick of waiting costs about " : "") << (s > 1 ? ", " : "") << num((last.reach[s] - first.reach[s]) / std::max(ticksSpan, 1), 1)
           << " more users (" << NAMES[s] << ")" << (s == NS - 1 ? " on average.\n" : "");
    // 2) timing vs strategy: how big is the choice-of-strategy effect against the timing effect?
    double strategyEffect = first.reach[1] - std::min(first.reach[2], first.reach[3]);       // best smart strategy vs Random, on time
    double timingEffect = last.reach[3] - first.reach[3];                                    // same strategy, early vs late
    os << "- Choosing the user well (best strategy vs Random, started at tick " << first.delay + 1 << ") changes the reach by " << num(strategyEffect, 1)
       << " users; starting at tick " << last.delay + 1 << " instead of " << first.delay + 1 << " changes it by " << num(timingEffect, 1) << " (" << NAMES[3] << ").\n";
    // 3) is there a delay at which a smart strategy is worse than a simple one on time?
    for (const Row& r : rows) {
        if (r.delay > first.delay && r.reach[3] > first.reach[2]) {
            os << "- " << NAMES[3] << " started at tick " << r.delay + 1 << " (" << num(r.reach[3], 1) << ") is already worse than " << NAMES[2]
               << " started at tick " << first.delay + 1 << " (" << num(first.reach[2], 1) << "): a simple strategy on time beats a clever one that is late.\n";
            break;
        }
    }
    // 4) when does the moderator become useless?
    for (const Row& r : rows) {
        double best = std::min(r.reach[1], std::min(r.reach[2], r.reach[3]));
        if (first.reach[0] > 0 && (r.reach[0] - best) < 0.1 * (first.reach[0] - std::min(first.reach[1], std::min(first.reach[2], first.reach[3])))) {
            os << "- From tick " << r.delay + 1 << " on, even the best strategy saves less than a tenth of what it saves when it starts at tick "
               << first.delay + 1 << ": the rumour has already spread.\n";
            break;
        }
    }
    os << "\n";
}
