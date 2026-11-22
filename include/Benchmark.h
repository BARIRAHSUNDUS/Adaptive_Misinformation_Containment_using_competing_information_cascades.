#ifndef BENCHMARK_H
#define BENCHMARK_H
#include <cstdint>
#include <ostream>
#include <vector>
#include <random>
#include "Config.h"
#include "SocialGraph.h"
// Returns cfg with the randomized probabilities drawn from `seed` (same seed -> same values).
// A probability whose randX flag is false keeps the value stored in cfg. The three values are always
// drawn in the same order, so fixing one probability does not change the other two.
SimulationConfig randomizedConfig(const SimulationConfig& cfg, uint64_t seed);
// The same graph / source choice the benchmark and single run use (so the GUI shows the same scenario).
SocialGraph buildGraph(const SimulationConfig& cfg, std::mt19937& rng);
int chooseSource(const SimulationConfig& cfg, int numUsers, std::mt19937& rng);
// 0 none, 1 random, 2 degree, 3 marginal
void runSingle(const SimulationConfig& cfg, int strategyId, std::ostream& os);
// Four setups (No Intervention + the three strategies) on identical graphs/sources/seeds; paired statistics against "No Intervention".
void runBenchmark(const SimulationConfig& cfg, std::ostream& os);
// Same experiments for several moderator budgets; prints average reach per strategy per budget.
void runBudgetSweep(const SimulationConfig& cfg, const std::vector<int>& budgets, std::ostream& os);
// Step 3: the three strategies under fixed conditions (random / scale-free network x slow / fast fact-check).
void runConditionStudy(const SimulationConfig& cfg, std::ostream& os);
// The main question: WHEN to start. Average reach per start delay (moderator first acts at tick delay+1) and strategy.
void runTimingStudy(const SimulationConfig& cfg, const std::vector<int>& delays, std::ostream& os);
#endif
