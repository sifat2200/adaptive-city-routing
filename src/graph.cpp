#include "graph.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

#include "utils.h"

namespace {

// Returns the stored name that matches `query` ignoring case, or "" if none does.
std::string resolveIn(const std::set<std::string>& names, const std::string& query) {
    const std::string wanted = toLower(trim(query));
    for (const std::string& name : names) {
        if (toLower(name) == wanted) return name;
    }
    return "";
}

}  // namespace

void Graph::addEdge(const std::string& from, const std::string& to, const std::string& transport,
                    double distance, double time, double cost) {
    adjacency_[from].push_back({to, transport, distance, time, cost});
    adjacency_[to].push_back({from, transport, distance, time, cost});
    roads_.push_back({from, to, transport, distance, time, cost});
    nodeNames_.insert(from);
    nodeNames_.insert(to);
    transportNames_.insert(transport);
}

LoadReport Graph::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("cannot open map file '" + filename + "'");
    }

    LoadReport report;
    std::string line;
    std::size_t lineNumber = 0;

    while (std::getline(file, line)) {
        ++lineNumber;
        const std::string text = trim(line);
        if (text.empty() || text[0] == '#') continue;

        const std::string where = "line " + std::to_string(lineNumber) + ": ";
        std::istringstream fields(text);
        std::string from, to, transport, extra;
        double distance = 0.0, time = 0.0, cost = 0.0;

        if (!(fields >> from >> to >> transport >> distance >> time >> cost) || (fields >> extra)) {
            report.warnings.push_back(where + "expected 'from to transport distance time cost', line skipped");
            continue;
        }
        if (distance < 0.0 || time < 0.0 || cost < 0.0) {
            report.warnings.push_back(where + "distance, time and cost cannot be negative, line skipped");
            continue;
        }
        if (from == to) {
            report.warnings.push_back(where + "a road must join two different locations, line skipped");
            continue;
        }

        addEdge(from, to, transport, distance, time, cost);
        ++report.edgesLoaded;
    }

    return report;
}

const std::vector<Edge>& Graph::getNeighbors(const std::string& node) const {
    static const std::vector<Edge> noEdges;
    const auto it = adjacency_.find(node);
    return it != adjacency_.end() ? it->second : noEdges;
}

const std::vector<Road>& Graph::getRoads() const {
    return roads_;
}

std::vector<std::string> Graph::getAllNodes() const {
    return std::vector<std::string>(nodeNames_.begin(), nodeNames_.end());
}

std::vector<std::string> Graph::getTransportTypes() const {
    return std::vector<std::string>(transportNames_.begin(), transportNames_.end());
}

bool Graph::hasNode(const std::string& node) const {
    return nodeNames_.count(node) > 0;
}

std::string Graph::resolveNode(const std::string& name) const {
    return resolveIn(nodeNames_, name);
}

std::string Graph::resolveTransport(const std::string& name) const {
    return resolveIn(transportNames_, name);
}
