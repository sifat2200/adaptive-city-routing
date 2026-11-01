#ifndef GRAPH_H
#define GRAPH_H

#include <cstddef>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

// One road segment, as seen from one of its two ends.
struct Edge {
    std::string destination;  // location at the other end of the road
    std::string transport;    // e.g. "Bus", "Metro", "Walking"
    double distance;          // kilometres
    double time;              // minutes
    double cost;              // taka
};

// A road exactly as it was written in the map file (used for display).
struct Road {
    std::string from;
    std::string to;
    std::string transport;
    double distance;  // kilometres
    double time;      // minutes
    double cost;      // taka
};

// Summary of a map-loading operation.
struct LoadReport {
    std::size_t edgesLoaded = 0;        // number of road segments added
    std::vector<std::string> warnings;  // one message per skipped line
};

// Undirected, weighted multigraph describing the city road network.
class Graph {
public:
    // Adds a road segment that can be used in both directions.
    void addEdge(const std::string& from, const std::string& to, const std::string& transport,
                 double distance, double time, double cost);

    // Reads "from to transport distance time cost" records, one per line.
    // Blank lines and lines starting with '#' are ignored. Malformed lines
    // are skipped and reported in LoadReport::warnings.
    // Throws std::runtime_error if the file cannot be opened.
    LoadReport loadFromFile(const std::string& filename);

    // Roads leaving `node`. Returns an empty list for unknown nodes.
    const std::vector<Edge>& getNeighbors(const std::string& node) const;

    // All roads, in the order they were added (file order).
    const std::vector<Road>& getRoads() const;

    std::vector<std::string> getAllNodes() const;        // alphabetical order
    std::vector<std::string> getTransportTypes() const;  // alphabetical order

    bool hasNode(const std::string& node) const;

    // Case-insensitive lookups. Return the canonical spelling, or "" if unknown.
    std::string resolveNode(const std::string& name) const;
    std::string resolveTransport(const std::string& name) const;

private:
    std::unordered_map<std::string, std::vector<Edge>> adjacency_;
    std::vector<Road> roads_;
    std::set<std::string> nodeNames_;
    std::set<std::string> transportNames_;
};

#endif
