#pragma once

#include <vector>

struct Cell {
    int x, y;
};

// An undirected connection between two adjacent free grid cells.
struct GridEdge {
    Cell from;
    Cell to;
    int weight;
};

class WarehouseGrid {
public:
    WarehouseGrid(int rows, int cols);

    void addObstacle(int x, int y);

    bool isFree(int x, int y) const;

    // NEW: Check whether a coordinate is inside the grid
    bool isInside(int x, int y) const;

    int getRows() const;
    int getCols() const;

    // Gives every horizontal and vertical grid connection a random cost in the
    // inclusive range [minimumWeight, maximumWeight].
    void randomizeWeights(int minimumWeight = 1, int maximumWeight = 9);
    int edgeWeight(int fromX, int fromY, int toX, int toY) const;
    std::vector<GridEdge> getEdges() const;

private:
    int rows, cols;
    std::vector<std::vector<int>> grid; // 0 = free, 1 = obstacle
    std::vector<std::vector<int>> horizontalWeights;
    std::vector<std::vector<int>> verticalWeights;
};
