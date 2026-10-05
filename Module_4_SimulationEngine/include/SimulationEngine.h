#include <vector>
#include "ModeratorBudget.h"
#include "CascadeModel.h"

using namespace std;

class SimulationEngine
{
private:
    int mticks;
    int warmup;
    
    ModeratorBudget budget;
    CascadeModel* model;

    vector<int> intervention;

    void makeSchedule();
    bool checkTick(int t);

public:
    SimulationEngine(
        int mticks,
        int warmup,
        int budget,
        CascadeModel* c
    );

    void showSchedule();
    void run();
};