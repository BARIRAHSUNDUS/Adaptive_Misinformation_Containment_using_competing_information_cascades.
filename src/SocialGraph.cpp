#include "SocialGraph.h"
#include <algorithm>
#include <queue>

int SocialGraph::addUser() {
    int id = (int)users.size();
    users.push_back(User(id));
    adjacencyList.emplace_back();
    return id;
}

bool SocialGraph::isValid(int u) const { return u >= 0 && u < (int)users.size(); }

bool SocialGraph::addConnection(int u, int v) {
    if (!isValid(u) || !isValid(v) || u == v) return false;
    const auto& a = adjacencyList[u];
    if (std::find(a.begin(), a.end(), v) != a.end()) return false;
    adjacencyList[u].push_back(v);
    adjacencyList[v].push_back(u);
    ++edgeCount;
    return true;
}

const std::vector<int>& SocialGraph::getNeighbours(int u) const {
    static const std::vector<int> empty;
    return isValid(u) ? adjacencyList[u] : empty;
}

int SocialGraph::getDegree(int u) const { return isValid(u) ? (int)adjacencyList[u].size() : -1; }

int SocialGraph::getMaxDegreeUser() const {
    if (users.empty()) return -1;
    int best = 0;
    for (int i = 1; i < (int)adjacencyList.size(); i++)
        if (adjacencyList[i].size() > adjacencyList[best].size()) best = i;
    return best;
}

int SocialGraph::numUsers() const { return (int)users.size(); }
int SocialGraph::numEdges() const { return edgeCount; }

std::vector<int> SocialGraph::BFS(int start) const {
    std::vector<int> order;
    if (!isValid(start)) return order;
    std::vector<char> seen(users.size(), 0);
    std::queue<int> q;
    seen[start] = 1; q.push(start);
    while (!q.empty()) {
        int cur = q.front(); q.pop();
        order.push_back(cur);
        for (int nb : adjacencyList[cur])
            if (!seen[nb]) { seen[nb] = 1; q.push(nb); }
    }
    return order;
}

int SocialGraph::countComponents() const {
    std::vector<char> seen(users.size(), 0);
    int comps = 0;
    for (int i = 0; i < (int)users.size(); i++) {
        if (seen[i]) continue;
        ++comps;
        for (int v : BFS(i)) seen[v] = 1;
    }
    return comps;
}

int SocialGraph::countIsolated() const {
    int c = 0;
    for (const auto& a : adjacencyList) if (a.empty()) ++c;
    return c;
}

void SocialGraph::displayGraph(std::ostream& os) const {
    os << "Adjacency list:\n";
    for (int i = 0; i < (int)adjacencyList.size(); i++) {
        os << "  " << i << " -> ";
        if (adjacencyList[i].empty()) os << "(none)";
        for (int nb : adjacencyList[i]) os << nb << ' ';
        os << '\n';
    }
}

SocialGraph SocialGraph::generateRandom(int n, double p, std::mt19937& rng) {
    SocialGraph g;
    for (int i = 0; i < n; i++) g.addUser();
    std::uniform_real_distribution<double> d(0.0, 1.0);
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (d(rng) < p) g.addConnection(i, j);
    return g;
}

SocialGraph SocialGraph::generateScaleFree(int n, int m, std::mt19937& rng) {
    SocialGraph g;
    for (int i = 0; i < n; i++) g.addUser();
    if (n <= 1 || m < 1) return g;
    if (m > n - 1) m = n - 1;
    std::vector<int> bag;                       // every user appears once per connection it has
    const int m0 = m + 1;                       // initial clique (m0 <= n because m <= n-1)
    for (int i = 0; i < m0; i++)
        for (int j = i + 1; j < m0; j++) { g.addConnection(i, j); bag.push_back(i); bag.push_back(j); }
    for (int v = m0; v < n; v++) {
        std::vector<int> targets;
        while ((int)targets.size() < m) {       // terminates: at least m distinct users are in the bag
            std::uniform_int_distribution<size_t> d(0, bag.size() - 1);
            int t = bag[d(rng)];
            if (std::find(targets.begin(), targets.end(), t) == targets.end()) targets.push_back(t);
        }
        for (int t : targets) { g.addConnection(v, t); bag.push_back(v); bag.push_back(t); }
    }
    return g;
}