#ifndef CASCADEMODEL_H
#define CASCADEMODEL_H
#include <vector>
#include <queue>
#include <cstdint>
#include "SocialGraph.h"
#include "UserState.h"

struct TickReport { int newMisinformed = 0; int newCorrected = 0; };
struct Transmission { int user; int from; };
struct TickEvents { std::vector<Transmission> misinformed, corrected; };
class CascadeModel {
private:
    const SocialGraph* graph_;
    double pMis_, pFact_;
    std::vector<UserState> state_;
    std::queue<int> misQ_;     
    std::queue<int> factQ_;   
    int tick_ = 0, everInfected_ = 0, currentMis_ = 0, corrected_ = 0, direct_ = 0;
public:
    CascadeModel(const SocialGraph& g, double misProb, double factProb);
    bool seedMisinformation(int source);       
    bool correctUser(int user);              
    
    TickReport step(uint64_t seed, TickEvents* events = nullptr);
    UserState stateOf(int user) const;
    int  currentTick() const;
    int  misinformationReach() const;           
    int  currentMisinformed() const;
    int  factCheckReach() const;                
    int  directCorrections() const;
    int  propagatedCorrections() const;
    bool misinformationActive() const;         y
   
    std::vector<int> misinformationFrontier() const;
    
    std::vector<int> eligibleUsers() const;
    bool isQuiescent() const;                 
};
#endif
