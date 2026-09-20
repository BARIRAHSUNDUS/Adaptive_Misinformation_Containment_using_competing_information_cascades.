#ifndef HIGHESTDEGREESTRATEGY_H
#define HIGHESTDEGREESTRATEGY_H

#include "InterventionStrategy.h"

using namespace std;

class HighestDegreeStrategy : public InterventionStrategy
{
public:
    int selectUser(
        const vector<vector<int>>& graph,
        const vector<int>& status,
        int remainingBudget
    ) override;

    string getName() const override;
};

#endif
