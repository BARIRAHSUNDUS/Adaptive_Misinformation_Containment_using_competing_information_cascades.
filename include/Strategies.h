#ifndef STRATEGIES_H
#define STRATEGIES_H
#include <random>
#include "InterventionStrategy.h"

class RandomStrategy : public InterventionStrategy {
    std::mt19937 rng_{12345};
public:
    int selectCandidate(const InterventionContext& ctx) override;
    std::string name() const override { return "Random"; }
    void reset(uint64_t seed) override { rng_.seed((uint32_t)(seed ^ (seed >> 32))); }
};

class HighestDegreeStrategy : public InterventionStrategy {
public:
    int selectCandidate(const InterventionContext& ctx) override;
    std::string name() const override { return "Highest Degree"; }
};

// Adaptive greedy: estimated marginal gain via Monte Carlo with common random numbers.
class MarginalGainStrategy : public InterventionStrategy {
    int trials_;
public:
    explicit MarginalGainStrategy(int trials) : trials_(trials < 1 ? 1 : trials) {}
    int selectCandidate(const InterventionContext& ctx) override;
    std::string name() const override { return "Marginal Gain"; }
};
#endif
