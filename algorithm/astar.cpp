#include "astar.h"

#include <algorithm>
#include <queue>
#include <tuple>
#include <unordered_map>
#include <vector>

struct Node {
    int x;
    int y;
    int g;
    int h;
    int f;
    int parentX;
    int parentY;
    bool operator>(const Node &other) const { return f > other.f; }
};

std::vector<Cell> astarPath(const WarehouseGrid &grid, Cell start, Cell target) {
    if (!grid.isFree(start.x, start.y) || !grid.isFree(target.x, target.y)) {
        return {};
    }

    const int rows = grid.getRows();
    const int cols = grid.getCols();
    std::vector<std::vector<int>> gScore(rows, std::vector<int>(cols, 1e9));
    std::vector<std::vector<int>> closed(rows, std::vector<int>(cols, 0));
    std::vector<std::vector<Cell>> parent(rows, std::vector<Cell>(cols, {-1, -1}));
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;

    auto heuristic = [&](int x, int y) {
        return std::abs(x - target.x) + std::abs(y - target.y);
    };

    gScore[start.x][start.y] = 0;
    pq.push({start.x, start.y, 0, heuristic(start.x, start.y), heuristic(start.x, start.y), -1, -1});

    while (!pq.empty()) {
        Node cur = pq.top();
        pq.pop();

        if (cur.x == target.x && cur.y == target.y) {
            std::vector<Cell> path;
            Cell c{cur.x, cur.y};
            while (c.x != -1 && c.y != -1) {
                path.push_back(c);
                c = parent[c.x][c.y];
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        if (closed[cur.x][cur.y]) {
            continue;
        }
        closed[cur.x][cur.y] = 1;

        static const int dx[4] = {1, -1, 0, 0};
        static const int dy[4] = {0, 0, 1, -1};

        for (int i = 0; i < 4; ++i) {
            int nx = cur.x + dx[i];
            int ny = cur.y + dy[i];
            if (nx < 0 || ny < 0 || nx >= rows || ny >= cols || !grid.isFree(nx, ny) || closed[nx][ny]) {
                continue;
            }

            int tentative = cur.g + grid.edgeWeight(cur.x, cur.y, nx, ny);
            if (tentative < gScore[nx][ny]) {
                gScore[nx][ny] = tentative;
                parent[nx][ny] = {cur.x, cur.y};
                int h = heuristic(nx, ny);
                pq.push({nx, ny, tentative, h, tentative + h, cur.x, cur.y});
            }
        }
    }

    return {};
}
