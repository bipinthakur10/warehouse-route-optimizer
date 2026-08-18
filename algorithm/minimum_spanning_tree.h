#pragma once

#include <vector>

struct WeightedEdge {
    int from;
    int to;
    int weight;
};

struct MinimumSpanningTree {
    std::vector<WeightedEdge> edges;
    int totalWeight = 0;
    bool spansAllVertices = false;
};

// `vertexCount` uses vertices numbered 0 through vertexCount - 1.
MinimumSpanningTree kruskalMst(int vertexCount, std::vector<WeightedEdge> edges);
MinimumSpanningTree primMst(int vertexCount, const std::vector<WeightedEdge> &edges,
                            int startVertex = 0);
