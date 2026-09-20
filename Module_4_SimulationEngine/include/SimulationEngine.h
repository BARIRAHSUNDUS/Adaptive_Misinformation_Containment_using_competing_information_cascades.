#include <vector>
#include "ModeratorBudget.h"
#include "CascadeModel.h"

using namespace std;

class SimulationEngine
{
private:
    int maxTicks;
    int warmUpTicks;

    ModeratorBudget budget;

    CascadeModel* cascade;

    vector<int> interventionTicks;

    void createInterventionSchedule();

    bool isInterventionTick(int tick);

public:

    SimulationEngine(
        int maxTicks,
        int warmUpTicks,
        int budget,
        CascadeModel* cascadeModel
    );

    void showInterventionSchedule();

    void run();
};
