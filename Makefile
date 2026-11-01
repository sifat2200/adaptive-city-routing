# Build file for the Adaptive City Routing System.
#
#   make          build the routing program       -> adaptive_routing
#   make test     build and run the unit tests    -> run_tests
#   make run      build and start the program with the default map
#   make clean    remove built binaries

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -O2
CPPFLAGS  = -Iinclude

APP_TARGET  = adaptive_routing
TEST_TARGET = run_tests
MAP_FILE    = data/city_map.txt

APP_SRCS  = src/graph.cpp src/algorithms.cpp src/cli.cpp src/main.cpp
TEST_SRCS = src/graph.cpp src/algorithms.cpp tests/test_routing.cpp
HEADERS   = $(wildcard include/*.h)

.PHONY: all test run clean

all: $(APP_TARGET)

$(APP_TARGET): $(APP_SRCS) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(APP_SRCS) -o $@

$(TEST_TARGET): $(TEST_SRCS) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(TEST_SRCS) -o $@

test: $(TEST_TARGET)
	./$(TEST_TARGET) $(MAP_FILE)

run: $(APP_TARGET)
	./$(APP_TARGET) $(MAP_FILE)

clean:
	rm -f $(APP_TARGET) $(TEST_TARGET) adaptive_routing.exe run_tests.exe
