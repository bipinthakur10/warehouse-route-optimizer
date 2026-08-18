# Warehouse Robot Route Optimizer

A C++17 web application that finds a robot route through a warehouse grid. The backend uses Drogon and the frontend is a small HTML, CSS, and JavaScript interface.

## Features

- Interactive grid with start, destination, and obstacle cells
- Route algorithms: BFS, DFS, A*, Dijkstra, and Floyd-Warshall
- Minimum-spanning-tree algorithms: Kruskal and Prim
- A mathematical explanation log for every selected algorithm
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

The server listens on `http://127.0.0.1:8080` by default and serves the frontend itself. Open `http://127.0.0.1:8080` in a browser, create a grid, choose an algorithm, and click **Find Path**. No separate Python or frontend server is required.

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
  "end": [4, 4],
  "obstacles": [[1, 1], [2, 2]],
  "algorithm": "Dijkstra"
}
```

Supported values for `algorithm` are:

- `BFS`
- `DFS`
- `A*`
- `Dijkstra`
- `Floyd-Warshall`
- `Kruskal`
- `Prim`

Example response:

```json
{
  "success": true,
  "algorithm": "DIJKSTRA",
  "steps": 8,
  "path": [[0, 0], [0, 1], [0, 2]],
  "algorithmLog": [
    "Formula: candidateDistance = distance[current] + edgeWeight...",
    "Step 1: distance(0, 1) = distance(0, 0) + 1 = 0 + 1 = 1."
  ],
  "message": "Path computed successfully."
}
```

## Important notes

- BFS and Dijkstra give a shortest route on this unit-weight grid.
- A* gives a shortest route using Manhattan distance as its heuristic.
- DFS finds a valid route but does not guarantee the shortest one.
- Floyd-Warshall calculates all-pairs shortest paths and is best for small grids because its running time is `O(V^3)`.
- Kruskal and Prim build a minimum spanning tree of the free cells. The displayed route is the unique path inside that tree, so it is not necessarily the shortest start-to-end route.

See [REPORT.md](REPORT.md) for the complete project documentation.
See [CODE_FLOW.md](CODE_FLOW.md) for the application execution flow.
