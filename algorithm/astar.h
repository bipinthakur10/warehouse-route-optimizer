#pragma once

#include "../graph/warehousegrid.h"
#include <vector>

std::vector<Cell> astarPath(const WarehouseGrid &grid, Cell start, Cell target);
