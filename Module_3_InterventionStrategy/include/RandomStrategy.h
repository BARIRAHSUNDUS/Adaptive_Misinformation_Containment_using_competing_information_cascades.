#ifndef RANDOMSTRATEGY_H
#define RANDOMSTRATEGY_H

#include <random>
#include "InterventionStrategy.h"

using namespace std;

class RandomStrategy : public InterventionStrategy
{
private:
    mt19937 rng;

public:
    // Own RNG stream, separate from CascadeModel's rand() calls, so
    // this strategy's picks don't interfere with propagation rolls
    // when comparing strategies on the same seeded graph.
    RandomStrategy(unsigned int seed);

    int selectUser(
        const vector<vector<int>>& graph,
        const vector<int>& status,
        int remainingBudget
    ) override;

    string getName() const override;
};

#endif
