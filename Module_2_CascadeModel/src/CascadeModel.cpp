#include "../include/CascadeModel.h"

#include <iostream>
#include <queue>
#include <cstdlib>

using namespace std;

CascadeModel::CascadeModel(vector<vector<int>> g, int n)
{
    graph = g;
    status.resize(n, NORMAL);
}

// Spread misinformation using BFS
int CascadeModel::spreadMisinformation(
    int source,
    double spreadProbability)
{
    queue<int> q;

    int reachedUsers = 0;

    status[source] = MISINFORMATION;

    q.push(source);

    reachedUsers++;

    while (!q.empty())
    {
        int currentUser = q.front();
        q.pop();

        for (int nextUser : graph[currentUser])
        {
            // Do not spread misinformation to fact-checked users
            if (status[nextUser] == FACT_CHECKED)
                continue;

            // Spread only to normal users
            if (status[nextUser] == NORMAL)
            {
                double randomValue =
                    (double)rand() / (double)RAND_MAX;

                if (randomValue < spreadProbability)
                {
                    status[nextUser] = MISINFORMATION;

                    q.push(nextUser);

                    reachedUsers++;
                }
            }
        }
    }

    return reachedUsers;
}

// Fact-check a particular user
void CascadeModel::factCheck(int user)
{
    status[user] = FACT_CHECKED;
}

// Spread fact-check information using BFS
int CascadeModel::spreadFactCheck(int source)
{
    queue<int> q;

    vector<bool> visited(status.size(), false);

    q.push(source);

    visited[source] = true;

    int reachedUsers = 0;

    while (!q.empty())
    {
        int currentUser = q.front();

        q.pop();

        reachedUsers++;

        for (int nextUser : graph[currentUser])
        {
            if (!visited[nextUser])
            {
                visited[nextUser] = true;

                q.push(nextUser);
            }
        }
    }

    return reachedUsers;
}

// Display current status of all users
void CascadeModel::showStatus()
{
    cout << "\n----- Current User Status -----\n";

    for (int i = 0; i < status.size(); i++)
    {
        cout << "User " << i << ": ";

        if (status[i] == NORMAL)
            cout << "Normal";

        else if (status[i] == MISINFORMATION)
            cout << "Misinformation";

        else if (status[i] == FACT_CHECKED)
            cout << "Fact-Checked";

        cout << endl;
    }
}
