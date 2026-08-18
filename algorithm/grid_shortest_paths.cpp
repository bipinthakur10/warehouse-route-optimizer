#include "grid_shortest_paths.h"

#include <algorithm>
#include <limits>
#include <queue>
#include <utility>

namespace {
constexpr int kInfinity = std::numeric_limits<int>::max() / 4;

bool validEndpoints(const WarehouseGrid &grid, Cell start, Cell target)
{
    return grid.isFree(start.x, start.y) && grid.isFree(target.x, target.y);
}

int cellId(const WarehouseGrid &grid, int x, int y)
{
    return x * grid.getCols() + y;
}

Cell cellFromId(const WarehouseGrid &grid, int id)
{
    return {id / grid.getCols(), id % grid.getCols()};
}

std::vector<Cell> buildPath(const WarehouseGrid &grid, const std::vector<int> &parent,
                            int startId, int targetId)
{
    if (parent[targetId] == -1 && startId != targetId)
        return {};

    std::vector<Cell> path;
    for (int current = targetId; current != -1; current = parent[current])
        path.push_back(cellFromId(grid, current));

    std::reverse(path.begin(), path.end());
    return path;
}
} // namespace

std::vector<Cell> dijkstraPath(const WarehouseGrid &grid, Cell start, Cell target)
{
    if (!validEndpoints(grid, start, target))
        return {};

    const int rows = grid.getRows();
    const int cols = grid.getCols();
    const int vertexCount = rows * cols;
    const int startId = cellId(grid, start.x, start.y);
    const int targetId = cellId(grid, target.x, target.y);
    std::vector<int> distance(vertexCount, kInfinity);
    std::vector<int> parent(vertexCount, -1);
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>,
                        std::greater<std::pair<int, int>>> queue;

    distance[startId] = 0;
    queue.push({0, startId});
    static const int dx[4] = {1, -1, 0, 0};
    static const int dy[4] = {0, 0, 1, -1};

    while (!queue.empty()) {
        const auto [currentDistance, currentId] = queue.top();
        queue.pop();
        if (currentDistance != distance[currentId])
            continue;
        if (currentId == targetId)
            break;

        const Cell current = cellFromId(grid, currentId);
        for (int direction = 0; direction < 4; ++direction) {
            const int nextX = current.x + dx[direction];
            const int nextY = current.y + dy[direction];
            if (!grid.isFree(nextX, nextY))
                continue;

            const int nextId = cellId(grid, nextX, nextY);
            const int candidateDistance = currentDistance + grid.edgeWeight(current.x, current.y, nextX, nextY);
            if (candidateDistance < distance[nextId]) {
                distance[nextId] = candidateDistance;
                parent[nextId] = currentId;
                queue.push({candidateDistance, nextId});
            }
        }
    }

    return buildPath(grid, parent, startId, targetId);
}

std::vector<Cell> floydWarshallPath(const WarehouseGrid &grid, Cell start, Cell target)
{
    if (!validEndpoints(grid, start, target))
        return {};

    const int rows = grid.getRows();
    const int cols = grid.getCols();
    const int vertexCount = rows * cols;
    const int startId = cellId(grid, start.x, start.y);
    const int targetId = cellId(grid, target.x, target.y);
    std::vector<std::vector<int>> distance(vertexCount, std::vector<int>(vertexCount, kInfinity));
    std::vector<std::vector<int>> next(vertexCount, std::vector<int>(vertexCount, -1));
    static const int dx[4] = {1, -1, 0, 0};
    static const int dy[4] = {0, 0, 1, -1};

    for (int id = 0; id < vertexCount; ++id) {
        const Cell current = cellFromId(grid, id);
        if (!grid.isFree(current.x, current.y))
            continue;
        distance[id][id] = 0;
        for (int direction = 0; direction < 4; ++direction) {
            const int nextX = current.x + dx[direction];
            const int nextY = current.y + dy[direction];
            if (!grid.isFree(nextX, nextY))
                continue;
            const int neighbour = cellId(grid, nextX, nextY);
            distance[id][neighbour] = grid.edgeWeight(current.x, current.y, nextX, nextY);
            next[id][neighbour] = neighbour;
        }
    }

    for (int through = 0; through < vertexCount; ++through) {
        for (int from = 0; from < vertexCount; ++from) {
            if (distance[from][through] == kInfinity)
                continue;
            for (int to = 0; to < vertexCount; ++to) {
                if (distance[through][to] == kInfinity)
                    continue;
                const int candidate = distance[from][through] + distance[through][to];
                if (candidate < distance[from][to]) {
                    distance[from][to] = candidate;
                    next[from][to] = next[from][through];
                }
            }
        }
    }

    if (next[startId][targetId] == -1 && startId != targetId)
        return {};

    std::vector<Cell> path{start};
    for (int current = startId; current != targetId;) {
        current = next[current][targetId];
        if (current == -1)
            return {};
        path.push_back(cellFromId(grid, current));
    }
    return path;
}
