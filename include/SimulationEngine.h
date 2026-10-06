#ifndef SIMULATIONENGINE_H
#define SIMULATIONENGINE_H
#include <vector>
#include <ostream>
#include <cstdint>
#include "Config.h"
#include "SocialGraph.h"
#include "CascadeModel.h"
#include "ModeratorBudget.h"
#include "InterventionStrategy.h"

struct SimulationResult {
    int misinformationReach = 0;
    int finalMisinformed = 0;
    int factCheckReach = 0;
    int directCorrections = 0;
    int propagatedCorrections = 0;
    int interventionsUsed = 0;
    int terminationTick = 0;
    double runtimeMs = 0.0;
    std::vector<int> selectedUsers;
};

class SimulationEngine {
    const SocialGraph& graph_;
    SimulationConfig cfg_;
    InterventionStrategy* strategy_;     // nullptr = no-intervention baseline
    std::vector<int> schedule_;
public:
    SimulationEngine(const SocialGraph& g, const SimulationConfig& cfg, InterventionStrategy* s);
    // Tick 0 = initialisation; ticks 1..T = propagation rounds. Interventions happen AFTER
    // the propagation of their tick; the fact-check then spreads from the next tick on.
    SimulationResult run(int source, uint64_t runSeed, std::ostream* log = nullptr);
    const std::vector<int>& schedule() const { return schedule_; }
    // t_i = warmup + 1 + floor((i-1)*(T-warmup)/b), duplicates removed, all <= T.
    static std::vector<int> makeSchedule(int totalTicks, int warmup, int budget);
};
#endif
