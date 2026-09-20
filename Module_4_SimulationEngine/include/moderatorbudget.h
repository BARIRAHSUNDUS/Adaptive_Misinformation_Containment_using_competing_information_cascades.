class moderatorbudget
{
private:
    int totalBudget;
    int remainingBudget;

public:
    moderatorbudget(int budget);

    bool canIntervene();
    bool useIntervention();

    int getTotalBudget();
    int getRemainingBudget();
};
