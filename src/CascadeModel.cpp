#include "CascadeModel.h"
#include <algorithm>

namespace {
double clamp01(double x) { return std::min(1.0, std::max(0.0, x)); }

// Scrambles the bits of a number so that similar inputs give unrelated outputs (a standard "hash").
uint64_t mix(uint64_t x) {
    x += 0x9E3779B97F4A7C15ULL;
    x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9ULL;
    x = (x ^ (x >> 27)) * 0x94D049BB133111EBULL;
    return x ^ (x >> 31);
}

// "Random" number in [0,1) for the attempt  from -> to  at a given tick.
// WHY NOT std::mt19937? A normal generator gives different numbers if even one earlier attempt
// was skipped. Here the number depends ONLY on (seed, tick, from, to), so two simulations with
// the same seed behave identically except where the intervention really changes something.
// That makes "with intervention" vs "without intervention" a fair comparison.
double randomValue(uint64_t seed, int tick, int from, int to) {
    uint64_t h = mix(seed);
    h = mix(h ^ (uint64_t)tick);
    h = mix(h ^ (((uint64_t)(uint32_t)from << 32) | (uint32_t)to));
    return (double)(h >> 11) / 9007199254740992.0;      // 53 random bits -> [0,1)
}
}

CascadeModel::CascadeModel(const SocialGraph& g, double misProb, double factProb)
    : graph_(&g), pMis_(clamp01(misProb)), pFact_(clamp01(factProb)),
      state_(g.numUsers(), UserState::NEUTRAL) {}

bool CascadeModel::seedMisinformation(int s) {
    if (!graph_->isValid(s) || state_[s] != UserState::NEUTRAL) return false;
    state_[s] = UserState::MISINFORMED;
    ++everInfected_; ++currentMis_;
    misQ_.push(s);
    return true;
}

bool CascadeModel::correctUser(int u) {
    if (!graph_->isValid(u) || state_[u] == UserState::CORRECTED) return false;
    if (state_[u] == UserState::MISINFORMED) --currentMis_;
    state_[u] = UserState::CORRECTED;
    ++corrected_; ++direct_;
    factQ_.push(u);
    return true;
}

TickReport CascadeModel::step(uint64_t seed) {
    ++tick_;
    const int n = graph_->numUsers();
    std::queue<int> curMis, curFact;
    curMis.swap(misQ_);
    curFact.swap(factQ_);
    std::vector<char> markMis(n, 0), markCor(n, 0);
    std::vector<int> misList, corList;

    // All decisions use the state at the START of the tick (synchronous).
    while (!curMis.empty()) {
        int u = curMis.front(); curMis.pop();
        if (state_[u] != UserState::MISINFORMED) continue;      // corrected since being queued
        for (int v : graph_->getNeighbours(u)) {
            if (state_[v] != UserState::NEUTRAL || markMis[v]) continue;
            if (randomValue(seed, tick_, u, v) < pMis_) { markMis[v] = 1; misList.push_back(v); }
        }
    }
    while (!curFact.empty()) {
        int u = curFact.front(); curFact.pop();
        for (int v : graph_->getNeighbours(u)) {
            if (state_[v] == UserState::CORRECTED || markCor[v]) continue;
            if (randomValue(seed, tick_, u, v) < pFact_) { markCor[v] = 1; corList.push_back(v); }
        }
    }

    TickReport rep;
    // Fact-check wins conflicts: apply corrections first, then misinformation to the rest.
    for (int v : corList) {
        if (state_[v] == UserState::MISINFORMED) --currentMis_;
        state_[v] = UserState::CORRECTED;
        ++corrected_; ++rep.newCorrected;
        factQ_.push(v);
    }
    for (int v : misList) {
        if (markCor[v]) continue;
        state_[v] = UserState::MISINFORMED;
        ++everInfected_; ++currentMis_; ++rep.newMisinformed;
        misQ_.push(v);
    }
    return rep;
}

UserState CascadeModel::stateOf(int u) const { return graph_->isValid(u) ? state_[u] : UserState::NEUTRAL; }
int  CascadeModel::currentTick() const { return tick_; }
int  CascadeModel::misinformationReach() const { return everInfected_; }
int  CascadeModel::currentMisinformed() const { return currentMis_; }
int  CascadeModel::factCheckReach() const { return corrected_; }
int  CascadeModel::directCorrections() const { return direct_; }
int  CascadeModel::propagatedCorrections() const { return corrected_ - direct_; }
bool CascadeModel::misinformationActive() const { return !misQ_.empty(); }
bool CascadeModel::isQuiescent() const { return misQ_.empty() && factQ_.empty(); }
