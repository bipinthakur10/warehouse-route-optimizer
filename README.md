# Warehouse Robot Route Optimizer

A C++17 web application that finds a robot route through a warehouse grid. The backend uses Drogon and the frontend is a small HTML, CSS, and JavaScript interface.

## Features

- Interactive route graph with start, ordered target, and blocked nodes
- Up to eight targets in one request, visited in the order supplied
- Animated point-by-point route simulation with replay and speed controls
- Algorithms: A*, Dijkstra, and Floyd-Warshall
- One shared-graph comparison table with route status, moves, and weighted cost
- A mathematical explanation log for the selected algorithm
- JSON API at `POST /route`

## Requirements

- CMake 3.16 or newer
- C++17 compiler
- Drogon
- yaml-cpp
- jsoncpp

## Build and run

```bash
cd /home/bipin-kumar/C_Programming/DSALAB
cmake -S . -B build
cmake --build build
./build/server
```

The server listens on `http://127.0.0.1:8080` by default and serves the frontend itself. Open `http://127.0.0.1:8080` in a browser, choose a node action, click graph nodes to set the start, targets, or blocks, choose an algorithm, and click **Find Routes**. The graph then shows the selected route and a comparison table for all three algorithms using the same edge weights. No separate Python or frontend server is required.

If port 8080 is occupied, stop the older server process before starting the new build:

```bash
pkill -f '/build/server'
```

## API usage

Send a `POST` request to `http://127.0.0.1:8080/route`.

```json
{
  "rows": 5,
  "cols": 5,
  "start": [0, 0],
  "targets": [[0, 4], [4, 4]],
  "obstacles": [[1, 1], [2, 2]],
  "algorithm": "Dijkstra"
}
```

Supported values for `algorithm` are:

- `A*`
- `Dijkstra`
- `Floyd-Warshall`

Example response:

```json
{
  "success": true,
  "algorithm": "DIJKSTRA",
  "steps": 8,
  "cost": 23,
  "path": [[0, 0], [0, 1], [0, 2]],
  "targetPaths": [
    [[0, 0], [0, 1], [0, 2], [0, 3], [0, 4]],
    [[0, 4], [1, 4], [2, 4], [3, 4], [4, 4]]
  ],
  "comparisons": [
    {"algorithm": "A*", "success": true, "steps": 8, "cost": 23},
    {"algorithm": "DIJKSTRA", "success": true, "steps": 8, "cost": 23},
    {"algorithm": "FLOYD-WARSHALL", "success": true, "steps": 8, "cost": 23}
  ],
  "algorithmLog": [
    "Formula: candidateDistance = distance[current] + edgeWeight...",
    "Step 1: distance(0, 1) = distance(0, 0) + 1 = 0 + 1 = 1."
  ],
  "message": "Path computed successfully."
}
```

`targets` accepts one to eight `[row, col]` coordinates. Each target becomes the start of the next leg. For compatibility, a request with the former single `end` field is still accepted.

## Important notes

- A*, Dijkstra, and Floyd-Warshall find a minimum-cost route on the positive-weight graph.
- Every comparison result is calculated from the same graph and edge weights in a single API request.
- Floyd-Warshall calculates all-pairs shortest paths and is best for small graphs because its running time is `O(V^3)`.

See [REPORT.md](REPORT.md) for the complete project documentation.
See [CODE_FLOW.md](CODE_FLOW.md) for the application execution flow.
