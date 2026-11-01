#ifndef UTILS_H
#define UTILS_H

#include <cctype>
#include <cstddef>
#include <sstream>
#include <string>
#include <vector>

// Small string helpers shared by the map loader and the command-line interface.

// Removes leading and trailing whitespace (spaces, tabs, carriage returns).
inline std::string trim(const std::string& text) {
    const char* whitespace = " \t\r\n";
    const std::size_t first = text.find_first_not_of(whitespace);
    if (first == std::string::npos) return "";
    const std::size_t last = text.find_last_not_of(whitespace);
    return text.substr(first, last - first + 1);
}

// Returns a lower-case copy of the text (ASCII only).
inline std::string toLower(std::string text) {
    for (char& c : text) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return text;
}

// Splits "a, b ,c" into {"a", "b", "c"}. Empty items are dropped.
inline std::vector<std::string> splitCsv(const std::string& text) {
    std::vector<std::string> items;
    std::stringstream stream(text);
    std::string item;
    while (std::getline(stream, item, ',')) {
        item = trim(item);
        if (!item.empty()) items.push_back(item);
    }
    return items;
}

// Joins items with a separator: join({"a","b"}, ", ") -> "a, b".
inline std::string join(const std::vector<std::string>& items, const std::string& separator) {
    std::string result;
    for (std::size_t i = 0; i < items.size(); ++i) {
        if (i > 0) result += separator;
        result += items[i];
    }
    return result;
}

#endif
