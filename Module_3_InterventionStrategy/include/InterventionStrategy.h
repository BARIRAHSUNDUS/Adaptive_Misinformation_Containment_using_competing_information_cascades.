#ifndef INTERVENTIONSTRATEGY_H
#define INTERVENTIONSTRATEGY_H

#include <vector>
#include <string>

using namespace std;

// Abstract base class for every moderator intervention strategy.
//
// Each strategy looks at the CURRENT adjacency list and the CURRENT
// status vector (same representation CascadeModel uses internally)
// and picks one user to fact-check next, or -1 if there is no
// eligible candidate (budget exhausted upstream, or every user is
// already FACT_CHECKED).
//
// Eligibility rule used by every concrete strategy: a user already
// FACT_CHECKED is never re-selected. NORMAL and MISINFORMATION users
// are both eligible -- correcting an already-misinformed user still
// stops them from spreading further, so it's a useful pick.
class InterventionStrategy
{
public:
    virtual int selectUser(
        const vector<vector<int>>& graph,
        const vector<int>& status,
        int remainingBudget
    ) = 0;

    virtual string getName() const = 0;

    virtual ~InterventionStrategy() {}
};

#endif
