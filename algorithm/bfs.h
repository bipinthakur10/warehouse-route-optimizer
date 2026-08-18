#include <bits/stdc++.h>
#pragma once
#include "../graph/warehousegrid.h"
#include <vector>

using namespace std;

std::vector<Cell> bfsPath(const WarehouseGrid& grid, Cell start, Cell target);
