#include "algorithms.h"

#include <algorithm>
#include <chrono>
#include <functional>
#include <limits>
#include <queue>
#include <unordered_map>
#include <utility>

namespace {

constexpr double kInfinity = std::numeric_limits<double>::infinity();

// The cost of crossing one road segment under the chosen criterion.
double edgeObjective(const Edge& edge, Criterion criterion, const Weights& weights) {
    switch (criterion) {
        case Criterion::Distance:
            return edge.distance;
        case Criterion::Time:
            return edge.time;
        case Criterion::Cost:
            return edge.cost;
        case Criterion::Weighted:
            return edge.distance * weights.distance
                 + edge.time * weights.time
                 + edge.cost * weights.cost;
    }
    return edge.distance;  // not reached: every enumerator is handled above
}

// Best known way of reaching one location during the search.
struct Label {
    double objective = kInfinity;  // best criterion value found so far
    double distance = 0.0;         // accumulated km along that best path
    double time = 0.0;             // accumulated minutes along that best path
    double cost = 0.0;             // accumulated taka along that best path
    std::string previous;          // predecessor on that best path
    std::string transport;         // transport used from `previous`
};

RouteResult search(const Graph& graph,
                   const std::string& source,
                   const std::string& destination,
                   Criterion criterion,
                   const Constraints& constraints,
                   const Weights& weights) {
    RouteResult result;

    if (!graph.hasNode(source) || !graph.hasNode(destination)) return result;
    if (constraints.avoidNodes.count(source) || constraints.avoidNodes.count(destination)) {
        return result;
    }

    if (source == destination) {
        result.found = true;
        result.path = {source};
        return result;
    }

    std::unordered_map<std::string, Label> labels;
    for (const std::string& node : graph.getAllNodes()) {
        labels[node];  // default-constructed: objective = infinity
    }
    labels[source].objective = 0.0;

    using QueueEntry = std::pair<double, std::string>;
    std::priority_queue<QueueEntry, std::vector<QueueEntry>, std::greater<QueueEntry>> queue;
    queue.push({0.0, source});

    while (!queue.empty()) {
        const QueueEntry top = queue.top();
        queue.pop();
        const double objective = top.first;
        const std::string current = top.second;

        if (objective > labels[current].objective) continue;  // stale queue entry
        if (current == destination) break;                    // destination is settled

        const Label currentLabel = labels[current];
        for (const Edge& edge : graph.getNeighbors(current)) {
            if (constraints.avoidNodes.count(edge.destination)) continue;
            if (constraints.avoidTransports.count(edge.transport)) continue;

            const double candidate = objective + edgeObjective(edge, criterion, weights);
            Label& next = labels[edge.destination];
            if (candidate < next.objective) {
                next.objective = candidate;
                next.distance = currentLabel.distance + edge.distance;
                next.time = currentLabel.time + edge.time;
                next.cost = currentLabel.cost + edge.cost;
                next.previous = current;
                next.transport = edge.transport;
                queue.push({candidate, edge.destination});
            }
        }
    }

    const Label& target = labels[destination];
    if (target.objective == kInfinity) return result;  // no admissible route

    result.found = true;
    result.objectiveValue = target.objective;
    result.totalDistance = target.distance;
    result.totalTime = target.time;
    result.totalCost = target.cost;

    // Walk back from the destination to the source, then reverse both lists.
    std::string node = destination;
    while (node != source) {
        result.path.push_back(node);
        result.transports.push_back(labels[node].transport);
        node = labels[node].previous;
    }
    result.path.push_back(source);
    std::reverse(result.path.begin(), result.path.end());
    std::reverse(result.transports.begin(), result.transports.end());

    return result;
}

}  // namespace

RouteResult findShortestRoute(const Graph& graph,
                              const std::string& source,
                              const std::string& destination,
                              Criterion criterion,
                              const Constraints& constraints,
                              const Weights& weights) {
    const auto start = std::chrono::steady_clock::now();
    RouteResult result = search(graph, source, destination, criterion, constraints, weights);
    const auto end = std::chrono::steady_clock::now();

    result.searchTimeMs = std::chrono::duration<double, std::milli>(end - start).count();
    return result;
}
