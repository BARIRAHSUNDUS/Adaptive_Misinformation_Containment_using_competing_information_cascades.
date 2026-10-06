#ifndef SOCIALGRAPH_H
#define SOCIALGRAPH_H
#include <vector>
#include <random>
#include <ostream>
#include "User.h"

class SocialGraph {
private:
    std::vector<User> users;
    std::vector<std::vector<int>> adjacencyList;
    int edgeCount = 0;
public:
    int  addUser();                              // returns new user's id (== index)
    bool addConnection(int u, int v);            // false for invalid/self-loop/duplicate
    bool isValid(int user) const;
    const std::vector<int>& getNeighbours(int user) const;
    int  getDegree(int user) const;              // -1 if invalid
    int  getMaxDegreeUser() const;               // ties -> smaller id, -1 if empty
    int  numUsers() const;
    int  numEdges() const;
    std::vector<int> BFS(int startUser) const;   // visit order
    int  countComponents() const;
    int  countIsolated() const;
    void displayGraph(std::ostream& os) const;
    // Evaluates EVERY unordered pair once: edge iff uniform[0,1) < p.
    static SocialGraph generateRandom(int n, double p, std::mt19937& rng);
};
#endif