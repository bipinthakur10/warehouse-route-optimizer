# Code Flow

```mermaid
flowchart TD
    A[User opens frontend/index.html] --> B[createGrid in frontend/script.js]
    B --> C[Build cells, start, end, and obstacle state]
    C --> D[User clicks Find Path]
    D --> E[findPath creates JSON request]
    E --> F[POST /route to Drogon server]

    F --> G[main.cpp route handler]
    G --> H[RouteController::computeRoute]
    H --> I{Valid JSON and required fields?}
    I -- No --> J[Return HTTP 400 JSON error]
    I -- Yes --> K[Create WarehouseGrid]
    K --> L{Coordinates in bounds and free?}
    L -- No --> J
    L -- Yes --> M[Normalize selected algorithm]
    M --> N{Start equals target?}
    N -- Yes --> O[Return one-cell path]
    N -- No --> P{Selected algorithm}

    P -->|BFS| Q[bfsPath]
    P -->|DFS| R[dfsPath]
    P -->|A*| S[astarPath]
    P -->|Dijkstra| T[dijkstraPath]
    P -->|Floyd-Warshall| U[floydWarshallPath]
    P -->|Kruskal| V[kruskalPath]
    P -->|Prim| W[primPath]
    P -->|Unsupported| J

    Q --> X[Build response: path, steps, algorithm log]
    R --> X
    S --> X
    T --> X
    U --> X
    V --> X
    W --> X
    O --> X
    X --> Y[Return JSON response]
    Y --> Z[Frontend checks result.success]
    Z -->|true| AA[drawPath and display steps/log]
    Z -->|false| AB[Display error or no-path message]
```

## Request/response flow

1. `frontend/script.js` stores the grid size, start/end cells, and obstacles.
2. `findPath()` sends those values plus the algorithm name to `POST /route`.
3. `main.cpp` forwards the request to `RouteController::computeRoute()`.
4. The controller validates input and converts obstacles into a `WarehouseGrid`.
5. It calls the corresponding algorithm function in `algorithm/`.
6. The resulting `vector<Cell>` becomes a JSON `path`, with `steps` and an explanation log.
7. The frontend draws the returned path and updates the status panel.

## Core data flow

```text
Grid form + clicked obstacles
        -> JavaScript request JSON
        -> RouteController validation
        -> WarehouseGrid (free cells / blocked cells)
        -> Pathfinding algorithm
        -> vector<Cell> path
        -> JSON response
        -> browser path rendering
```
