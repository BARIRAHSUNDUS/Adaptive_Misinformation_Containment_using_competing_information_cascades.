#include "Strategies.h"
// O(C) per intervention. Ties -> smaller user id.
int HighestDegreeStrategy::selectCandidate(const InterventionContext& ctx) {
    scores_.clear();
    int best = -1, bestDeg = -1;
    for (int u : ctx.candidates) {
        int d = ctx.graph.getDegree(u);
        scores_.push_back({u, (double)d});
        if (d > bestDeg || (d == bestDeg && u < best)) { bestDeg = d; best = u; }
    }
    return best;
}
