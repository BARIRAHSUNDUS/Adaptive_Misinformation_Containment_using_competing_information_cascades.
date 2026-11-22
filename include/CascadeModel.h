#ifndef CASCADEMODEL_H
#define CASCADEMODEL_H
#include <vector>
#include <queue>
#include <cstdint>
#include "SocialGraph.h"
#include "UserState.h"

struct TickReport { int newMisinformed = 0; int newCorrected = 0; };

// One successful transmission: `user` changed state because of `from` (-1 = no user: the source / the moderator).
struct Transmission { int user; int from; };
// Everything that happened during one propagation round (used by the GUI to animate it).
struct TickEvents { std::vector<Transmission> misinformed, corrected; };

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
    // One synchronous tick. If `events` is given, every successful transmission is recorded in it.
    // (Marginal Gain's hypothetical futures call step() without it, so they stay fast.)
    TickReport step(uint64_t seed, TickEvents* events = nullptr);
    UserState stateOf(int user) const;
    int  currentTick() const;
    int  misinformationReach() const;           // ever misinformed
    int  currentMisinformed() const;
    int  factCheckReach() const;                // direct + propagated
    int  directCorrections() const;
    int  propagatedCorrections() const;
    bool misinformationActive() const;          // the rumour can still spread: the frontier below is not empty
    // MISINFORMED users that have NOT spread yet (they spread in the next tick). Correcting one of
    // them still stops its spread; correcting a user that already spread cannot lower the reach.
    std::vector<int> misinformationFrontier() const;
    // The candidate pool: the users the moderator MAY fact-check right now.
    //   - NEUTRAL users (a corrected user passes the correction on, and cannot be misinformed any more)
    //   - MISINFORMED users that have not spread yet (correcting them stops their spread)
    // NOT eligible: CORRECTED users (nothing to gain) and MISINFORMED users that already spread (too late).
    // The pool is deliberately simple: it also contains neutral users the rumour may never reach (for example in a
    // separate part of the network). Telling the useful ones from the useless ones is the job of the strategy.
    // Whether a fact-check can still help at all is decided by SimulationEngine::run (rumour alive and a tick left).
    // Every strategy is offered exactly this list.
    std::vector<int> eligibleUsers() const;
    bool isQuiescent() const;                   // both frontiers empty
};
#endif
