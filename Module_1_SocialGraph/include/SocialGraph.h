#pragma once

#include <vector>
#include "User.h"

class SocialGraph
{
private:
    std::vector<User> users;
    std::vector<std::vector<int>> adjacencyList;

public:

    void addUser(int id);

    void addConnection(int u, int v);

    void displayGraph();

    void getNeighbours(int user);

    int getDegree(int user);

    int getMaxDegreeUser();

    void BFS(int startUser);
};
