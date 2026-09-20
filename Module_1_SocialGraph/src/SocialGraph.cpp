#include "SocialGraph.h"

#include <iostream>
#include <queue>
#include <algorithm>

using namespace std;


// Add a user
void SocialGraph::addUser(int id)
{
    users.push_back(User(id));
    adjacencyList.push_back({});
}


// Add connection between two users
void SocialGraph::addConnection(int u, int v)
{
    // Check valid users
    if (u < 0 || v < 0 ||
        u >= (int)users.size() ||
        v >= (int)users.size())
    {
        return;
    }

    // Do not allow self connection
    if (u == v)
    {
        return;
    }

    // Check if connection already exists
    if (find(adjacencyList[u].begin(),
             adjacencyList[u].end(),
             v) != adjacencyList[u].end())
    {
        return;
    }

    // Undirected graph
    adjacencyList[u].push_back(v);
    adjacencyList[v].push_back(u);
}


// Display complete graph
void SocialGraph::displayGraph()
{
    cout << "\n========================================\n";
    cout << "           GENERATED SOCIAL GRAPH\n";
    cout << "========================================\n";

    for (int i = 0;
         i < (int)adjacencyList.size();
         i++)
    {
        cout << "User " << i << " -> ";

        if (adjacencyList[i].empty())
        {
            cout << "No connections";
        }
        else
        {
            for (int neighbour : adjacencyList[i])
            {
                cout << neighbour << " ";
            }
        }

        cout << endl;
    }
}


// Show neighbours of a user
void SocialGraph::getNeighbours(int user)
{
    if (user < 0 ||
        user >= (int)users.size())
    {
        cout << "Invalid user ID.\n";
        return;
    }

    cout << "\nNeighbours of User "
         << user << " = ";

    if (adjacencyList[user].empty())
    {
        cout << "No connections";
    }
    else
    {
        for (int neighbour : adjacencyList[user])
        {
            cout << neighbour << " ";
        }
    }

    cout << endl;
}


// Find degree of a user
int SocialGraph::getDegree(int user)
{
    if (user < 0 ||
        user >= (int)users.size())
    {
        return -1;
    }

    return (int)adjacencyList[user].size();
}


// Find user with maximum degree
int SocialGraph::getMaxDegreeUser()
{
    if (users.empty())
    {
        return -1;
    }

    int maxUser = 0;
    int maxDegree = adjacencyList[0].size();

    for (int i = 1;
         i < (int)adjacencyList.size();
         i++)
    {
        int degree = adjacencyList[i].size();

        if (degree > maxDegree)
        {
            maxDegree = degree;
            maxUser = i;
        }
    }

    return maxUser;
}


// BFS traversal
void SocialGraph::BFS(int startUser)
{
    if (startUser < 0 ||
        startUser >= (int)users.size())
    {
        cout << "Invalid starting user.\n";
        return;
    }

    vector<bool> visited(
        users.size(),
        false
    );

    queue<int> q;

    visited[startUser] = true;
    q.push(startUser);

    cout << "\nBFS Traversal starting from User "
         << startUser << " = ";

    while (!q.empty())
    {
        int current = q.front();
        q.pop();

        cout << current << " ";

        for (int neighbour :
             adjacencyList[current])
        {
            if (!visited[neighbour])
            {
                visited[neighbour] = true;
                q.push(neighbour);
            }
        }
    }

    cout << endl;
}
