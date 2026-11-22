#ifndef CONFIG_H
#define CONFIG_H
#include <cstdint>

// Which random-graph model builds the network.
enum class GraphType { ErdosRenyi, ScaleFree };
// Defaults are chosen so that the rumour is still spreading when the moderator starts acting
// (a sparse network, moderate spread probability, early start). With the old defaults
// (15 users, edgeProb 0.4, mis 0.7) everybody was misinformed before the first intervention.
// Ranges the three probabilities are drawn from (once per run / experiment) when they are randomized.
const double EDGE_PROB_MIN = 0.02, EDGE_PROB_MAX = 0.08;    // connection probability (random graph)
const double MIS_PROB_MIN  = 0.20, MIS_PROB_MAX  = 0.60;    // misinformation spread probability
const double FACT_PROB_MIN = 0.40, FACT_PROB_MAX = 0.80;    // fact-check spread probability

struct SimulationConfig {
    int      users    = 100;
    double   edgeProb = 0.04;
    double   misProb  = 0.40;
    double   factProb = 0.60;
    int      ticks    = 25;
    int      warmup   = 1;
    int      budget   = 10;
    int      source   = -1;     // -1 = random
    uint64_t seed     = 1001;
    int      mcTrials = 100;
    int      runs     = 30;
    // true  = draw this probability at random (inside the range above) for every run / experiment
    // false = use the value in edgeProb / misProb / factProb (set automatically when --edgeprob etc. is given)
    bool     randEdge = true;
    bool     randMis  = true;
    bool     randFact = true;
    GraphType     graph      = GraphType::ErdosRenyi;
    int           baM        = 2;                    // edges per new user in the scale-free graph
};
#endif
