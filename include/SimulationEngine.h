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

// What the moderator did at one intervention (for the GUI).
struct InterventionTrace {
    int user = -1;
    int candidates = 0;                                // how many users were eligible
    std::vector<std::pair<int, double>> top;           // best-scored candidates (up to 5), best first
};
// Everything that happened in one tick. ticks[0] = initialisation (the source), ticks[t] = round t.
struct TickTrace {
    std::vector<Transmission> misinformed, corrected;  // propagation of this round
    std::vector<InterventionTrace> interventions;      // moderator actions AFTER the propagation
    bool skipped = false;                              // an intervention was due but could no longer help (rumour over, or last tick)
};
struct RunTrace {
    int source = -1;
    std::vector<TickTrace> ticks;
};

class SimulationEngine {
    const SocialGraph& graph_;
    SimulationConfig cfg_;
    InterventionStrategy* strategy_;     // nullptr = no-intervention baseline
public:
    SimulationEngine(const SocialGraph& g, const SimulationConfig& cfg, InterventionStrategy* s);
    // Tick 0 = initialisation; ticks 1..T = propagation rounds. Interventions happen AFTER
    // the propagation of their tick; the fact-check then spreads from the next tick on.
    SimulationResult run(int source, uint64_t runSeed, std::ostream* log = nullptr, RunTrace* trace = nullptr);
};
#endif
