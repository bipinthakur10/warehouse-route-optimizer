#pragma once

#include "../graph/warehousegrid.h"

#include <vector>

// Returns one valid route found by depth-first search. It is not guaranteed to be shortest.
std::vector<Cell> dfsPath(const WarehouseGrid &grid, Cell start, Cell target);
