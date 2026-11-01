#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <set>
#include <string>
#include <vector>

#include "graph.h"

// The quantity a route search minimises.
enum class Criterion {
    Distance,  // total distance (km)
    Time,      // total travel time (minutes)
    Cost,      // total fare (taka)
    Weighted   // weighted sum: distance*wd + time*wt + cost*wc
};

// Weights for Criterion::Weighted. All must be non-negative.
struct Weights {
    double distance = 1.0;
    double time = 1.0;
    double cost = 1.0;
};

// Restrictions supplied by the user.
struct Constraints {
    std::set<std::string> avoidNodes;       // locations the route must not visit
    std::set<std::string> avoidTransports;  // transport types the route must not use
};

// Result of a route search.
struct RouteResult {
    bool found = false;
    std::vector<std::string> path;        // locations, source first, destination last
    std::vector<std::string> transports;  // transports[i] joins path[i] to path[i + 1]
    double objectiveValue = 0.0;          // value of the quantity that was minimised
    double totalDistance = 0.0;           // km
    double totalTime = 0.0;               // minutes
    double totalCost = 0.0;               // taka
    double searchTimeMs = 0.0;            // wall-clock time of the search
};

// Finds the best route from `source` to `destination` using Dijkstra's
// algorithm. Complexity: O((V + E) log V).
RouteResult findShortestRoute(const Graph& graph,
                              const std::string& source,
                              const std::string& destination,
                              Criterion criterion,
                              const Constraints& constraints = Constraints(),
                              const Weights& weights = Weights());

#endif
