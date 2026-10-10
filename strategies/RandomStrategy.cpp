#include "Strategies.h"
int RandomStrategy::selectCandidate(const InterventionContext& ctx) {
    scores_.clear();
    if (ctx.candidates.empty()) return -1;
    std::uniform_int_distribution<size_t> d(0, ctx.candidates.size() - 1);
    return ctx.candidates[d(rng_)];
}
