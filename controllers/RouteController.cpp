#include "RouteController.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include "../algorithm/astar.h"
#include "../algorithm/grid_shortest_paths.h"
#include "../graph/warehousegrid.h"
#include <drogon/HttpResponse.h>
#include <jsoncpp/json/json.h>

namespace {
constexpr int kMaximumTargets = 8;

struct RouteSummary {
    std::vector<Cell> path;
    std::vector<std::vector<Cell>> targetPaths;
    bool success = false;
    Cell unreachableTarget{-1, -1};
    int cost = 0;
};

std::string coordinate(const Cell &cell)
{
    return "(" + std::to_string(cell.x) + ", " + std::to_string(cell.y) + ")";
}

Json::Value pathAsJson(const std::vector<Cell> &path)
{
    Json::Value jsonPath(Json::arrayValue);
    for (const Cell &cell : path) {
        Json::Value point(Json::arrayValue);
        point.append(cell.x);
        point.append(cell.y);
        jsonPath.append(point);
    }
    return jsonPath;
}

bool readCell(const Json::Value &value, Cell &cell)
{
    if (!value.isArray() || value.size() != 2 || !value[0].isInt() || !value[1].isInt())
        return false;
    cell = {value[0].asInt(), value[1].asInt()};
    return true;
}

Json::Value graphAsJson(const WarehouseGrid &grid)
{
    Json::Value graph;
    graph["nodes"] = Json::arrayValue;
    graph["edges"] = Json::arrayValue;
    for (int x = 0; x < grid.getRows(); ++x) {
        for (int y = 0; y < grid.getCols(); ++y) {
            if (!grid.isFree(x, y))
                continue;
            Json::Value node(Json::arrayValue);
            node.append(x);
            node.append(y);
            graph["nodes"].append(node);
        }
    }
    for (const GridEdge &edge : grid.getEdges()) {
        Json::Value item;
        item["from"] = Json::arrayValue;
        item["from"].append(edge.from.x);
        item["from"].append(edge.from.y);
        item["to"] = Json::arrayValue;
        item["to"].append(edge.to.x);
        item["to"].append(edge.to.y);
        item["weight"] = edge.weight;
        graph["edges"].append(item);
    }
    return graph;
}

int routeCost(const WarehouseGrid &grid, const std::vector<Cell> &path)
{
    int cost = 0;
    for (std::size_t index = 1; index < path.size(); ++index)
        cost += grid.edgeWeight(path[index - 1].x, path[index - 1].y,
                                path[index].x, path[index].y);
    return cost;
}

std::vector<Cell> findLeg(const WarehouseGrid &grid, const std::string &algorithm,
                          Cell start, Cell target)
{
    if (start.x == target.x && start.y == target.y)
        return {start};
    if (algorithm == "A*")
        return astarPath(grid, start, target);
    if (algorithm == "DIJKSTRA")
        return dijkstraPath(grid, start, target);
    return floydWarshallPath(grid, start, target);
}

RouteSummary calculateRoute(const WarehouseGrid &grid, const std::string &algorithm,
                            Cell start, const std::vector<Cell> &targets)
{
    RouteSummary result;
    Cell current = start;
    for (const Cell &target : targets) {
        std::vector<Cell> leg = findLeg(grid, algorithm, current, target);
        if (leg.empty()) {
            result.unreachableTarget = target;
            return result;
        }
        result.targetPaths.push_back(leg);
        if (result.path.empty())
            result.path = leg;
        else
            result.path.insert(result.path.end(), std::next(leg.begin()), leg.end());
        current = target;
    }
    result.success = true;
    result.cost = routeCost(grid, result.path);
    return result;
}

Json::Value createAlgorithmLog(const std::string &algorithm, const std::vector<Cell> &path,
                               bool routeFound)
{
    Json::Value log(Json::arrayValue);
    if (algorithm == "A*") {
        log.append("Formula: f(n) = g(n) + h(n), where g is the accumulated edge cost and h is the Manhattan-distance estimate.");
        if (!path.empty()) {
            const Cell target = path.back();
            for (std::size_t index = 0; index < path.size(); ++index) {
                const int h = std::abs(path[index].x - target.x) + std::abs(path[index].y - target.y);
                log.append("Step " + std::to_string(index) + ": at " + coordinate(path[index]) +
                           ", h = " + std::to_string(h) + "; g uses the labelled graph-edge costs.");
            }
        }
    } else if (algorithm == "DIJKSTRA") {
        log.append("Formula: candidateDistance = distance[current] + edgeWeight. Edge weights are the random labels shown in the graph.");
        for (std::size_t index = 0; index < path.size(); ++index) {
            if (index == 0)
                log.append("Step 0: distance" + coordinate(path[index]) + " = 0.");
            else
                log.append("Step " + std::to_string(index) + ": relax edge " + coordinate(path[index - 1]) +
                           " - " + coordinate(path[index]) + " using its displayed weight.");
        }
    } else {
        log.append("Formula: D[i][j] = min(D[i][j], D[i][k] + D[k][j]); direct-edge costs use the graph's random labels.");
        for (std::size_t index = 1; index < path.size(); ++index) {
            log.append("Step " + std::to_string(index) + ": update the route to " + coordinate(path[index]) +
                       " through " + coordinate(path[index - 1]) + ".");
        }
    }

    if (routeFound)
        log.append("Result: route found with " + std::to_string(path.empty() ? 0 : path.size() - 1) + " total moves.");
    else
        log.append("Result: no route exists to one of the selected targets.");
    return log;
}

Json::Value comparisonAsJson(const std::string &algorithm, const RouteSummary &route)
{
    Json::Value comparison;
    comparison["algorithm"] = algorithm;
    comparison["success"] = route.success;
    comparison["steps"] = route.success ? static_cast<int>(route.path.size()) - 1 : 0;
    comparison["cost"] = route.success ? route.cost : Json::Value(Json::nullValue);
    return comparison;
}

void sendError(const std::function<void(const drogon::HttpResponsePtr &)> &callback,
               const std::string &message)
{
    Json::Value error;
    error["success"] = false;
    error["message"] = message;
    auto response = drogon::HttpResponse::newHttpJsonResponse(error);
    response->setStatusCode(drogon::k400BadRequest);
    callback(response);
}
} // namespace

void RouteController::computeRoute(
    const drogon::HttpRequestPtr &req,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    if (!req || !req->getJsonObject()) {
        sendError(callback, "Invalid JSON request.");
        return;
    }

    const auto json = req->getJsonObject();
    for (const std::string &field : {"rows", "cols", "start", "obstacles"}) {
        if (!json->isMember(field)) {
            sendError(callback, "Missing required field: " + field);
            return;
        }
    }

    const int rows = (*json)["rows"].asInt();
    const int cols = (*json)["cols"].asInt();
    if (rows <= 0 || cols <= 0) {
        sendError(callback, "Rows and cols must be greater than zero.");
        return;
    }

    WarehouseGrid grid(rows, cols);
    if (!(*json)["obstacles"].isArray()) {
        sendError(callback, "Obstacles must be a list of coordinate pairs.");
        return;
    }
    for (const Json::Value &jsonObstacle : (*json)["obstacles"]) {
        Cell obstacle;
        if (!readCell(jsonObstacle, obstacle) || !grid.isInside(obstacle.x, obstacle.y)) {
            sendError(callback, "Every obstacle must be an in-bounds coordinate pair.");
            return;
        }
        grid.addObstacle(obstacle.x, obstacle.y);
    }

    Cell start;
    if (!readCell((*json)["start"], start) || !grid.isInside(start.x, start.y)) {
        sendError(callback, "Start must be an in-bounds coordinate pair [row, col].");
        return;
    }
    if (!grid.isFree(start.x, start.y)) {
        sendError(callback, "Invalid start position (occupied).");
        return;
    }

    std::vector<Cell> targets;
    if (json->isMember("targets")) {
        const Json::Value &jsonTargets = (*json)["targets"];
        if (!jsonTargets.isArray() || jsonTargets.empty() || jsonTargets.size() > kMaximumTargets) {
            sendError(callback, "Targets must contain between 1 and 8 coordinate pairs.");
            return;
        }
        for (const Json::Value &jsonTarget : jsonTargets) {
            Cell target;
            if (!readCell(jsonTarget, target) || !grid.isInside(target.x, target.y)) {
                sendError(callback, "Every target must be an in-bounds coordinate pair.");
                return;
            }
            if (!grid.isFree(target.x, target.y)) {
                sendError(callback, "Invalid target position (occupied).");
                return;
            }
            targets.push_back(target);
        }
    } else if (json->isMember("end")) {
        Cell target;
        if (!readCell((*json)["end"], target) || !grid.isInside(target.x, target.y)) {
            sendError(callback, "End must be an in-bounds coordinate pair [row, col].");
            return;
        }
        if (!grid.isFree(target.x, target.y)) {
            sendError(callback, "Invalid target position (occupied).");
            return;
        }
        targets.push_back(target);
    } else {
        sendError(callback, "Missing required field: targets.");
        return;
    }

    std::string algorithm = json->isMember("algorithm") ? (*json)["algorithm"].asString() : "A*";
    std::transform(algorithm.begin(), algorithm.end(), algorithm.begin(),
                   [](unsigned char character) { return static_cast<char>(std::toupper(character)); });
    if (algorithm == "FLOYD_WARSHALL")
        algorithm = "FLOYD-WARSHALL";

    const std::vector<std::string> algorithms{"A*", "DIJKSTRA", "FLOYD-WARSHALL"};
    if (std::find(algorithms.begin(), algorithms.end(), algorithm) == algorithms.end()) {
        sendError(callback, "Unsupported algorithm. Use A*, Dijkstra, or Floyd-Warshall.");
        return;
    }

    // The graph is weighted once, then all three algorithms use exactly those same edge weights.
    grid.randomizeWeights();
    std::vector<std::pair<std::string, RouteSummary>> comparisons;
    for (const std::string &comparisonAlgorithm : algorithms)
        comparisons.push_back({comparisonAlgorithm, calculateRoute(grid, comparisonAlgorithm, start, targets)});

    const auto selected = std::find_if(comparisons.begin(), comparisons.end(),
                                       [&](const auto &entry) { return entry.first == algorithm; });
    const RouteSummary &route = selected->second;

    Json::Value result;
    result["success"] = route.success;
    result["algorithm"] = algorithm;
    result["steps"] = route.success ? static_cast<int>(route.path.size()) - 1 : 0;
    result["cost"] = route.success ? route.cost : Json::Value(Json::nullValue);
    result["path"] = pathAsJson(route.path);
    result["targets"] = pathAsJson(targets);
    result["targetPaths"] = Json::arrayValue;
    for (const std::vector<Cell> &targetPath : route.targetPaths)
        result["targetPaths"].append(pathAsJson(targetPath));
    result["comparisons"] = Json::arrayValue;
    for (const auto &comparison : comparisons)
        result["comparisons"].append(comparisonAsJson(comparison.first, comparison.second));
    result["graph"] = graphAsJson(grid);
    result["message"] = route.success
        ? "Routes and comparison computed successfully."
        : "No route found to target " + coordinate(route.unreachableTarget) + ".";
    result["algorithmLog"] = createAlgorithmLog(algorithm, route.path, route.success);

    callback(drogon::HttpResponse::newHttpJsonResponse(result));
}
