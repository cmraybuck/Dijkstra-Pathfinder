#include <iostream>
#include <fstream>
#include <list>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <random>
#include <string>
#include <queue>
#include <climits>
#include <sstream>
#include <cfloat>
#include <algorithm>

using std::list;
using std::unordered_set;
using namespace std;

class Graph
{
public:
    class Edge
    {
    public:
        Graph* destination;
        double distance;
    };
    std::string name;
    list<Edge> edgeList;
    int index;
    void addEdge(Graph* g, double distance)
    {
        edgeList.push_back({ g, distance });
        g->edgeList.push_back({ this, distance });
    }
};

list<Graph> GenerateGraphs() {
    string data;
    list<Graph> totalGraphs;
    unordered_map<string, Graph*> nodes;
    ifstream DataFile("./data/edges.txt");
    while (DataFile >> data) {
        Graph* location;
        if (nodes.count(data)) {
            location = nodes[data];
        }
        else {
            totalGraphs.push_back(Graph());
            location = &totalGraphs.back();
            location->name = data;
            nodes[data] = location;
        }
        DataFile >> data;
        Graph* destination;
        if (nodes.count(data)) {
            destination = nodes[data];
        }
        else {
            totalGraphs.push_back(Graph());
            destination = &totalGraphs.back();
            destination->name = data;
            nodes[data] = destination;
        }
        DataFile >> data;
        double distance = stod(data);
        location->addEdge(destination, distance);
    }
    DataFile.close();
    int i = 0;
    for (Graph& g : totalGraphs) {
        g.index = i++;
    }
    return totalGraphs;
}


pair<double, vector<string>> dijkstra(Graph* start, Graph* end, const list<Graph>& locations) {   // This will return 2 things: a double and a string ptr containing the path
    int V = locations.size();

    priority_queue<pair<double, Graph*>, vector<pair<double, Graph*>>,
        greater<pair<double, Graph*>>> pq;                                         // Instantiate min-heap priority queue

    vector<double> dist(V, DBL_MAX);                                // Initialize distances to all nodes as infinite
    vector<Graph*> prev(V, nullptr);                           // Pointer to keep track of the previous node

    dist[start->index] = 0;                                  // Distance to start node is 0
    pq.emplace(0, start);                                    // Push start node into priority queue

    while (!pq.empty()) {                                     // While priority queue is not empty
        pair<double, Graph*> topPair = pq.top();              // Get node with smallest distance
        pq.pop();   								          // We then remove it from the queue since we do not need to check it again

        double currentDist = topPair.first;
        Graph* node = topPair.second;

        if (node == end) {
            break;
        }

        if (currentDist > dist[node->index]) {                    // If we have already found a better path to this node, skip processing
            continue;
        }

        for (Graph::Edge& e : node->edgeList) {                     // Explore neighbors
            Graph* neighbor = e.destination;
            double weight = e.distance;

            if (currentDist + weight < dist[neighbor->index]) {           // If a shorter path to neighbor is found
                dist[neighbor->index] = currentDist + weight;           // Update distance to neighbor
                prev[neighbor->index] = node;
                pq.emplace(dist[neighbor->index], neighbor);            // Push neighbor into priority queue
            }
        }
    }

    vector<string> path;
    Graph* cur = end;

    while (cur != nullptr) {
        path.push_back(cur->name);
        cur = prev[cur->index];
    }

    // Reverse the path because it’s from end to start
    reverse(path.begin(), path.end());

    return { dist[end->index], path };
}


Graph* findLocation(list<Graph>& locations, const string& place) {
    for (Graph& g : locations) {
        if (g.name == place) {
            return &g;
        }
    }
    return nullptr;
}

string stringLookup(string location, list<Graph> graphs)
{
    Graph* desiredNode = nullptr;//
    string nearestStop = "null";
    double distance = 0;
    int firstLoop = 1;
    for (Graph& node : graphs)
    {
        if (node.name == location) {
            desiredNode = &node;
            break;
        }
    }
    if (desiredNode == nullptr) {
        cout << "Error: location not found in file." << endl;
        return "Error";
    }
    for (auto Edge : desiredNode->edgeList) {
        if (firstLoop) {
            distance = Edge.distance;
            nearestStop = Edge.destination->name;
            firstLoop = 0;
            continue;
        }
        if (Edge.distance < distance) {
            distance = Edge.distance;
            nearestStop = Edge.destination->name;
        }
    }
    return nearestStop;
}

pair<double, vector<string>> FindRoute(list<Graph>& graphs, const string& from, const string& to) // Input a graph list and two location names, returns shortest distance using Dijkstra's algorithm
{
    Graph* start = findLocation(graphs, from);
    Graph* end = findLocation(graphs, to);

    if (!start || !end) {
        return { -1, {} };
    }

    return dijkstra(start, end, graphs);
}

int main() {
    list<Graph> graphs = GenerateGraphs();
    bool powerOn = true;
    string input1;
    string input2;
    string input3;
    int timeToTravel;
    int csvGenerated = 0;
    cout << "--------------" << endl << "Commands: " << endl << "> /exit: Terminate program" << endl << "> /lookup: Enter string lookup mode. First enter \"/lookup\" in the terminal. A confirmation message will appear in the terminal that you have entered string lookup mode." << endl << "--------------" << endl;
    while (powerOn)
    {
        cin >> input1;
        if (input1 == "/exit")
        {
            cout << "Exiting program..." << endl;
            powerOn = false;
            break;
        }
        if (input1 == "/lookup") {
            cout << "Entering string lookup mode..." << endl;
            cout << "Enter location: " << endl;
            cin >> input2;
            string nearest = stringLookup(input2, graphs);
            if (nearest != "Error") {
                cout << "Closest stop to " << input2 << " is " << nearest << endl;
            }
        }
        else
        {
            cin >> input2;
            auto result = FindRoute(graphs, input1, input2);
            if (result.first < 0 || result.first == DBL_MAX) {
                cout << "error no_path" << endl;
            }
            else {
                timeToTravel = ceil(result.first / 1.4);
                cout << "ok " << result.first << " " << timeToTravel << endl;
                ofstream csvFile("./data/output.csv", ios::app);
                if (!csvGenerated)
                {
                    csvFile << "src,dst,distance_m,eta_s,time_ms\n";
                    csvGenerated = 1;
                }
                csvFile << input1 << "," << input2 << "," << result.first << "," << timeToTravel << "," << timeToTravel * 1000 << "\n";
                csvFile.close();
                auto result = FindRoute(graphs, input1, input2);
                ofstream WriteFile("./data/path.txt", ios::app);
                for (size_t i = 0; i < result.second.size(); ++i) {
                    WriteFile << result.second[i];
                    if (i != result.second.size() - 1) {
                        WriteFile << " -> ";
                    }
                }
                WriteFile << "\n";
                WriteFile.close();
            }
        }
    }
    return 0;
}