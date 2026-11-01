#ifndef CLI_H
#define CLI_H

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "algorithms.h"
#include "graph.h"

// Thrown when standard input ends (Ctrl+D, or a redirected input file runs out).
class EndOfInput : public std::runtime_error {
public:
    EndOfInput() : std::runtime_error("end of input") {}
};

// Everything the user enters for one routing request.
struct RouteQuery {
    std::string source;
    std::string destination;
    Constraints constraints;
};

namespace cli {

// ----- Input. Every prompt re-asks until the answer is valid. -----

// Prints the prompt and returns the trimmed line. Throws EndOfInput at end of input.
std::string readLine(const std::string& prompt);

// Whole number in [minValue, maxValue].
int readInt(const std::string& prompt, int minValue, int maxValue);

// Real number that is zero or greater.
double readNonNegativeNumber(const std::string& prompt);

// Asks for the three weights of the custom scoring function.
Weights promptWeights();

// Asks for source, destination, locations to avoid and transports to avoid.
RouteQuery promptRouteQuery(const Graph& graph);

// ----- Output. -----

void printBanner();
void printMenu();
void printAvailableLocations(const Graph& graph);
void printMap(const Graph& graph);
void printRoute(const std::string& title, const RouteResult& route, const RouteQuery& query);
void printComparison(const std::vector<std::pair<std::string, RouteResult>>& rows);

}  // namespace cli

#endif
