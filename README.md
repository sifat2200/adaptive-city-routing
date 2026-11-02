# Adaptive City Routing System

A command-line route planner for a city road network. It finds the best route between two locations under four strategies: shortest distance, fastest time, cheapest cost, and a custom weighted score. Users can also exclude locations and transport types from the route.

The project is written in C++17 with no external dependencies. It runs on Linux, macOS, Windows, and Google Colab.

---

## Features

- **Four routing strategies**: shortest distance, fastest time, cheapest cost, and a user-defined weighted score.
- **Route constraints**: avoid specific locations or transport types (for example, "avoid Mirpur" or "no Metro").
- **Comparison mode**: runs every strategy for the same trip and prints a summary table.
- **Validated input**: menu choices, numbers, location names, and transport names are checked, and the user is asked again until the input is valid. Names are not case-sensitive.
- **Robust map loading**: malformed lines are skipped with a warning that gives the line number. The program does not crash.
- **Graceful end of input**: works with redirected input files, and exits cleanly when input runs out.
- **Unit tests**: 21 automated checks against hand-verified route results.

---

## Project structure

```
adaptive-city-routing/
├── Makefile                     Build, test, and run targets
├── README.md                    This file
├── .gitignore                   Files excluded from Git
├── include/                     Public interfaces (headers)
│   ├── algorithms.h             Route search API (Criterion, Weights, Constraints, RouteResult)
│   ├── cli.h                    Input prompts and output formatting
│   ├── graph.h                  Road network (Graph, Edge, Road, LoadReport)
│   └── utils.h                  String helpers (trim, lower-case, split, join)
├── src/                         Implementation
│   ├── algorithms.cpp           Dijkstra's algorithm with constraints
│   ├── cli.cpp                  Menu prompts, validation, tables
│   ├── graph.cpp                Map file parser and graph storage
│   └── main.cpp                 Program entry point and menu flow
├── tests/
│   └── test_routing.cpp         Unit tests
├── data/
│   └── city_map.txt             Road network of the sample city
├── examples/
│   ├── demo_input.txt           Scripted session that exercises every feature
│   └── invalid_input_demo.txt   Scripted session that shows input validation
└── notebooks/
    └── Adaptive_City_Routing_Colab.ipynb   Google Colab runner
```

---

## Map file format

The road network is read from a plain-text file. Each non-blank line describes one two-way road:

```
from  to  transport  distance(km)  time(minutes)  cost(taka)
```

Example (`data/city_map.txt`):

```
Uttara  Banani  Bus      12  35  40
Banani  Gulshan Walking   3  25   0
Gulshan Motijheel Bus    12  35  45
```

Rules:

- Fields are separated by spaces or tabs.
- Location names are single words (no spaces). Transport names are single words too.
- Distance, time, and cost must be zero or greater.
- Lines beginning with `#` are comments. Blank lines are ignored.
- Each road can be used in both directions.
- Malformed lines are skipped and reported at startup.

To use your own city, copy `data/city_map.txt`, edit it, and pass the new file as an argument (see below).

---

## Building and running

### Option 1: Linux or macOS (or WSL, or Git Bash with `make`)

Requirements: `g++` with C++17 support (GCC 7 or newer, or Clang 5 or newer) and `make`.

```bash
make            # build adaptive_routing
make run        # build and start with data/city_map.txt
make test       # build and run the unit tests
make clean      # remove built files
```

To use a different map file:

```bash
./adaptive_routing path/to/your_map.txt
```

### Option 2: Windows (MinGW-w64 or MSYS2)

From a terminal in the project folder, without `make`:

```bat
g++ -std=c++17 -Wall -Wextra -Iinclude src/graph.cpp src/algorithms.cpp src/cli.cpp src/main.cpp -o adaptive_routing.exe
adaptive_routing.exe data/city_map.txt
```

Tests on Windows:

```bat
g++ -std=c++17 -Wall -Wextra -Iinclude src/graph.cpp src/algorithms.cpp tests/test_routing.cpp -o run_tests.exe
run_tests.exe data/city_map.txt
```

Note: the original `project.exe` is a compiled Windows binary and is not part of the source project. Rebuild it from the source above rather than committing binaries.

---

## Using the program

When the program starts it loads the map and shows the main menu:

```
  1. Display city map
  2. Shortest distance route
  3. Fastest time route
  4. Cheapest cost route
  5. Custom weighted route
  6. Compare all strategies
  7. Exit
```

For options 2 to 6 the program asks for:

1. **Source location**: one of the listed locations.
2. **Destination location**: one of the listed locations.
3. **Locations to avoid**: comma-separated, or press Enter to skip (for example, `Mirpur, Shahbag`).
4. **Transport types to avoid**: comma-separated, or press Enter to skip (for example, `Walking`).

Option 5 also asks for three non-negative weights. The custom score for each road is:

```
score = distance × wd + time × wt + cost × wc
```

The units differ (km, minutes, taka), so choose weights that match your priorities. At least one weight must be greater than zero.

### Sample session

Input: `2`, then `Uttara`, `Motijheel`, and two blank lines (no restrictions). Output (shortened):

```
--- Shortest distance route ---
Path     : Uttara --[Bus]--> Mirpur --[Metro]--> Shahbag --[Metro]--> Motijheel
Distance : 26.00 km
Time     : 62.00 minutes
Cost     : 90.00 taka
Search   : 0.011 ms
```

Option 6 with the same trip prints every strategy and a comparison table:

```
--- Comparison summary ---
Strategy                    Distance(km)   Time(min)  Cost(taka)  Segments
--------------------------------------------------------------------------
Shortest distance                  26.00       62.00       90.00         3
Fastest time                       26.00       62.00       90.00         3
Cheapest cost                      27.00       95.00       85.00         3
Balanced (0.4/0.4/0.2)             26.00       62.00       90.00         3
```

(Exact figures depend on the map. The table above was produced from the sample map with the values shown.)

---

## Running in Google Colab

1. Push this project to GitHub (see the section below).
2. Open `notebooks/Adaptive_City_Routing_Colab.ipynb` in Colab (File, Upload notebook, or open it from GitHub).
3. In the first code cell, replace `YOUR_USERNAME` with your GitHub username.
4. Choose **Runtime, Run all**, or run the cells in order.

The notebook clones the repository, builds it with `make`, runs the demo, runs a custom input file you can edit, and runs the tests.

Colab notes:

- Colab's `!` shell cells do not reliably accept typed input. The notebook therefore feeds the program from a text file with `< file`. Edit that file and run it again.
- If the repository is private, create a GitHub personal access token and store it in Colab's Secrets panel. Do not paste tokens into notebooks or commit them.

---

## Running the tests

```bash
make test
```

The tests cover the following, using hand-computed expectations from `data/city_map.txt`:

| Test area | What is checked |
|---|---|
| Map loading | 14 segments loaded, no warnings, file order kept |
| Name lookup | Case-insensitive location and transport names; unknown names rejected |
| Shortest distance | 26 km via Mirpur and Shahbag |
| Fastest time | 62 minutes |
| Cheapest cost | 85 taka via Banani and Gulshan (uses a free walking segment) |
| Weighted score | Balanced score of 53.2 |
| Constraints | Avoiding Mirpur (27 km detour); avoiding Mirpur and Walking (120 taka); no route when Bus and Metro are both avoided; no route when the source is avoided |
| Edge cases | Source equal to destination; unknown source |

The program exits with status `0` if all tests pass and `1` otherwise.

---

## How it works

Route search uses **Dijkstra's algorithm** with a min-priority queue.

- Each road has a numeric cost for the chosen criterion: distance, time, cost, or the weighted score.
- The search repeatedly settles the cheapest unvisited location and relaxes its roads.
- Avoided locations and transport types are skipped during relaxation.
- The search stops as soon as the destination is settled.
- The route is rebuilt by following predecessor links back to the source.

Because every road weight is non-negative, Dijkstra's algorithm returns an optimal route. Running time is O((V + E) log V), where V is the number of locations and E is the number of roads.

Each result also reports the total distance, time, and cost along the chosen path, not only the value that was minimised.

---

## Assumptions and limitations

- Roads are two-way, and the same weights apply in both directions.
- Location and transport names must not contain spaces. Use `Gulshan2` or `Gulshan_North` instead of `Gulshan North`.
- The weighted score combines units (km, minutes, taka). It is a relative preference, not a physical quantity.
- Waiting times, traffic, and time-of-day effects are not modelled.
- The search is single-criterion. It does not produce Pareto-optimal alternatives.

---

## Troubleshooting

| Symptom | Likely cause and fix |
|---|---|
| `Error: cannot open map file 'data/city_map.txt'` | Run the program from the project root, or pass the correct path as an argument. |
| `No route found` | Check whether your avoid lists block every path. The comparison table reports this per strategy. |
| Warnings about lines at startup | Fix the reported line in the map file. The line number is shown. |
| `make: command not found` on Windows | Use the MinGW `g++` commands in the Windows section, or install MSYS2. |
| Colab cell says `No such file or directory` | Run the clone cell first, so that the working directory is the project folder. |

---
