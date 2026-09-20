#ifndef CASCADEMODEL_H
#define CASCADEMODEL_H

#include <vector>
#include "UserState.h"

using namespace std;

class CascadeModel
{
private:
    vector<vector<int>> graph;
    vector<int> status;

public:
    CascadeModel(vector<vector<int>> g, int n);

    int spreadMisinformation(
        int source,
        double spreadProbability
    );

    void factCheck(int user);

    int spreadFactCheck(int source);

    void showStatus();
};

#endif
