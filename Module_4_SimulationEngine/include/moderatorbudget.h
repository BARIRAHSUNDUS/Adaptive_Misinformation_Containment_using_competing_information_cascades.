class ModeratorBudget
{
private:
    int TotalBudget;
    int RemainingBudget;

public:
    ModeratorBudget(int budget);

    bool canIntervene();
    bool useIntervention();

    int getTotalBudget();
    int getRemainigBudget();
};