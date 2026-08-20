# Project Report: Warehouse Robot Route Optimizer

## 1. Introduction

The Warehouse Robot Route Optimizer is a graph-algorithm visualizer for a two-dimensional warehouse. Each free cell is treated as a graph vertex. A robot may move up, down, left, or right to an adjacent free cell. Obstacles are excluded from the graph.

The project demonstrates route-search algorithms and minimum-spanning-tree (MST) algorithms through a Drogon C++ backend and a browser frontend. The result includes a visual route and a calculation log showing the relevant formula and values at each path step.

## 2. Objectives

1. Find routes from a selected start cell through one or more ordered target cells while avoiding obstacles.
2. Compare graph algorithms on the same warehouse grid.
3. Show the mathematical calculation behind each algorithm.
4. Provide an API that a frontend or another application can use.

## 3. System design

```text
Browser frontend
       |
       | POST /route (JSON grid, cells, algorithm)
       v
Drogon RouteController
       | 
       +--> WarehouseGrid: validates free cells and obstacles
       |
       +--> Search algorithms: BFS, DFS, A*, Dijkstra, Floyd-Warshall
       |
       +--> MST algorithms: Kruskal, Prim
       |
       v
JSON response: path, steps, message, algorithmLog
```

### Main source modules

| Module | Responsibility |
|---|---|
| `graph/warehousegrid.*` | Stores the grid and obstacle information. |
| `algorithm/bfs.*` | Breadth-first shortest route. |
| `algorithm/dfs.*` | Depth-first valid route. |
| `algorithm/astar.*` | Heuristic shortest route. |
| `algorithm/grid_shortest_paths.*` | Dijkstra and Floyd-Warshall routes. |
| `algorithm/minimum_spanning_tree.*` | Generic Kruskal and Prim MST implementations. |
| `algorithm/grid_spanning_tree.*` | Converts the warehouse grid into a graph for Kruskal and Prim. |
| `controllers/RouteController.cpp` | Validates input, selects an algorithm, and creates the explanation log. |

## 4. Graph model

For a grid containing `R` rows and `C` columns, each free cell `(x, y)` is a vertex. An edge exists between two orthogonally adjacent free cells. Every movement has weight 1:

```text
w((x, y), (nx, ny)) = 1
```

Diagonal movement is not allowed. The graph is undirected for MST algorithms and may be traversed in either direction for route algorithms.

## 5. Implemented algorithms

### 5.1 Breadth-First Search (BFS)

BFS uses a queue and explores all cells at the current distance before moving to the next distance. Because every grid movement has the same cost, BFS produces a shortest route.

```text
distance[v] = distance[u] + 1
```

Where `u` is the current cell and `v` is an unvisited adjacent cell.

- Time complexity: `O(V + E)`
- Space complexity: `O(V)`
- Result: shortest route in an unweighted grid

### 5.2 Depth-First Search (DFS)

DFS follows one unvisited branch as deeply as possible. If it reaches a dead end, it backtracks to the latest cell that has an unvisited neighbour.

```text
DFS(u): mark u visited; for each unvisited neighbour v, call DFS(v)
```

- Time complexity: `O(V + E)`
- Space complexity: `O(V)`
- Result: a valid route, not necessarily the shortest route

### 5.3 A* Search

A* uses a priority queue ordered by total estimated route cost.

```text
f(n) = g(n) + h(n)
h(n) = |x - targetX| + |y - targetY|
```

`g(n)` is the travelled cost from the start and `h(n)` is Manhattan distance to the target. On this grid, Manhattan distance never overestimates the remaining route cost, so A* produces a shortest route.

- Typical time complexity: `O(V log V)`
- Space complexity: `O(V)`
- Result: shortest route

### 5.4 Dijkstra's algorithm

Dijkstra stores the current best distance to every cell. It always processes the unprocessed cell with the smallest distance.

```text
candidate = distance[u] + weight(u, v)
distance[v] = min(distance[v], candidate)
```

This update is called relaxation. Since all edge weights are non-negative (and equal to 1), Dijkstra is valid and produces a shortest route.

- Time complexity: `O((V + E) log V)` using a priority queue
- Space complexity: `O(V)`
- Result: shortest route

### 5.5 Floyd-Warshall algorithm

Floyd-Warshall computes shortest distances between all pairs of cells. Each cell `k` is considered as an intermediate cell between cells `i` and `j`.

```text
D[i][j] = min(D[i][j], D[i][k] + D[k][j])
```

The implementation stores a `next` matrix alongside the distance matrix to reconstruct the selected start-to-end route.

- Time complexity: `O(V^3)`
- Space complexity: `O(V^2)`
- Result: shortest route; appropriate for small grids

### 5.6 Kruskal's algorithm

Kruskal constructs an MST by sorting all edges by increasing weight. It accepts an edge only when it does not create a cycle. The implementation uses a Disjoint Set Union (Union-Find) structure to detect cycles.

```text
Sort edges by weight
If Find(u) != Find(v):
    add (u, v) to MST
    Union(u, v)
```

- Time complexity: `O(E log E)`
- Space complexity: `O(V)`
- Result: minimum spanning tree

### 5.7 Prim's algorithm

Prim grows one MST from a selected starting vertex. It chooses the smallest edge from the current tree to any unvisited vertex.

```text
key[v] = min(key[v], weight(u, v))
```

- Time complexity: `O(E log V)` using a priority queue
- Space complexity: `O(V + E)`
- Result: minimum spanning tree

### MST behaviour in this application

Kruskal and Prim solve a different problem from shortest-path algorithms: they minimize the total weight required to connect all free cells. For visualization, the application builds an MST and returns the start-to-end path within that tree. This route is valid, but it is not guaranteed to be shortest.

## 6. Explanation log

The response includes `algorithmLog`, an array displayed in the frontend. It records calculations for the returned route.

Examples:

```text
DFS: Step 3: depth(2, 1) = 3.
Dijkstra: distance(0, 2) = distance(0, 1) + 1 = 1 + 1 = 2.
A*: g = 2, h = |0 - 4| + |2 - 4| = 6, f = 2 + 6 = 8.
Floyd-Warshall: D(0, 0)(0, 2) = D(0, 0)(0, 1) + D(0, 1)(0, 2) = 1 + 1 = 2.
Kruskal: accept edge (0, 1) - (0, 2), weight = 1.
Prim: key(0, 2) = min(infinity, weight(0, 1)(0, 2)) = 1.
```

## 7. API documentation

### `POST /route`

Request fields:

| Field | Type | Meaning |
|---|---|---|
| `rows` | integer | Number of grid rows; must be positive. |
| `cols` | integer | Number of grid columns; must be positive. |
| `start` | `[x, y]` | Starting cell. |
| `targets` | `[[x, y], ...]` | One to eight destination cells, visited in order. |
| `end` | `[x, y]` | Legacy single destination field; accepted when `targets` is omitted. |
| `obstacles` | `[[x, y], ...]` | Cells the robot cannot enter. |
| `algorithm` | string | Selected algorithm name. |

Successful response fields:

| Field | Meaning |
|---|---|
| `success` | Whether a route was found. |
| `algorithm` | Algorithm executed by the backend. |
| `steps` | Number of moves in the returned route. |
| `path` | Ordered list of `[x, y]` cells. |
| `targetPaths` | A separate ordered path for each reached target. |
| `algorithmLog` | Formula-based calculation log. |
| `message` | Success or failure message. |

## 8. Build, execution, and test

Build the project:

```bash
cmake -S . -B build
cmake --build build
```

Run the server:

```bash
./build/server
```

The configuration file sets port `8080`. The frontend sends requests to `http://127.0.0.1:8080/route`.

## 9. Limitations and future work

- Floyd-Warshall is expensive on large grids; its matrix requires `O(V^2)` memory and its loops require `O(V^3)` time.
- Every current grid edge has weight 1. A future version can support weighted terrain, such as a higher cost for congested or unsafe cells.
- DFS and MST routes should not be used when shortest travel distance is required.
- The visualization currently highlights the final route. A future version can animate each queue, stack, priority-queue, or matrix update.

## 10. Conclusion

The project combines seven fundamental graph algorithms in one warehouse-grid application. It demonstrates both shortest-path and spanning-tree concepts, provides a usable browser interface, and explains the mathematical calculation used by each selected algorithm.
