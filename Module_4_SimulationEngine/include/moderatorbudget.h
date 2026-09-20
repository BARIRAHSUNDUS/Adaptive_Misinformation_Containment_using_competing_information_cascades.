class ModeratorBudget
{
private:
    int totalBudget;
    int remainingBudget;

public:
    ModeratorBudget(int budget);

    bool canIntervene();
    bool useIntervention();

    int getTotalBudget();
    int getRemainingBudget();
};
