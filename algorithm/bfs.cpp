#include "bfs.h"
#include <queue>
#include <algorithm>

std::vector<Cell> bfsPath(const WarehouseGrid& grid, Cell start, Cell target) {
    if (!grid.isFree(start.x, start.y) || !grid.isFree(target.x, target.y)) {
        return {};
    }

    int n = grid.getRows(), m = grid.getCols();
    std::vector<std::vector<bool>> visited(n, std::vector<bool>(m, false));
    std::vector<std::vector<Cell>> parent(n, std::vector<Cell>(m, {-1,-1}));
    std::queue<Cell> q;
    q.push(start);
    visited[start.x][start.y] = true;

    int dx[4] = {1,-1,0,0};
    int dy[4] = {0,0,1,-1};

    while(!q.empty()) {
        Cell cur = q.front(); q.pop();
        if(cur.x == target.x && cur.y == target.y) break;

        for(int i=0;i<4;i++) {
            int nx = cur.x + dx[i], ny = cur.y + dy[i];
            if(grid.isFree(nx,ny) && !visited[nx][ny]) {
                visited[nx][ny] = true;
                parent[nx][ny] = cur;
                q.push({nx,ny});
            }
        }
    }

    // Do not report a one-cell path containing only the target when it was
    // never reached (the previous reconstruction did that for blocked routes).
    if (!visited[target.x][target.y]) {
        return {};
    }

    // reconstruct path
    std::vector<Cell> path;
    Cell cur = target;
    while(cur.x!=-1 && cur.y!=-1) {
        path.push_back(cur);
        cur = parent[cur.x][cur.y];
    }
    std::reverse(path.begin(), path.end());
    return path;
}
