// Self-contained unit tests for the graph loader and the route search.
// Expected values are worked out by hand from data/city_map.txt.
//
// Build and run:  make test

#include <cmath>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "algorithms.h"
#include "graph.h"

namespace {

int passed = 0;
int failed = 0;

using Names = std::vector<std::string>;

void check(bool condition, const std::string& description) {
    if (condition) {
        ++passed;
        std::cout << "  [PASS] " << description << "\n";
    } else {
        ++failed;
        std::cout << "  [FAIL] " << description << "\n";
    }
}

bool nearlyEqual(double a, double b) {
    return std::fabs(a - b) < 1e-9;
}

}  // namespace

int main(int argc, char* argv[]) {
    const std::string mapFile = (argc > 1) ? argv[1] : "data/city_map.txt";

    Graph graph;
    try {
        const LoadReport report = graph.loadFromFile(mapFile);

        std::cout << "Map loading\n";
        check(report.edgesLoaded == 14, "map contains 14 road segments");
        check(report.warnings.empty(), "map has no malformed lines");
        check(graph.getRoads().front().from == "Uttara" && graph.getRoads().front().to == "Banani",
              "roads keep their order from the file");
    } catch (const std::exception& error) {
        std::cerr << "Cannot run tests: " << error.what() << "\n";
        return 1;
    }

    std::cout << "Name lookup\n";
    check(graph.resolveNode("uttara") == "Uttara", "lookup ignores case");
    check(graph.resolveNode("Dhaka").empty(), "unknown location is rejected");
    check(graph.resolveTransport("METRO") == "Metro", "transport lookup ignores case");

    const Constraints none;

    std::cout << "Shortest distance (Uttara -> Motijheel)\n";
    const RouteResult shortest =
        findShortestRoute(graph, "Uttara", "Motijheel", Criterion::Distance, none);
    check(shortest.found, "a route exists");
    check(nearlyEqual(shortest.totalDistance, 26.0), "distance is 26 km");
    check(shortest.path == Names({"Uttara", "Mirpur", "Shahbag", "Motijheel"}),
          "path is Uttara-Mirpur-Shahbag-Motijheel");
    check(shortest.transports == Names({"Bus", "Metro", "Metro"}),
          "transports are Bus, Metro, Metro");

    std::cout << "Fastest time\n";
    const RouteResult fastest =
        findShortestRoute(graph, "Uttara", "Motijheel", Criterion::Time, none);
    check(nearlyEqual(fastest.totalTime, 62.0), "time is 62 minutes");

    std::cout << "Cheapest cost\n";
    const RouteResult cheapest =
        findShortestRoute(graph, "Uttara", "Motijheel", Criterion::Cost, none);
    check(nearlyEqual(cheapest.totalCost, 85.0), "cost is 85 taka");
    check(cheapest.path == Names({"Uttara", "Banani", "Gulshan", "Motijheel"}),
          "cheapest path uses the free walking segment");

    std::cout << "Weighted score\n";
    const Weights balanced{0.4, 0.4, 0.2};
    const RouteResult weighted =
        findShortestRoute(graph, "Uttara", "Motijheel", Criterion::Weighted, none, balanced);
    check(nearlyEqual(weighted.objectiveValue, 53.2), "balanced score is 53.2");

    std::cout << "Constraints\n";
    Constraints avoidMirpur;
    avoidMirpur.avoidNodes = {"Mirpur"};
    const RouteResult detour =
        findShortestRoute(graph, "Uttara", "Motijheel", Criterion::Distance, avoidMirpur);
    check(nearlyEqual(detour.totalDistance, 27.0), "avoiding Mirpur gives a 27 km route");
    check(detour.path == Names({"Uttara", "Banani", "Gulshan", "Motijheel"}),
          "detour path is Uttara-Banani-Gulshan-Motijheel");

    Constraints avoidMirpurAndWalking;
    avoidMirpurAndWalking.avoidNodes = {"Mirpur"};
    avoidMirpurAndWalking.avoidTransports = {"Walking"};
    const RouteResult pricierDetour =
        findShortestRoute(graph, "Uttara", "Motijheel", Criterion::Cost, avoidMirpurAndWalking);
    check(nearlyEqual(pricierDetour.totalCost, 120.0),
          "avoiding Mirpur and Walking gives a 120 taka route");

    Constraints noBusNoMetro;
    noBusNoMetro.avoidTransports = {"Bus", "Metro"};
    check(!findShortestRoute(graph, "Uttara", "Motijheel", Criterion::Distance, noBusNoMetro).found,
          "no route when both Bus and Metro are avoided");

    Constraints avoidSource;
    avoidSource.avoidNodes = {"Uttara"};
    check(!findShortestRoute(graph, "Uttara", "Motijheel", Criterion::Distance, avoidSource).found,
          "no route when the source is avoided");

    std::cout << "Edge cases\n";
    const RouteResult same = findShortestRoute(graph, "Banani", "Banani", Criterion::Time, none);
    check(same.found && nearlyEqual(same.totalTime, 0.0) && same.path == Names({"Banani"}),
          "source equal to destination gives a zero-length trip");
    check(!findShortestRoute(graph, "Dhaka", "Motijheel", Criterion::Distance, none).found,
          "unknown source gives no route");

    std::cout << "\nResult: " << passed << " passed, " << failed << " failed\n";
    return failed == 0 ? 0 : 1;
}
