#pragma once

#include "../graph/warehousegrid.h"

#include <vector>

// Both functions return an empty vector when the target cannot be reached.
std::vector<Cell> dijkstraPath(const WarehouseGrid &grid, Cell start, Cell target);
std::vector<Cell> floydWarshallPath(const WarehouseGrid &grid, Cell start, Cell target);
