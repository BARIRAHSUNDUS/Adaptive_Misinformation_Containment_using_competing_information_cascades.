#ifndef CASCADEMODEL_H
#define CASCADEMODEL_H
#include <vector>
#include <queue>
#include <cstdint>
#include "SocialGraph.h"
#include "UserState.h"

struct TickReport { int newMisinformed = 0; int newCorrected = 0; };

// Two competing independent-cascade processes on one graph, advanced ONE
// synchronous tick at a time. Cheap to copy (graph is held by pointer), which
// is what lets MarginalGainStrategy run hypothetical futures.
class CascadeModel {
private:
    const SocialGraph* graph_;
    double pMis_, pFact_;
    std::vector<UserState> state_;
    std::queue<int> misQ_;     // users newly misinformed (spread next tick)
    std::queue<int> factQ_;    // users newly corrected   (spread next tick)
    int tick_ = 0, everInfected_ = 0, currentMis_ = 0, corrected_ = 0, direct_ = 0;
public:
    CascadeModel(const SocialGraph& g, double misProb, double factProb);
    bool seedMisinformation(int source);        // tick 0
    bool correctUser(int user);                 // moderator intervention
    TickReport step(uint64_t seed);             // one synchronous tick
    UserState stateOf(int user) const;
    int  currentTick() const;
    int  misinformationReach() const;           // ever misinformed
    int  currentMisinformed() const;
    int  factCheckReach() const;                // direct + propagated
    int  directCorrections() const;
    int  propagatedCorrections() const;
    bool misinformationActive() const;
    bool isQuiescent() const;                   // both frontiers empty
};
#endif
