#ifndef INTERVENTIONSTRATEGY_H
#define INTERVENTIONSTRATEGY_H
#include <vector>
#include <string>
#include <utility>
#include <cstdint>
#include "SocialGraph.h"
#include "CascadeModel.h"


struct InterventionContext {
    const SocialGraph&  graph;
    const CascadeModel& cascade;
    int tick;                            
    int totalTicks;
    int interventionIndex;                 
    uint64_t seed;                          
    const std::vector<int>& candidates;     
};

class InterventionStrategy {
protected:
    std::vector<std::pair<int, double>> scores_;   
public:
    virtual ~InterventionStrategy() = default;
    virtual int selectCandidate(const InterventionContext& ctx) = 0; 
    virtual std::string name() const = 0;
    virtual void reset(uint64_t /*seed*/) {}
    const std::vector<std::pair<int, double>>& lastScores() const { return scores_; }
};
#endif
