#include "Strategies.h"
#include <queue>

namespace {

int futureReach(CascadeModel sim, int userToCorrect, int ticksLeft, uint64_t seed) {
    if (userToCorrect >= 0) sim.correctUser(userToCorrect);
    for (int t = 0; t < ticksLeft && !sim.isQuiescent(); t++) sim.step(seed);
    return sim.misinformationReach();
}
}

int MarginalGainStrategy::selectCandidate(const InterventionContext& ctx) {
    scores_.clear();
    if (ctx.candidates.empty()) return -1;
    const int ticksLeft = ctx.totalTicks - ctx.cascade.currentTick();

    std::vector<uint64_t> trialSeed(trials_);
    std::vector<int> baseline(trials_);
    for (int k = 0; k < trials_; k++) {
        trialSeed[k] = ctx.seed + (uint64_t)ctx.interventionIndex * 100000 + k;
        baseline[k] = futureReach(ctx.cascade, -1, ticksLeft, trialSeed[k]);
    }

    std::priority_queue<std::pair<double, int>> heap;
    for (int c : ctx.candidates) {
        double saved = 0;
        for (int k = 0; k < trials_; k++)
            saved += baseline[k] - futureReach(ctx.cascade, c, ticksLeft, trialSeed[k]);
        double gain = saved / trials_;
        scores_.push_back({c, gain});
        heap.push({gain, -c});
    }
    return -heap.top().second;
}
