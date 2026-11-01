#include "cli.h"

#include <cstddef>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "utils.h"

namespace cli {

namespace {

const std::string kRule(74, '-');

// Asks for a location until the user names one that exists (case does not matter).
std::string promptLocation(const Graph& graph, const std::string& prompt) {
    while (true) {
        const std::string input = readLine(prompt);
        const std::string canonical = graph.resolveNode(input);
        if (!canonical.empty()) return canonical;
        std::cout << "  Unknown location '" << input << "'. Choose one of the names listed above.\n";
    }
}

// Reads a comma-separated list and returns the canonical names it contains.
// `resolve` maps user text to a canonical name, or "" when the name is unknown;
// unknown names are reported and ignored.
template <typename Resolver>
std::set<std::string> promptNameList(const std::string& prompt, const std::string& kind, Resolver resolve) {
    std::set<std::string> chosen;
    for (const std::string& name : splitCsv(readLine(prompt))) {
        const std::string canonical = resolve(name);
        if (canonical.empty()) {
            std::cout << "  Warning: unknown " << kind << " '" << name << "' ignored.\n";
        } else {
            chosen.insert(canonical);
        }
    }
    return chosen;
}

}  // namespace

// ---------------------------------------------------------------- input

std::string readLine(const std::string& prompt) {
    std::cout << prompt << std::flush;
    std::string line;
    if (!std::getline(std::cin, line)) throw EndOfInput();
    return trim(line);
}

int readInt(const std::string& prompt, int minValue, int maxValue) {
    while (true) {
        std::istringstream stream(readLine(prompt));
        int value = 0;
        char leftover = 0;
        if (stream >> value && !(stream >> leftover) && value >= minValue && value <= maxValue) {
            return value;
        }
        std::cout << "  Please enter a whole number from " << minValue << " to " << maxValue << ".\n";
    }
}

double readNonNegativeNumber(const std::string& prompt) {
    while (true) {
        std::istringstream stream(readLine(prompt));
        double value = 0.0;
        char leftover = 0;
        if (stream >> value && !(stream >> leftover) && value >= 0.0) {
            return value;
        }
        std::cout << "  Please enter a number that is zero or greater.\n";
    }
}

Weights promptWeights() {
    std::cout << "\nCustom score = distance x wd + time x wt + cost x wc\n";
    std::cout << "(Units differ: km, minutes and taka. Choose weights that reflect your priorities.)\n";
    while (true) {
        Weights weights;
        weights.distance = readNonNegativeNumber("  Distance weight (wd, >= 0): ");
        weights.time     = readNonNegativeNumber("  Time weight     (wt, >= 0): ");
        weights.cost     = readNonNegativeNumber("  Cost weight     (wc, >= 0): ");
        if (weights.distance + weights.time + weights.cost > 0.0) return weights;
        std::cout << "  At least one weight must be greater than zero.\n";
    }
}

RouteQuery promptRouteQuery(const Graph& graph) {
    printAvailableLocations(graph);

    RouteQuery query;
    query.source = promptLocation(graph, "\nEnter source location: ");
    query.destination = promptLocation(graph, "Enter destination location: ");

    query.constraints.avoidNodes = promptNameList(
        "Locations to avoid, comma-separated (Enter to skip): ", "location",
        [&graph](const std::string& name) { return graph.resolveNode(name); });

    // A route must start and end at the chosen locations, so they cannot be avoided.
    if (query.constraints.avoidNodes.count(query.source) ||
        query.constraints.avoidNodes.count(query.destination)) {
        std::cout << "  Note: the source and destination cannot be avoided and were removed from the list.\n";
        query.constraints.avoidNodes.erase(query.source);
        query.constraints.avoidNodes.erase(query.destination);
    }

    query.constraints.avoidTransports = promptNameList(
        "Transport types to avoid, comma-separated (Enter to skip): ", "transport type",
        [&graph](const std::string& name) { return graph.resolveTransport(name); });

    return query;
}

// ---------------------------------------------------------------- output

void printBanner() {
    std::cout << "\n" << kRule << "\n"
              << "  ADAPTIVE CITY ROUTING SYSTEM\n"
              << "  Shortest, fastest, cheapest and custom routes across the city\n"
              << kRule << "\n";
}

void printMenu() {
    std::cout << "\n  1. Display city map\n"
              << "  2. Shortest distance route\n"
              << "  3. Fastest time route\n"
              << "  4. Cheapest cost route\n"
              << "  5. Custom weighted route\n"
              << "  6. Compare all strategies\n"
              << "  7. Exit\n";
}

void printAvailableLocations(const Graph& graph) {
    std::cout << "\nAvailable locations : " << join(graph.getAllNodes(), ", ") << "\n";
    std::cout << "Transport types     : " << join(graph.getTransportTypes(), ", ") << "\n";
    std::cout << "(Names are not case-sensitive.)\n";
}

void printMap(const Graph& graph) {
    std::cout << "\n" << kRule << "\n"
              << "CITY MAP (each road listed once)\n"
              << kRule << "\n";
    std::cout << std::left << std::setw(14) << "From" << std::setw(14) << "To"
              << std::setw(12) << "Transport"
              << std::right << std::setw(14) << "Distance(km)"
              << std::setw(12) << "Time(min)"
              << std::setw(12) << "Cost(taka)" << "\n";
    std::cout << kRule << "\n";

    std::cout << std::fixed << std::setprecision(1);
    for (const Road& road : graph.getRoads()) {
        std::cout << std::left << std::setw(14) << road.from << std::setw(14) << road.to
                  << std::setw(12) << road.transport
                  << std::right << std::setw(14) << road.distance
                  << std::setw(12) << road.time
                  << std::setw(12) << road.cost << "\n";
    }
    std::cout << kRule << "\n";
}

void printRoute(const std::string& title, const RouteResult& route, const RouteQuery& query) {
    std::cout << "\n--- " << title << " ---\n";

    if (!route.found) {
        std::cout << "No route found from " << query.source << " to " << query.destination
                  << " with the given restrictions.\n";
        return;
    }

    std::cout << "Path     : ";
    for (std::size_t i = 0; i < route.path.size(); ++i) {
        std::cout << route.path[i];
        if (i < route.transports.size()) {
            std::cout << " --[" << route.transports[i] << "]--> ";
        }
    }
    std::cout << "\n" << std::fixed << std::setprecision(2)
              << "Distance : " << route.totalDistance << " km\n"
              << "Time     : " << route.totalTime << " minutes\n"
              << "Cost     : " << route.totalCost << " taka\n"
              << std::setprecision(3)
              << "Search   : " << route.searchTimeMs << " ms\n";
}

void printComparison(const std::vector<std::pair<std::string, RouteResult>>& rows) {
    std::cout << "\n--- Comparison summary ---\n";
    std::cout << std::left << std::setw(26) << "Strategy"
              << std::right << std::setw(14) << "Distance(km)"
              << std::setw(12) << "Time(min)"
              << std::setw(12) << "Cost(taka)"
              << std::setw(10) << "Segments" << "\n";
    std::cout << kRule << "\n";

    for (const auto& row : rows) {
        const std::string& name = row.first;
        const RouteResult& route = row.second;

        std::cout << std::left << std::setw(26) << name;
        if (!route.found) {
            std::cout << std::right << std::setw(48) << "no route" << "\n";
            continue;
        }
        std::cout << std::right << std::fixed << std::setprecision(2)
                  << std::setw(14) << route.totalDistance
                  << std::setw(12) << route.totalTime
                  << std::setw(12) << route.totalCost
                  << std::setw(10) << route.path.size() - 1 << "\n";
    }
    std::cout << kRule << "\n";
}

}  // namespace cli
