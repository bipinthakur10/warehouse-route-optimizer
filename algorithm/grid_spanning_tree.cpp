#include "grid_spanning_tree.h"

#include "minimum_spanning_tree.h"

#include <algorithm>
#include <queue>

namespace {
struct GridGraph {
    std::vector<int> cellToVertex;
    std::vector<Cell> vertexToCell;
    std::vector<WeightedEdge> edges;
};

int cellIndex(const WarehouseGrid &grid, int x, int y)
{
    return x * grid.getCols() + y;
}

GridGraph createFreeCellGraph(const WarehouseGrid &grid)
{
    GridGraph graph;
    graph.cellToVertex.assign(grid.getRows() * grid.getCols(), -1);
    for (int x = 0; x < grid.getRows(); ++x) {
        for (int y = 0; y < grid.getCols(); ++y) {
            if (grid.isFree(x, y)) {
                graph.cellToVertex[cellIndex(grid, x, y)] = static_cast<int>(graph.vertexToCell.size());
                graph.vertexToCell.push_back({x, y});
            }
        }
    }

    static const int dx[2] = {1, 0};
    static const int dy[2] = {0, 1};
    for (int vertex = 0; vertex < static_cast<int>(graph.vertexToCell.size()); ++vertex) {
        const Cell current = graph.vertexToCell[vertex];
        for (int direction = 0; direction < 2; ++direction) {
            const int nextX = current.x + dx[direction];
            const int nextY = current.y + dy[direction];
            if (!grid.isFree(nextX, nextY))
                continue;
            graph.edges.push_back({vertex, graph.cellToVertex[cellIndex(grid, nextX, nextY)],
                                   grid.edgeWeight(current.x, current.y, nextX, nextY)});
        }
    }
    return graph;
}

std::vector<Cell> pathInTree(const GridGraph &graph, const MinimumSpanningTree &tree,
                             int startVertex, int targetVertex)
{
    std::vector<std::vector<int>> adjacency(graph.vertexToCell.size());
    for (const WeightedEdge &edge : tree.edges) {
        adjacency[edge.from].push_back(edge.to);
        adjacency[edge.to].push_back(edge.from);
    }

    std::vector<int> parent(graph.vertexToCell.size(), -1);
    std::queue<int> queue;
    queue.push(startVertex);
    parent[startVertex] = startVertex;
    while (!queue.empty()) {
        const int current = queue.front();
        queue.pop();
        if (current == targetVertex)
            break;
        for (const int next : adjacency[current]) {
            if (parent[next] == -1) {
                parent[next] = current;
                queue.push(next);
            }
        }
    }
    if (parent[targetVertex] == -1)
        return {};

    std::vector<Cell> path;
    for (int current = targetVertex; current != startVertex; current = parent[current])
        path.push_back(graph.vertexToCell[current]);
    path.push_back(graph.vertexToCell[startVertex]);
    std::reverse(path.begin(), path.end());
    return path;
}

template <typename BuildTree>
std::vector<Cell> spanningTreePath(const WarehouseGrid &grid, Cell start, Cell target, BuildTree buildTree)
{
    if (!grid.isFree(start.x, start.y) || !grid.isFree(target.x, target.y))
        return {};
    const GridGraph graph = createFreeCellGraph(grid);
    const int startVertex = graph.cellToVertex[cellIndex(grid, start.x, start.y)];
    const int targetVertex = graph.cellToVertex[cellIndex(grid, target.x, target.y)];
    return pathInTree(graph, buildTree(graph, startVertex), startVertex, targetVertex);
}
} // namespace

std::vector<Cell> kruskalPath(const WarehouseGrid &grid, Cell start, Cell target)
{
    return spanningTreePath(grid, start, target, [](const GridGraph &graph, int) {
        return kruskalMst(static_cast<int>(graph.vertexToCell.size()), graph.edges);
    });
}

std::vector<Cell> primPath(const WarehouseGrid &grid, Cell start, Cell target)
{
    return spanningTreePath(grid, start, target, [](const GridGraph &graph, int startVertex) {
        return primMst(static_cast<int>(graph.vertexToCell.size()), graph.edges, startVertex);
    });
}
