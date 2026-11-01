// Adaptive City Routing System
//
// Usage:  ./adaptive_routing [map_file]
//         ./adaptive_routing data/city_map.txt < examples/demo_input.txt
//
// The program is menu-driven and reads all input from standard input, so it
// works both interactively and with redirected input files (used by Colab).

#include <exception>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "algorithms.h"
#include "cli.h"
#include "graph.h"

namespace {

const std::string kDefaultMapFile = "data/city_map.txt";

// Balanced profile used in the comparison table: distance 40%, time 40%, cost 20%.
const Weights kBalancedWeights{0.4, 0.4, 0.2};

void runStrategy(const Graph& graph,
                 const RouteQuery& query,
                 Criterion criterion,
                 const std::string& title,
                 const Weights& weights = Weights()) {
    const RouteResult route = findShortestRoute(graph, query.source, query.destination,
                                                criterion, query.constraints, weights);
    cli::printRoute(title, route, query);
}

void runComparison(const Graph& graph, const RouteQuery& query) {
    const std::vector<std::pair<std::string, RouteResult>> rows = {
        {"Shortest distance",
         findShortestRoute(graph, query.source, query.destination, Criterion::Distance, query.constraints)},
        {"Fastest time",
         findShortestRoute(graph, query.source, query.destination, Criterion::Time, query.constraints)},
        {"Cheapest cost",
         findShortestRoute(graph, query.source, query.destination, Criterion::Cost, query.constraints)},
        {"Balanced (0.4/0.4/0.2)",
         findShortestRoute(graph, query.source, query.destination, Criterion::Weighted,
                           query.constraints, kBalancedWeights)},
    };

    for (const auto& row : rows) {
        cli::printRoute(row.first, row.second, query);
    }
    cli::printComparison(rows);
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " [map_file]\n";
        return 1;
    }
    const std::string mapFile = (argc == 2) ? argv[1] : kDefaultMapFile;

    Graph graph;
    try {
        const LoadReport report = graph.loadFromFile(mapFile);
        std::cout << "Loaded " << report.edgesLoaded << " road segments from '" << mapFile << "'.\n";
        for (const std::string& warning : report.warnings) {
            std::cout << "  Warning: " << warning << "\n";
        }
        if (report.edgesLoaded == 0) {
            std::cerr << "Error: the map file contains no valid road segments.\n";
            return 1;
        }
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << "\n";
        return 1;
    }

    try {
        cli::printBanner();
        while (true) {
            cli::printMenu();
            const int choice = cli::readInt("\nEnter your choice (1-7): ", 1, 7);

            switch (choice) {
                case 1:
                    cli::printMap(graph);
                    break;
                case 2:
                    runStrategy(graph, cli::promptRouteQuery(graph), Criterion::Distance,
                                "Shortest distance route");
                    break;
                case 3:
                    runStrategy(graph, cli::promptRouteQuery(graph), Criterion::Time,
                                "Fastest time route");
                    break;
                case 4:
                    runStrategy(graph, cli::promptRouteQuery(graph), Criterion::Cost,
                                "Cheapest cost route");
                    break;
                case 5: {
                    const RouteQuery query = cli::promptRouteQuery(graph);
                    const Weights weights = cli::promptWeights();
                    runStrategy(graph, query, Criterion::Weighted, "Custom weighted route", weights);
                    break;
                }
                case 6:
                    runComparison(graph, cli::promptRouteQuery(graph));
                    break;
                case 7:
                    std::cout << "\nProgram ended. Goodbye!\n";
                    return 0;
                default:
                    break;  // unreachable: readInt guarantees 1..7
            }
        }
    } catch (const EndOfInput&) {
        std::cout << "\nInput closed. Program ended.\n";
        return 0;
    }
}
