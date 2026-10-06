#ifndef BENCHMARK_H
#define BENCHMARK_H
#include <ostream>
#include "Config.h"
void runBenchmark(const SimulationConfig& cfg, std::ostream& os);
void runSingle(const SimulationConfig& cfg, int strategyId, std::ostream& os);  // 0 none,1 random,2 degree,3 marginal
#endif