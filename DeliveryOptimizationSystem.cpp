/**
 * =============================================================================
 *  Delivery Optimization System
 * =============================================================================
 *  Algorithms included:
 *    1. Package Management  (add, view)
 *    2. 0/1 Knapsack DP     (optimal vehicle load)
 *    3. Dijkstra            (shortest delivery routes)
 *    4. Kruskal + UnionFind (minimum spanning tree)
 *    5. Quick Sort          (sort packages)
 * =============================================================================
 */

using namespace std;

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <limits>
#include <queue>
#include <algorithm>
#include <functional>
#include <stdexcept>
#include <sstream>

// ─────────────────────────────────────────────────────────────────────────────
//  SECTION 1 — PACKAGE
//  A delivery item with an ID, weight (kg), and profit ($).
// ─────────────────────────────────────────────────────────────────────────────

struct Package
{
    string id;
    double weight; // kg
    double profit; // $

    Package(const string &i, double w, double p)
        : id(i), weight(w), profit(p)
    {
        if (w < 0)
            throw invalid_argument("Weight cannot be negative.");
        if (p < 0)
            throw invalid_argument("Profit cannot be negative.");
    }

    double ratio() const
    {
        return (weight == 0.0) ? 0.0 : profit / weight;
    }
};

// ── PackageManager

class PackageManager
{
public:
    // Add a package, reject duplicate IDs
    bool add(const Package &pkg)
    {
        for (const auto &p : packages_)
            if (p.id == pkg.id)
            {
                cout << "  [!] ID '" << pkg.id << "' already exists.\n";
                return false;
            }
        packages_.push_back(pkg);
        return true;
    }

    // Print all packages
    void printAll() const
    {
        if (packages_.empty())
        {
            cout << "  No packages in the system.\n";
            return;
        }
        printSep();
        cout << "| " << left << setw(10) << "ID"
             << " | " << right << setw(10) << "Weight(kg)"
             << " | " << right << setw(10) << "Profit($)"
             << " | " << right << setw(10) << "Ratio($/kg)"
             << " |\n";
        printSep();
        for (const auto &p : packages_)
        {
            cout << "| " << left << setw(10) << p.id
                 << " | " << right << fixed << setprecision(2)
                 << setw(10) << p.weight
                 << " | " << setw(10) << p.profit
                 << " | " << setw(10) << p.ratio()
                 << " |\n";
        }
        printSep();
        cout << "  Total: " << packages_.size() << " package(s)\n\n";
    }

    const vector<Package> &all() const { return packages_; }
    size_t count() const { return packages_.size(); }

private:
    vector<Package> packages_;

    void printSep() const
    {
        cout << "+-" << string(10, '-') << "-+-"
             << string(10, '-') << "-+-"
             << string(10, '-') << "-+-"
             << string(10, '-') << "-+\n";
    }
};

// ─────────────────────────────────────────────────────────────────────────────
//  SECTION 2 — 0/1 KNAPSACK
// ─────────────────────────────────────────────────────────────────────────────

struct KnapsackResult
{
    vector<Package> selected;
    double totalWeight = 0.0;
    double totalProfit = 0.0;
};

KnapsackResult solveKnapsack(const vector<Package> &pkgs, double capacityKg)
{
    KnapsackResult result;
    if (pkgs.empty())
        return result;

    const int n = static_cast<int>(pkgs.size());

    // Convert weights to integers (precision: 2 decimal places → units of 10g)
    const int SCALE = 100;
    const int W = static_cast<int>(capacityKg * SCALE);

    vector<int> wi(n);
    vector<int> pi(n);
    for (int i = 0; i < n; ++i)
    {
        wi[i] = static_cast<int>(pkgs[i].weight * SCALE);
        pi[i] = static_cast<int>(pkgs[i].profit * SCALE);
    }

    // ── Build DP table ────────────────────────────────────────────────────────
    vector<vector<int>> dp(n + 1, vector<int>(W + 1, 0));

    for (int i = 1; i <= n; ++i)
    {
        for (int w = 0; w <= W; ++w)
        {
            dp[i][w] = dp[i - 1][w]; // default: skip
            if (wi[i - 1] <= w)
            {                                                    // package fits
                int take = dp[i - 1][w - wi[i - 1]] + pi[i - 1]; // take it
                dp[i][w] = max(dp[i][w], take);
            }
        }
    }

    // ── Backtrack to find selected packages ───────────────────────────────────
    int w = W;
    for (int i = n; i >= 1; --i)
    {
        if (dp[i][w] != dp[i - 1][w])
        { // value changed → package taken
            result.selected.push_back(pkgs[i - 1]);
            result.totalWeight += pkgs[i - 1].weight;
            result.totalProfit += pkgs[i - 1].profit;
            w -= wi[i - 1];
        }
    }
    reverse(result.selected.begin(), result.selected.end());
    return result;
}

void printKnapsackResult(const KnapsackResult &r)
{
    cout << "\n  OPTIMAL LOAD (0/1 Knapsack DP)\n";
    cout << "  " << string(52, '-') << "\n";

    if (r.selected.empty())
    {
        cout << "  No packages fit within capacity.\n\n";
        return;
    }

    cout << "  " << left << setw(3) << "#"
         << setw(12) << "ID"
         << right << setw(12) << "Weight(kg)"
         << setw(12) << "Profit($)" << "\n";
    cout << "  " << string(52, '-') << "\n";

    for (size_t i = 0; i < r.selected.size(); ++i)
    {
        const auto &p = r.selected[i];
        cout << "  " << left << setw(3) << (i + 1)
             << setw(12) << p.id
             << right << fixed << setprecision(2)
             << setw(12) << p.weight
             << setw(12) << p.profit << "\n";
    }

    cout << "  " << string(52, '-') << "\n";
    cout << "  Total weight : " << fixed << setprecision(2)
         << r.totalWeight << " kg\n";
    cout << "  Total profit : $" << r.totalProfit << "\n\n";
}

// ─────────────────────────────────────────────────────────────────────────────
//  SECTION 3 — GRAPH + DIJKSTRA
// ─────────────────────────────────────────────────────────────────────────────

const double INF = numeric_limits<double>::infinity();

// Represents an edge from one node to another
struct Neighbor
{
    int node;    // destination node index
    double cost; // edge weight
};

struct Graph
{
    int numNodes;
    vector<string> names;
    vector<vector<Neighbor>> adjList; // adjacency list

    // Constructor
    Graph(int n)
    {
        numNodes = n;
        names.resize(n);
        adjList.resize(n);

        for (int i = 0; i < n; i++)
        {
            names[i] = "Node-" + to_string(i);
        }
    }

    // Set custom name for a node
    void setName(int index, const string &name)
    {
        names[index] = name;
    }

    // Add directed edge
    void addEdge(int from, int to, double cost)
    {
        Neighbor edge;
        edge.node = to;
        edge.cost = cost;
        adjList[from].push_back(edge);
    }

    // Add undirected edge
    void addBiEdge(int u, int v, double cost)
    {
        addEdge(u, v, cost);
        addEdge(v, u, cost);
    }

    // ── DIJKSTRA ──────────────────────────────────────────────────────────────
    void dijkstra(int source,
                  vector<double> &dist,
                  vector<int> &parent) const
    {
        // Initialize
        dist.clear();
        parent.clear();

        dist.resize(numNodes, INF);
        parent.resize(numNodes, -1);

        dist[source] = 0.0;

        // Min-heap: (distance, node)
        priority_queue<pair<double, int>,
                       vector<pair<double, int>>,
                       greater<pair<double, int>>>
            pq;

        pq.push(make_pair(0.0, source));

        while (!pq.empty())
        {

            pair<double, int> top = pq.top();
            pq.pop();

            double currentDist = top.first;
            int currentNode = top.second;

            // Skip outdated entries
            if (currentDist > dist[currentNode])
                continue;

            // Explore neighbors
            for (const Neighbor &edge : adjList[currentNode])
            {

                int nextNode = edge.node;
                double weight = edge.cost;

                double newDistance = dist[currentNode] + weight;

                // Relaxation
                if (newDistance < dist[nextNode])
                {
                    dist[nextNode] = newDistance;
                    parent[nextNode] = currentNode;

                    pq.push(make_pair(newDistance, nextNode));
                }
            }
        }
    }

    // Reconstruct path from source to destination
    vector<int> getPath(const vector<int> &parent,
                        int source,
                        int destination) const
    {
        vector<int> path;

        int current = destination;

        while (current != -1)
        {
            path.push_back(current);
            current = parent[current];
        }

        reverse(path.begin(), path.end());

        // Check if valid path
        if (path.empty() || path[0] != source)
            return {};

        return path;
    }

    // Print Dijkstra result
    void printDijkstra(int source) const
    {
        vector<double> dist;
        vector<int> parent;

        dijkstra(source, dist, parent);

        cout << "\n  SHORTEST ROUTES from \"" << names[source] << "\"\n";
        cout << "  " << string(60, '-') << "\n";

        for (int i = 0; i < numNodes; i++)
        {

            cout << "  To " << names[i] << " : ";

            if (dist[i] == INF)
            {
                cout << "unreachable\n";
                continue;
            }

            cout << fixed << setprecision(2) << dist[i] << "  | Path: ";

            vector<int> path = getPath(parent, source, i);

            for (int j = 0; j < path.size(); j++)
            {
                cout << names[path[j]];
                if (j != path.size() - 1)
                    cout << " -> ";
            }

            cout << "\n";
        }

        cout << "\n";
    }

    // Print adjacency list
    void printAdjList() const
    {
        cout << "\n  GRAPH STRUCTURE\n";
        cout << "  " << string(40, '-') << "\n";

        for (int i = 0; i < numNodes; i++)
        {

            cout << "  " << names[i] << ":\n";

            for (const Neighbor &edge : adjList[i])
            {
                cout << "     -> " << names[edge.node]
                     << " (cost: " << edge.cost << ")\n";
            }
        }

        cout << "\n";
    }
};
// ─────────────────────────────────────────────────────────────────────────────
//  SECTION 4 — UNION-FIND  (used by Kruskal)
//
//  Tracks which nodes are already connected so Kruskal can skip
//  edges that would form a cycle.
// ─────────────────────────────────────────────────────────────────────────────

class UnionFind
{
public:
    explicit UnionFind(int n) : parent_(n), rank_(n, 0)
    {
        for (int i = 0; i < n; ++i)
            parent_[i] = i; // each node is its own root
    }

    // Walk up to root, compressing path on the way back
    int find(int x)
    {
        if (parent_[x] != x)
            parent_[x] = find(parent_[x]); // path compression
        return parent_[x];
    }

    // Merge by rank: attach shorter tree under taller one
    // Returns true if merged (different components), false if same (would cycle)
    bool unite(int x, int y)
    {
        int rx = find(x), ry = find(y);
        if (rx == ry)
            return false; // already connected → skip edge

        if (rank_[rx] < rank_[ry])
            parent_[rx] = ry;
        else if (rank_[rx] > rank_[ry])
            parent_[ry] = rx;
        else
        {
            parent_[ry] = rx;
            rank_[rx]++;
        }
        return true;
    }

private:
    vector<int> parent_;
    vector<int> rank_;
};

// ─────────────────────────────────────────────────────────────────────────────
//  SECTION 5 — KRUSKAL'S MST
// ─────────────────────────────────────────────────────────────────────────────

struct Edge
{
    int src, dest;
    double weight;
};

struct MSTResult
{
    vector<Edge> edges;
    double totalWeight = 0.0;
};

MSTResult kruskal(const Graph &g)
{
    MSTResult result;

    // Collect unique edges (undirected: keep only src < dest)
    vector<Edge> edges;
    for (int u = 0; u < g.numNodes; ++u)
        for (auto [v, w] : g.adjList[u])
            if (u < v)
                edges.push_back({u, v, w});

    // Sort by weight ascending
    sort(edges.begin(), edges.end(),
         [](const Edge &a, const Edge &b)
         { return a.weight < b.weight; });

    UnionFind uf(g.numNodes);

    for (const Edge &e : edges)
    {
        if (static_cast<int>(result.edges.size()) == g.numNodes - 1)
            break;
        if (uf.unite(e.src, e.dest))
        { // no cycle → add to MST
            result.edges.push_back(e);
            result.totalWeight += e.weight;
        }
    }
    return result;
}

void printMST(const MSTResult &r, const Graph &g)
{
    cout << "\n  MINIMUM SPANNING TREE (Kruskal)\n";
    cout << "  " << string(48, '-') << "\n";
    cout << "  " << left << setw(3) << "#"
         << setw(16) << "From"
         << setw(16) << "To"
         << right << setw(10) << "Weight" << "\n";
    cout << "  " << string(48, '-') << "\n";

    for (size_t i = 0; i < r.edges.size(); ++i)
    {
        const auto &e = r.edges[i];
        cout << "  " << left << setw(3) << (i + 1)
             << setw(16) << g.names[e.src]
             << setw(16) << g.names[e.dest]
             << right << fixed << setprecision(2)
             << setw(10) << e.weight << "\n";
    }

    cout << "  " << string(48, '-') << "\n";
    bool valid = (static_cast<int>(r.edges.size()) == g.numNodes - 1);
    cout << "  Total MST weight : " << r.totalWeight << "\n";
    cout << "  Spanning tree    : "
         << (valid ? "valid (all nodes connected)" : "incomplete (disconnected graph)")
         << "\n\n";
}

// ─────────────────────────────────────────────────────────────────────────────
//  SECTION 6 — QUICK SORT
//
//  Sorts vector<Package> in-place using median-of-three pivot selection.
//
//  Comparators:  byWeight, byProfit, byRatio (profit/kg)
// ─────────────────────────────────────────────────────────────────────────────

using Cmp = function<bool(const Package &, const Package &)>;

namespace SortBy
{
    bool weight(const Package &a, const Package &b) { return a.weight < b.weight; }
    bool profit(const Package &a, const Package &b) { return a.profit < b.profit; }
    bool ratio(const Package &a, const Package &b) { return a.ratio() > b.ratio(); }
}

// Returns the index of the median among packages[low], [mid], [high]
int medianOfThree(vector<Package> &v, int low, int high, const Cmp &cmp)
{
    int mid = low + (high - low) / 2;
    if (cmp(v[mid], v[low]))
        swap(v[mid], v[low]);
    if (cmp(v[high], v[low]))
        swap(v[high], v[low]);
    if (cmp(v[high], v[mid]))
        swap(v[high], v[mid]);
    return mid; // v[mid] is the median
}

// Lomuto partition: pivot moved to end, then elements sorted around it
int partition(vector<Package> &v, int low, int high, const Cmp &cmp)
{
    int pivotIdx = medianOfThree(v, low, high, cmp);
    swap(v[pivotIdx], v[high]); // move pivot to end
    const Package &pivot = v[high];
    int i = low - 1;
    for (int j = low; j < high; ++j)
        if (cmp(v[j], pivot))
            swap(v[++i], v[j]);
    swap(v[i + 1], v[high]); // place pivot in final position
    return i + 1;
}

void quickSortHelper(vector<Package> &v, int low, int high, const Cmp &cmp)
{
    if (low >= high)
        return;
    int p = partition(v, low, high, cmp);
    quickSortHelper(v, low, p - 1, cmp);
    quickSortHelper(v, p + 1, high, cmp);
}

void quickSort(vector<Package> &v, const Cmp &cmp)
{
    if (v.size() < 2)
        return;
    quickSortHelper(v, 0, static_cast<int>(v.size()) - 1, cmp);
}

void printSortedPackages(const vector<Package> &v, const string &by)
{
    cout << "\n  PACKAGES sorted by " << by << "\n";
    cout << "  " << string(52, '-') << "\n";
    cout << "  " << left << setw(4) << "#"
         << setw(12) << "ID"
         << right << setw(12) << "Weight(kg)"
         << setw(12) << "Profit($)"
         << setw(12) << "Ratio" << "\n";
    cout << "  " << string(52, '-') << "\n";
    for (size_t i = 0; i < v.size(); ++i)
    {
        cout << "  " << left << setw(4) << (i + 1)
             << setw(12) << v[i].id
             << right << fixed << setprecision(2)
             << setw(12) << v[i].weight
             << setw(12) << v[i].profit
             << setw(12) << v[i].ratio() << "\n";
    }
    cout << "  " << string(52, '-') << "\n\n";
}

// ─────────────────────────────────────────────────────────────────────────────
//  SECTION 7 — MENU UI HELPERS
// ─────────────────────────────────────────────────────────────────────────────

namespace UI
{
    void flush()
    {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    int readInt(const string &prompt)
    {
        int v;
        while (true)
        {
            cout << prompt;
            if (cin >> v)
            {
                flush();
                return v;
            }
            cout << "  [!] Enter a whole number.\n";
            flush();
        }
    }

    double readPositive(const string &prompt)
    {
        double v;
        while (true)
        {
            cout << prompt;
            if (cin >> v && v >= 0)
            {
                flush();
                return v;
            }
            cout << "  [!] Enter a number >= 0.\n";
            flush();
        }
    }

    string readString(const string &prompt)
    {
        string s;
        while (true)
        {
            cout << prompt;
            getline(cin, s);
            size_t start = s.find_first_not_of(" \t");
            if (start != string::npos)
                return s.substr(start, s.find_last_not_of(" \t") - start + 1);
            cout << "  [!] Cannot be empty.\n";
        }
    }

    void pause()
    {
        cout << "\n  Press [Enter] to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  SECTION 8 — MAIN  (menu loop)
// ─────────────────────────────────────────────────────────────────────────────

void printMenu()
{
    cout << "\n"
         << "  +--------------------------------------------------+\n"
         << "  |       DELIVERY OPTIMIZATION SYSTEM               |\n"
         << "  +--------------------------------------------------+\n"
         << "  |  [1] Add Package                                 |\n"
         << "  |  [2] View Packages                               |\n"
         << "  |  [3] Optimize Load         (Knapsack DP)         |\n"
         << "  |  [4] Find Shortest Route   (Dijkstra)            |\n"
         << "  |  [5] Build Network         (Kruskal MST)         |\n"
         << "  |  [6] Sort Packages         (Quick Sort)          |\n"
         << "  |  [7] Exit                                        |\n"
         << "  +--------------------------------------------------+\n";
}

int main()
{
    // ── System setup ──────────────────────────────────────────────────────────
    const double CAPACITY = 50.0; // vehicle capacity in kg
    PackageManager pm;

    // Pre-built 5-node network (can be replaced via menu option 5)
    Graph g(5);
    g.setName(0, "Depot");
    g.setName(1, "City A");
    g.setName(2, "City B");
    g.setName(3, "City C");
    g.setName(4, "City D");
    g.addBiEdge(0, 1, 10.0);
    g.addBiEdge(0, 2, 6.0);
    g.addBiEdge(1, 2, 5.0);
    g.addBiEdge(1, 3, 15.0);
    g.addBiEdge(2, 4, 4.0);
    g.addBiEdge(3, 4, 10.0);

    // ── Demo packages ──────────────────────────────────────────────────────────
    pm.add(Package("PKG-001", 10.0, 60.0));
    pm.add(Package("PKG-002", 20.0, 100.0));
    pm.add(Package("PKG-003", 30.0, 120.0));
    pm.add(Package("PKG-004", 15.0, 90.0));
    pm.add(Package("PKG-005", 25.0, 80.0));
    pm.add(Package("PKG-006", 5.0, 50.0));

    cout << "\n  Delivery Optimization System  (vehicle capacity: "
         << CAPACITY << " kg)\n"
         << "  Demo data loaded: 6 packages, 5-node network.\n";

    // ── Main loop ──────────────────────────────────────────────────────────────
    bool running = true;
    while (running)
    {
        printMenu();
        int choice = UI::readInt("  Enter option [1-7]: ");

        try
        {
            switch (choice)
            {

            // ── 1. Add Package ────────────────────────────────────────────────
            case 1:
            {
                cout << "\n  ADD PACKAGE\n";
                string id = UI::readString("  ID            : ");
                double w = UI::readPositive("  Weight (kg)   : ");
                double p = UI::readPositive("  Profit ($)    : ");
                if (pm.add(Package(id, w, p)))
                    cout << "  Added '" << id << "' successfully.\n";
                UI::pause();
                break;
            }

            // ── 2. View Packages ──────────────────────────────────────────────
            case 2:
                cout << "\n  PACKAGE INVENTORY\n";
                pm.printAll();
                UI::pause();
                break;

            // ── 3. Knapsack ───────────────────────────────────────────────────
            case 3:
            {
                auto result = solveKnapsack(pm.all(), CAPACITY);
                printKnapsackResult(result);
                UI::pause();
                break;
            }

                // ── 4. Dijkstra ───────────────────────────────────────────────────
            case 4:
            {
                cout << "\n  Use demo network or enter a custom one?\n"
                     << "    [1] Demo network\n"
                     << "    [2] Custom network\n";

                int c = UI::readInt("  Choice: ");

               if (c == 2)
                {
                    int n = UI::readInt("  Number of nodes: ");
                    if (n <= 0)
                    {
                        cout << "  [!] Must be > 0.\n";
                        break;
                    }

                    g = Graph(n);

                    cout << "  Name each node (Enter to keep default):\n";

                    cin.ignore();

                    for (int i = 0; i < n; ++i)
                    {
                        cout << "  Node " << i << " [Node-" << i << "]: ";
                        string name;
                        getline(cin, name);

                        if (!name.empty())
                            g.setName(i, name);
                    }

                    int numEdges = UI::readInt("  Number of roads: ");

                    for (int i = 0; i < numEdges; ++i)
                    {
                        cout << "  Road " << (i + 1) << ":\n";

                        int u = UI::readInt("    From node : ");
                        int v = UI::readInt("    To node   : ");
                        double w = UI::readPositive("    Distance  : ");

                        try
                        {
                            g.addBiEdge(u, v, w);
                        }
                        catch (...)
                        {
                            cout << "  [!] Invalid nodes.\n";
                            --i;
                        }
                    }
                }

                g.printAdjList();

                int src = UI::readInt("  Source node index (0-" + to_string(g.numNodes - 1) + "): ");

                if (src < 0 || src >= g.numNodes)
                    cout << "  [!] Invalid node index.\n";
                else
                    g.printDijkstra(src);

                UI::pause();
                break;
            }
            // case 4: {
            //     g.printAdjList();
            //     int src = UI::readInt("  Source node index (0-"
            //                           + to_string(g.numNodes-1) + "): ");
            //     if (src < 0 || src >= g.numNodes)
            //         cout << "  [!] Invalid node index.\n";
            //     else
            //         g.printDijkstra(src);
            //     UI::pause();
            //     break;
            // }

            // ── 5. Kruskal MST ────────────────────────────────────────────────
            case 5:
            {
                cout << "\n  Use demo network or enter a custom one?\n"
                     << "    [1] Demo network\n"
                     << "    [2] Custom network\n";
                int c = UI::readInt("  Choice: ");

                if (c == 2)
                {
                    int n = UI::readInt("  Number of nodes: ");
                    if (n <= 0)
                    {
                        cout << "  [!] Must be > 0.\n";
                        break;
                    }
                    g = Graph(n);
                    cout << "  Name each node (Enter to keep default):\n";
                    for (int i = 0; i < n; ++i)
                    {
                        cout << "  Node " << i << " [Node-" << i << "]: ";
                        string name;
                        getline(cin, name);
                        if (!name.empty())
                            g.setName(i, name);
                    }
                    int numEdges = UI::readInt("  Number of roads: ");
                    for (int i = 0; i < numEdges; ++i)
                    {
                        cout << "  Road " << (i + 1) << ": ";
                        int u = UI::readInt("    From node : ");
                        int v = UI::readInt("    To   node : ");
                        double w = UI::readPositive("    Distance  : ");
                        try
                        {
                            g.addBiEdge(u, v, w);
                        }
                        catch (...)
                        {
                            cout << "  [!] Invalid nodes.\n";
                        }
                    }
                }

                auto result = kruskal(g);
                printMST(result, g);
                UI::pause();
                break;
            }

            // ── 6. Quick Sort ─────────────────────────────────────────────────
            case 6:
            {
                if (pm.count() == 0)
                {
                    cout << "  [!] No packages to sort.\n";
                    UI::pause();
                    break;
                }
                cout << "\n  Sort packages by:\n"
                     << "    [1] Weight  (ascending)\n"
                     << "    [2] Profit  (descending)\n"
                     << "    [3] Ratio   (profit/kg, descending)\n";
                int key = UI::readInt("  Choice: ");

                vector<Package> sorted(pm.all().begin(), pm.all().end());
                string byLabel;

                if (key == 1)
                {
                    quickSort(sorted, SortBy::weight);
                    byLabel = "weight";
                }
                else if (key == 2)
                {
                    quickSort(sorted, SortBy::profit);
                    byLabel = "profit";
                }
                else if (key == 3)
                {
                    quickSort(sorted, SortBy::ratio);
                    byLabel = "ratio";
                }
                else
                {
                    cout << "  [!] Invalid choice.\n";
                    UI::pause();
                    break;
                }

                printSortedPackages(sorted, byLabel);
                UI::pause();
                break;
            }

            // ── 7. Exit ───────────────────────────────────────────────────────
            case 7:
                cout << "\n  Goodbye!\n\n";
                running = false;
                break;

            default:
                cout << "  [!] Invalid option. Choose 1-7.\n";
            }
        }
        catch (const exception &e)
        {
            cout << "  [!] Error: " << e.what() << "\n";
            UI::pause();
        }
    }
    return 0;
}
