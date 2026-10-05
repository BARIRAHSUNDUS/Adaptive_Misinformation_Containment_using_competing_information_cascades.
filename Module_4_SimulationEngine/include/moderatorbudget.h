class ModeratorBudget
{
private:
    int Tb;
    int Rb;

public:
    ModeratorBudget(int b);

    bool canIntervene();
    bool useIntervention();

    int getTb();
    int getRb();
};