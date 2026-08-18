#include "dfs.h"

namespace {
bool search(const WarehouseGrid &grid, Cell current, Cell target,
            std::vector<std::vector<bool>> &visited, std::vector<Cell> &path)
{
    visited[current.x][current.y] = true;
    path.push_back(current);
    if (current.x == target.x && current.y == target.y)
        return true;

    static const int dx[4] = {1, -1, 0, 0};
    static const int dy[4] = {0, 0, 1, -1};
    for (int direction = 0; direction < 4; ++direction) {
        const int nextX = current.x + dx[direction];
        const int nextY = current.y + dy[direction];
        if (grid.isFree(nextX, nextY) && !visited[nextX][nextY] &&
            search(grid, {nextX, nextY}, target, visited, path)) {
            return true;
        }
    }

    path.pop_back();
    return false;
}
} // namespace

std::vector<Cell> dfsPath(const WarehouseGrid &grid, Cell start, Cell target)
{
    if (!grid.isFree(start.x, start.y) || !grid.isFree(target.x, target.y))
        return {};

    std::vector<std::vector<bool>> visited(
        grid.getRows(), std::vector<bool>(grid.getCols(), false));
    std::vector<Cell> path;
    if (!search(grid, start, target, visited, path))
        return {};
    return path;
}
