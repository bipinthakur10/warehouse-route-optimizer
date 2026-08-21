# Algorithm and Application Flowcharts

 
## 1. Complete Request and Comparison Flow
 
flowchart TD
    A[User configures start, targets, blocks, and selected algorithm] --> B[Click Find Routes]
    B --> C[Frontend sends POST /route]
    C --> D{Request data valid?}
    D -- No --> E[Return HTTP 400 error]
    D -- Yes --> F[Create WarehouseGrid]
    F --> G[Generate one random weight for every free edge]
    G --> H[Run A* over ordered target legs]
    G --> I[Run Dijkstra over ordered target legs]
    G --> J[Run Floyd-Warshall over ordered target legs]
    H --> K[Create A* summary]
    I --> L[Create Dijkstra summary]
    J --> M[Create Floyd-Warshall summary]
    K --> N[Build selected route, log, and comparisons]
    L --> N
    M --> N
    N --> O[Return graph and JSON response]
    O --> P[Draw selected path on graph]
    P --> Q[Render same-graph comparison table]
    Q --> R[Animate marker through each route step]
```

### Key points

- The graph is built once per request.
- The exact same edge weights are passed to all three algorithms.
- Only the selected algorithm's route is highlighted and animated.
- The table still includes all three algorithm results.

## 2. Ordered-Target Flow


flowchart TD
    A[Set current location = start] --> B[Read next ordered target]
    B --> C[Run one selected algorithm for this leg]
    C --> D{Route found?}
    D -- No --> E[Stop and report unreachable target]
    D -- Yes --> F[Store target leg]
    F --> G[Append leg to full route without duplicate joining vertex]
    G --> H[Set current location = reached target]
    H --> I{More targets?}
    I -- Yes --> B
    I -- No --> J[Return complete route, total moves, and total cost]
```

### Key points

- Targets are visited in the exact order chosen by the user.
- A target becomes the next leg's start.
- One failed leg means the complete ordered route fails.
- The API returns both the complete path and each individual target leg.

## 3. A* Search Flow

 
flowchart TD
    A[Initialize gScore to infinity] --> B[Set start gScore = 0]
    B --> C[Push start using f = g + h]
    C --> D{Priority queue empty?}
    D -- Yes --> E[No route]
    D -- No --> F[Pop lowest f node]
    F --> G{Node is target?}
    G -- Yes --> H[Reconstruct route from parent links]
    G -- No --> I{Node already closed?}
    I -- Yes --> D
    I -- No --> J[Close node and inspect neighbours]
    J --> K{Free unclosed neighbour?}
    K -- No --> J
    K -- Yes --> L[Candidate g = current g + edge weight]
    L --> M{Candidate improves gScore?}
    M -- No --> J
    M -- Yes --> N[Save parent and push neighbour with f = g + h]
    N --> J
```

### Key points

- `g` is the real travelled weighted cost.
- `h` is Manhattan distance to the target.
- `f = g + h` determines priority.
- Positive edge weights and no diagonal moves make this heuristic admissible in this project.

## 4. Dijkstra Flow
 
flowchart TD
    A[Set all distances to infinity] --> B[Set distance start = 0]
    B --> C[Push start into min-priority queue]
    C --> D{Priority queue empty?}
    D -- Yes --> E[No route]
    D -- No --> F[Pop lowest-distance entry]
    F --> G{Entry equals current best distance?}
    G -- No --> D
    G -- Yes --> H{Node is target?}
    H -- Yes --> I[Reconstruct route from parent links]
    H -- No --> J[Inspect free orthogonal neighbours]
    J --> K[Candidate = current distance + edge weight]
    K --> L{Candidate improves distance?}
    L -- No --> J
    L -- Yes --> M[Update distance and parent; push neighbour]
    M --> J
```

### Key points

- Dijkstra does not use a heuristic.
- It repeatedly finalizes the lowest known weighted distance.
- Stale priority-queue entries are ignored.
- It is correct because every edge weight is positive.

## 5. Floyd–Warshall Flow
 
flowchart TD
    A[Create distance and next-hop matrices] --> B[Set distances to infinity]
    B --> C[Set free vertex diagonals to 0]
    C --> D[Set direct edge distances and next hops]
    D --> E[Choose intermediate vertex k]
    E --> F[Choose source vertex i]
    F --> G[Choose destination vertex j]
    G --> H{Both partial routes exist?}
    H -- No --> G
    H -- Yes --> I[Candidate = distance i,k + distance k,j]
    I --> J{Candidate is cheaper?}
    J -- Yes --> K[Update distance i,j and next hop]
    J -- No --> G
    K --> G
    G --> L{All j values checked?}
    L -- No --> G
    L -- Yes --> M{All i values checked?}
    M -- No --> F
    M -- Yes --> N{All k values checked?}
    N -- No --> E
    N -- Yes --> O{Next hop from start to target exists?}
    O -- No --> P[No route]
    O -- Yes --> Q[Follow next hops to reconstruct route]
```

### Key points

- Floyd–Warshall calculates distances between every pair of vertices.
- `next[i][j]` stores how to reconstruct the path from `i` to `j`.
- It is practical only on small graphs because it uses cubic time.
- In this application it uses the same weighted graph as A* and Dijkstra.

## 6. Reading the Comparison Table

| Field | Meaning |
|---|---|
| Algorithm | A*, Dijkstra, or Floyd–Warshall. |
| Status | Whether every ordered target was reached. |
| Moves | Number of edges in the final joined route. |
| Cost | Sum of all random edge weights in that route. |

For the same request, all successful rows should have the same minimum cost. Their route shape can differ only when multiple routes have equal cost.
