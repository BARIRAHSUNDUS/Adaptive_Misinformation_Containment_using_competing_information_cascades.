#include "ModeratorBudget.h"
ModeratorBudget::ModeratorBudget(int budget) {
    if (budget < 0) budget = 0;
    Tb = budget; Rb = budget;
}
bool ModeratorBudget::canIntervene() const { return Rb > 0; }
bool ModeratorBudget::useIntervention() { if (Rb > 0) { --Rb; return true; } return false; }
int  ModeratorBudget::getTb() const { return Tb; }
int  ModeratorBudget::getRb() const { return Rb; }
