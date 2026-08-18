#pragma once

#include "../graph/warehousegrid.h"

#include <vector>

// The returned route follows the minimum spanning tree of all free grid cells.
std::vector<Cell> kruskalPath(const WarehouseGrid &grid, Cell start, Cell target);
std::vector<Cell> primPath(const WarehouseGrid &grid, Cell start, Cell target);
