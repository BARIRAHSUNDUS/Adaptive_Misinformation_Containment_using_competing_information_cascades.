#ifndef MARGINALGAINSTRATEGY_H
#define MARGINALGAINSTRATEGY_H

#include "InterventionStrategy.h"

using namespace std;

class MarginalGainStrategy : public InterventionStrategy
{
public:
    int selectUser(
        const vector<vector<int>>& graph,
        const vector<int>& status,
        int remainingBudget
    ) override;

    string getName() const override;

private:
    // Heuristic marginal-gain score: number of this user's NORMAL
    // (not-yet-reached) neighbours. Correcting a user with many
    // normal neighbours removes the most potential future spread.
    // This is a deterministic, one-hop approximation of "true"
    // marginal gain (which would require simulating the rest of the
    // cascade with/without this correction) -- a documented, reasonable
    // simplification rather than full Monte Carlo re-simulation.
    int computeLocalSpreadScore(
        const vector<vector<int>>& graph,
        const vector<int>& status,
        int user
    );
};

#endif
