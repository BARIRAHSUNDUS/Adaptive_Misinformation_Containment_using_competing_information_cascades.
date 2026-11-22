#ifndef INTERVENTIONSTRATEGY_H
#define INTERVENTIONSTRATEGY_H
#include <vector>
#include <string>
#include <utility>
#include <cstdint>
#include "SocialGraph.h"
#include "CascadeModel.h"


struct InterventionContext {
    const SocialGraph&  graph;
    const CascadeModel& cascade;
    int tick;                               // current tick (intervention happens after its propagation)
    int totalTicks;
    int interventionIndex;                  // 0-based
    uint64_t seed;                          // strategy-private seed (independent of the real run's randomness)
    const std::vector<int>& candidates;     // CascadeModel::eligibleUsers(): the same list for every strategy
};

class InterventionStrategy {
protected:
    std::vector<std::pair<int, double>> scores_;   // (user, score) of the last decision, for display
public:
    virtual ~InterventionStrategy() = default;
    virtual int selectCandidate(const InterventionContext& ctx) = 0;   
    virtual std::string name() const = 0;
    virtual void reset(uint64_t /*seed*/) {}
    const std::vector<std::pair<int, double>>& lastScores() const { return scores_; }
};
#endif
