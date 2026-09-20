#include "SimulationEngine.h"
#include <iostream>

using namespace std;


// Create intervention schedule

void SimulationEngine::createInterventionSchedule()
{
    interventionTicks.clear();

    int totalBudget = budget.getTotalBudget();

    if (totalBudget <= 0)
    {
        return;
    }

    if (maxTicks <= warmUpTicks)
    {
        return;
    }

    int span = maxTicks - warmUpTicks;

    int interval = span / totalBudget;

    if (interval <= 0)
    {
        interval = 1;
    }

    for (int i = 1;
         i <= totalBudget;
         i++)
    {
        int tick =
            warmUpTicks + (i * interval);

        if (tick <= maxTicks)
        {
            interventionTicks.push_back(tick);
        }
    }
}


// Check intervention tick

bool SimulationEngine::isInterventionTick(int tick)
{
    for (int i = 0;
         i < interventionTicks.size();
         i++)
    {
        if (interventionTicks[i] == tick)
        {
            return true;
        }
    }

    return false;
}


// Constructor

SimulationEngine::SimulationEngine(
    int maxTicksValue,
    int warmUpTicksValue,
    int budgetValue,
    CascadeModel* cascadeModel)
    : budget(budgetValue)
{
    maxTicks = maxTicksValue;

    warmUpTicks = warmUpTicksValue;

    cascade = cascadeModel;

    if (maxTicks < 0)
    {
        maxTicks = 0;
    }

    if (warmUpTicks < 0)
    {
        warmUpTicks = 0;
    }

    if (warmUpTicks > maxTicks)
    {
        warmUpTicks = maxTicks;
    }

    createInterventionSchedule();
}


// Display intervention schedule

void SimulationEngine::showInterventionSchedule()
{
    cout << "\n===== INTERVENTION SCHEDULE =====\n";

    if (interventionTicks.empty())
    {
        cout << "No intervention ticks scheduled.\n";
        return;
    }

    for (int i = 0;
         i < interventionTicks.size();
         i++)
    {
        cout << "Intervention at Tick "
             << interventionTicks[i]
             << endl;
    }
}


// Run simulation

void SimulationEngine::run()
{
    cout << "\n===== SIMULATION STARTED =====\n";

    for (int tick = 1;
         tick <= maxTicks;
         tick++)
    {
        cout << "\nTick "
             << tick
             << endl;


        // One propagation level
        // is processed in one tick

        int newlyInfected =
            cascade->processTick();


        cout << "New users infected: "
             << newlyInfected
             << endl;


        cout << "Total misinformation reach: "
             << cascade->getMisinformationCount()
             << endl;


        // Check intervention

        if (isInterventionTick(tick))
        {
            cout << "*** INTERVENTION TICK ***"
                 << endl;


            if (budget.canIntervene())
            {
                cout << "Intervention budget available."
                     << endl;


                /*
                   Strategy selection will be
                   connected here.

                   Example:

                   int selectedUser =
                       strategy.selectUser(...);

                   cascade->factCheck(selectedUser);

                   budget.useIntervention();
                */
            }
            else
            {
                cout << "Budget exhausted."
                     << endl;
            }
        }
    }


    cout << "\n===== SIMULATION COMPLETED =====\n";

    cout << "Final misinformation reach: "
         << cascade->getMisinformationCount()
         << endl;

    cout << "Remaining intervention budget: "
         << budget.getRemainingBudget()
         << endl;
}
