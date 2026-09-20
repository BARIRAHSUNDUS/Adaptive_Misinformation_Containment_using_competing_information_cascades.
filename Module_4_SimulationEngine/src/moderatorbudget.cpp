#include "ModeratorBudget.h"

ModeratorBudget::ModeratorBudget(int budget)
{
    if (budget < 0)
    {
        budget = 0;
    }

    totalBudget = budget;
    remainingBudget = budget;
}

bool ModeratorBudget::canIntervene()
{
    return remainingBudget > 0;
}

bool ModeratorBudget::useIntervention()
{
    if (remainingBudget <= 0)
    {
        return false;
    }

    remainingBudget--;

    return true;
}

int ModeratorBudget::getTotalBudget()
{
    return totalBudget;
}

int ModeratorBudget::getRemainingBudget()
{
    return remainingBudget;
}
