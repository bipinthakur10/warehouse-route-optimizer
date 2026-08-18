cmake_minimum_required(VERSION 3.16)
project(WarehouseSystem)

set(CMAKE_CXX_STANDARD 17)

find_package(Drogon REQUIRED)
find_package(yaml-cpp REQUIRED)

add_executable(server
   main.cpp
   controllers/RouteController.cpp
   controllers/WarehouseController.cpp
   graph/warehousegrid.cpp
   algorithm/bfs.cpp
   algorithm/astar.cpp
   algorithm/dfs.cpp
   algorithm/grid_shortest_paths.cpp
   algorithm/grid_spanning_tree.cpp
   algorithm/minimum_spanning_tree.cpp
)

target_include_directories(server PRIVATE /usr/include/jsoncpp)

target_link_libraries(server PRIVATE Drogon::Drogon yaml-cpp::yaml-cpp)
