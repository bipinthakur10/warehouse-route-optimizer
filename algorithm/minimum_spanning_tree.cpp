#include "minimum_spanning_tree.h"

#include <algorithm>
#include <numeric>
#include <queue>

namespace {
class DisjointSet {
public:
    explicit DisjointSet(int size) : parent(size), rank(size, 0)
    {
        std::iota(parent.begin(), parent.end(), 0);
    }

    int find(int vertex)
    {
        if (parent[vertex] != vertex)
            parent[vertex] = find(parent[vertex]);
        return parent[vertex];
    }

    bool unite(int first, int second)
    {
        first = find(first);
        second = find(second);
        if (first == second)
            return false;
        if (rank[first] < rank[second])
            std::swap(first, second);
        parent[second] = first;
        if (rank[first] == rank[second])
            ++rank[first];
        return true;
    }

private:
    std::vector<int> parent;
    std::vector<int> rank;
};

bool validEdge(const WeightedEdge &edge, int vertexCount)
{
    return edge.from >= 0 && edge.from < vertexCount && edge.to >= 0 && edge.to < vertexCount;
}
} // namespace

MinimumSpanningTree kruskalMst(int vertexCount, std::vector<WeightedEdge> edges)
{
    MinimumSpanningTree result;
    if (vertexCount <= 0)
        return result;

    std::sort(edges.begin(), edges.end(), [](const WeightedEdge &left, const WeightedEdge &right) {
        return left.weight < right.weight;
    });
    DisjointSet components(vertexCount);
    for (const WeightedEdge &edge : edges) {
        if (validEdge(edge, vertexCount) && components.unite(edge.from, edge.to)) {
            result.edges.push_back(edge);
            result.totalWeight += edge.weight;
            if (static_cast<int>(result.edges.size()) == vertexCount - 1)
                break;
        }
    }
    result.spansAllVertices = vertexCount == 1 || static_cast<int>(result.edges.size()) == vertexCount - 1;
    return result;
}

MinimumSpanningTree primMst(int vertexCount, const std::vector<WeightedEdge> &edges, int startVertex)
{
    MinimumSpanningTree result;
    if (vertexCount <= 0 || startVertex < 0 || startVertex >= vertexCount)
        return result;

    std::vector<std::vector<WeightedEdge>> adjacency(vertexCount);
    for (const WeightedEdge &edge : edges) {
        if (!validEdge(edge, vertexCount))
            continue;
        adjacency[edge.from].push_back(edge);
        adjacency[edge.to].push_back({edge.to, edge.from, edge.weight});
    }

    auto compareWeight = [](const WeightedEdge &left, const WeightedEdge &right) {
        return left.weight > right.weight;
    };
    std::priority_queue<WeightedEdge, std::vector<WeightedEdge>, decltype(compareWeight)> queue(compareWeight);
    std::vector<bool> visited(vertexCount, false);
    visited[startVertex] = true;
    for (const WeightedEdge &edge : adjacency[startVertex])
        queue.push(edge);

    while (!queue.empty() && static_cast<int>(result.edges.size()) < vertexCount - 1) {
        const WeightedEdge edge = queue.top();
        queue.pop();
        if (visited[edge.to])
            continue;
        visited[edge.to] = true;
        result.edges.push_back(edge);
        result.totalWeight += edge.weight;
        for (const WeightedEdge &nextEdge : adjacency[edge.to]) {
            if (!visited[nextEdge.to])
                queue.push(nextEdge);
        }
    }

    result.spansAllVertices = vertexCount == 1 || static_cast<int>(result.edges.size()) == vertexCount - 1;
    return result;
}
