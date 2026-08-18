#include "warehousegrid.h"

#include <algorithm>
#include <random>

WarehouseGrid::WarehouseGrid(int r, int c) : rows(r), cols(c) {
    grid.assign(rows, std::vector<int>(cols, 0));
    horizontalWeights.assign(rows, std::vector<int>(std::max(0, cols - 1), 1));
    verticalWeights.assign(std::max(0, rows - 1), std::vector<int>(cols, 1));
}

void WarehouseGrid::addObstacle(int x, int y)
{
    if (isInside(x, y))
        grid[x][y] = 1;
}

bool WarehouseGrid::isFree(int x, int y) const
{
    return isInside(x, y) && grid[x][y] == 0;
}

bool isInside(int x, int y, int rows, int cols)
{
    return x >= 0 &&
           x < rows &&
           y >= 0 &&
           y < cols;
}
bool WarehouseGrid::isInside(int x, int y) const
{
    return x >= 0 &&
           x < rows &&
           y >= 0 &&
           y < cols;
}
int WarehouseGrid::getRows() const { return rows; }
int WarehouseGrid::getCols() const { return cols; }

void WarehouseGrid::randomizeWeights(int minimumWeight, int maximumWeight)
{
    minimumWeight = std::max(1, minimumWeight);
    maximumWeight = std::max(minimumWeight, maximumWeight);
    std::random_device device;
    std::mt19937 generator(device());
    std::uniform_int_distribution<int> distribution(minimumWeight, maximumWeight);
    for (auto &row : horizontalWeights)
        for (int &weight : row)
            weight = distribution(generator);
    for (auto &row : verticalWeights)
        for (int &weight : row)
            weight = distribution(generator);
}

int WarehouseGrid::edgeWeight(int fromX, int fromY, int toX, int toY) const
{
    if (!isInside(fromX, fromY) || !isInside(toX, toY))
        return 0;
    if (fromX == toX && std::abs(fromY - toY) == 1)
        return horizontalWeights[fromX][std::min(fromY, toY)];
    if (fromY == toY && std::abs(fromX - toX) == 1)
        return verticalWeights[std::min(fromX, toX)][fromY];
    return 0;
}

std::vector<GridEdge> WarehouseGrid::getEdges() const
{
    std::vector<GridEdge> edges;
    for (int x = 0; x < rows; ++x) {
        for (int y = 0; y < cols; ++y) {
            if (!isFree(x, y))
                continue;
            if (isFree(x, y + 1))
                edges.push_back({{x, y}, {x, y + 1}, edgeWeight(x, y, x, y + 1)});
            if (isFree(x + 1, y))
                edges.push_back({{x, y}, {x + 1, y}, edgeWeight(x, y, x + 1, y)});
        }
    }
    return edges;
}
