#ifndef CONFIG_H
#define CONFIG_H
#include <cstdint>
struct SimulationConfig {
    int      users    = 15;
    double   edgeProb = 0.40;
    double   misProb  = 0.70;
    double   factProb = 0.50;
    int      ticks    = 15;
    int      warmup   = 3;
    int      budget   = 3;
    int      source   = -1;     // -1 = random
    uint64_t seed     = 1001;
    int      mcTrials = 100;
    int      runs     = 30;
};
#endif
