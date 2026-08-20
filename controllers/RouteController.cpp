#include "RouteController.h"

// Ensure standard headers required by included algorithm headers are available
#include <cctype>
#include <cmath>
#include <iterator>
#include <string>
#include <vector>

#include "../algorithm/astar.h"
#include "../algorithm/bfs.h"
#include "../algorithm/dfs.h"
#include "../algorithm/grid_shortest_paths.h"
#include "../algorithm/grid_spanning_tree.h"
#include "../graph/warehousegrid.h"
#include <drogon/HttpResponse.h>
#include <algorithm>
#include <drogon/drogon.h>
#include <jsoncpp/json/json.h>

namespace {
std::string coordinate(const Cell &cell)
{
    return "(" + std::to_string(cell.x) + ", " + std::to_string(cell.y) + ")";
}

Json::Value createAlgorithmLog(const std::string &algorithm, const std::vector<Cell> &path,
                               bool allTargetsReached = true)
{
    Json::Value log(Json::arrayValue);
    const bool pathFound = !path.empty();
    const int pathSteps = pathFound ? static_cast<int>(path.size()) - 1 : 0;

    if (algorithm == "BFS") {
        log.append("Formula: distance[neighbour] = distance[current] + 1. BFS visits cells level by level using a queue.");
        for (std::size_t index = 0; index < path.size(); ++index) {
            log.append("Step " + std::to_string(index) + ": distance" + coordinate(path[index]) +
                       " = " + std::to_string(index) + ".");
        }
    } else if (algorithm == "DFS") {
        log.append("Rule: visit one unvisited neighbour, then continue deeper. If it is blocked, backtrack to the previous cell.");
        for (std::size_t index = 0; index < path.size(); ++index) {
            log.append("Step " + std::to_string(index) + ": depth" + coordinate(path[index]) +
                       " = " + std::to_string(index) + ".");
        }
    } else if (algorithm == "A*") {
        log.append("Formula: f(n) = g(n) + h(n), where g is the accumulated random edge cost and h is the Manhattan-distance estimate.");
        if (pathFound) {
            const Cell target = path.back();
            for (std::size_t index = 0; index < path.size(); ++index) {
                const int h = std::abs(path[index].x - target.x) + std::abs(path[index].y - target.y);
                log.append("Step " + std::to_string(index) + ": at " + coordinate(path[index]) +
                           ", h = |" + std::to_string(path[index].x) +
                           " - " + std::to_string(target.x) + "| + |" + std::to_string(path[index].y) +
                           " - " + std::to_string(target.y) + "| = " + std::to_string(h) +
                           "; g uses the labelled graph-edge costs.");
            }
        }
    } else if (algorithm == "DIJKSTRA") {
        log.append("Formula: candidateDistance = distance[current] + edgeWeight. Edge weights are the random labels shown in the graph.");
        for (std::size_t index = 0; index < path.size(); ++index) {
            if (index == 0) {
                log.append("Step 0: distance" + coordinate(path[index]) + " = 0.");
            } else {
                log.append("Step " + std::to_string(index) + ": relax edge " + coordinate(path[index - 1]) +
                           " - " + coordinate(path[index]) + " using its displayed random weight.");
            }
        }
    } else if (algorithm == "FLOYD-WARSHALL") {
        log.append("Formula: D[i][j] = min(D[i][j], D[i][k] + D[k][j]); direct-edge costs use the graph's random labels.");
        for (std::size_t index = 1; index < path.size(); ++index) {
            log.append("Step " + std::to_string(index) + ": D" + coordinate(path.front()) +
                       coordinate(path[index]) + " considers the labelled edge from " + coordinate(path[index - 1]) + ".");
        }
    } else if (algorithm == "KRUSKAL") {
        log.append("Formula: sort the randomly weighted graph edges; add (u, v) only if Find(u) != Find(v), then Union(u, v).");
        for (std::size_t index = 1; index < path.size(); ++index) {
            log.append("Step " + std::to_string(index) + ": accept edge " + coordinate(path[index - 1]) +
                       " - " + coordinate(path[index]) + "; its random graph weight determines the MST order.");
        }
    } else if (algorithm == "PRIM") {
        log.append("Formula: key[v] = min(key[v], weight(u, v)). Repeatedly add the outside vertex with the smallest random edge weight.");
        for (std::size_t index = 1; index < path.size(); ++index) {
            log.append("Step " + std::to_string(index) + ": consider key" + coordinate(path[index]) +
                       " through the labelled edge from " + coordinate(path[index - 1]) + ".");
        }
    }

    if (pathFound && allTargetsReached)
        log.append("Result: route found with " + std::to_string(pathSteps) + " total moves.");
    else if (pathFound)
        log.append("Result: the route stopped before an unreachable target.");
    else
        log.append("Result: no route exists because obstacles block every connection to the target.");
    return log;
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

bool readCell(const Json::Value &value, Cell &cell)
{
    if (!value.isArray() || value.size() != 2 || !value[0].isInt() || !value[1].isInt())
        return false;
    cell = {value[0].asInt(), value[1].asInt()};
    return true;
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
} // namespace

void RouteController::computeRoute(
    const drogon::HttpRequestPtr &req,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    // ==========================
    // Validate JSON request
    // ==========================
    if (!req || !req->getJsonObject())
    {
        Json::Value error;
        error["success"] = false;
        error["message"] = "Invalid JSON request.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    auto json = req->getJsonObject();

    // ==========================
    // Required parameters
    // ==========================
    const std::vector<std::string> requiredFields = {
        "rows", "cols", "start", "obstacles"};

    for (const auto &field : requiredFields)
    {
        if (!json->isMember(field))
        {
            Json::Value error;
            error["success"] = false;
            error["message"] = "Missing required field: " + field;
            auto resp = drogon::HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(drogon::k400BadRequest);
            callback(resp);
            return;
        }
    }

    // ==========================
    // Read grid size
    // ==========================
    int rows = (*json)["rows"].asInt();
    int cols = (*json)["cols"].asInt();

    if (rows <= 0 || cols <= 0)
    {
        Json::Value error;
        error["success"] = false;
        error["message"] = "Rows and cols must be greater than zero.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    WarehouseGrid grid(rows, cols);

    // ==========================
    // Obstacles
    // ==========================
   for (const auto &obs : (*json)["obstacles"])
{
    if (obs.size() < 2)
        continue;

    int x = obs[0].asInt();
    int y = obs[1].asInt();

    if (!grid.isInside(x, y))
    {
        Json::Value error;
        error["success"] = false;
        error["message"] = "Obstacle is outside the grid.";

        auto resp = drogon::HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    grid.addObstacle(x, y);
}

    // A new set of random positive edge weights is used for every route request.
    // BFS/DFS still optimize traversal order; Dijkstra, A*, Floyd-Warshall and
    // the MST algorithms use these displayed costs.
    grid.randomizeWeights();

    // ==========================
    // Start & Targets
    // ==========================
    Cell start;
    if (!readCell((*json)["start"], start))
    {
        Json::Value error;
        error["success"] = false;
        error["message"] = "Start must be a coordinate pair [row, col].";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    std::vector<Cell> targets;
    if (json->isMember("targets"))
    {
        const Json::Value &jsonTargets = (*json)["targets"];
        if (!jsonTargets.isArray() || jsonTargets.empty() || jsonTargets.size() > 8)
        {
            Json::Value error;
            error["success"] = false;
            error["message"] = "Targets must contain between 1 and 8 coordinate pairs.";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(drogon::k400BadRequest);
            callback(resp);
            return;
        }
        for (const Json::Value &jsonTarget : jsonTargets)
        {
            Cell target;
            if (!readCell(jsonTarget, target))
            {
                Json::Value error;
                error["success"] = false;
                error["message"] = "Every target must be a coordinate pair [row, col].";
                auto resp = drogon::HttpResponse::newHttpJsonResponse(error);
                resp->setStatusCode(drogon::k400BadRequest);
                callback(resp);
                return;
            }
            targets.push_back(target);
        }
    }
    else if (json->isMember("end"))
    {
        Cell target;
        if (!readCell((*json)["end"], target))
        {
            Json::Value error;
            error["success"] = false;
            error["message"] = "End must be a coordinate pair [row, col].";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(drogon::k400BadRequest);
            callback(resp);
            return;
        }
        targets.push_back(target);
    }
    else
    {
        Json::Value error;
        error["success"] = false;
        error["message"] = "Missing required field: targets.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    // Boundary checks
    if (!grid.isInside(start.x, start.y))
    {
        Json::Value error;
        error["success"] = false;
        error["message"] = "Start position is outside the grid.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }
    // Occupancy checks
    if (!grid.isFree(start.x, start.y))
    {
        Json::Value error;
        error["success"] = false;
        error["message"] = "Invalid start position (occupied).";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }
    for (const Cell &target : targets)
    {
        if (!grid.isInside(target.x, target.y))
        {
            Json::Value error;
            error["success"] = false;
            error["message"] = "Target position is outside the grid.";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(drogon::k400BadRequest);
            callback(resp);
            return;
        }
        if (!grid.isFree(target.x, target.y))
        {
            Json::Value error;
            error["success"] = false;
            error["message"] = "Invalid target position (occupied).";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(error);
            resp->setStatusCode(drogon::k400BadRequest);
            callback(resp);
            return;
        }
    }
  
    // ==========================
    // Algorithm Selection
    // ==========================
    std::string algorithm = "BFS";

    if (json->isMember("algorithm"))
    {
        algorithm = (*json)["algorithm"].asString();
    }
    std::transform(algorithm.begin(),
     algorithm.end(), 
     algorithm.begin(), 
     ::toupper);
    if (algorithm == "FLOYD_WARSHALL")
        algorithm = "FLOYD-WARSHALL";

    const bool supportedAlgorithm = algorithm == "BFS" || algorithm == "A*" ||
                                  algorithm == "DFS" || algorithm == "DIJKSTRA" ||
                                  algorithm == "FLOYD-WARSHALL" || algorithm == "KRUSKAL" ||
                                  algorithm == "PRIM";
    if (!supportedAlgorithm)
    {
        Json::Value error;
        error["success"] = false;
        error["message"] = "Unsupported algorithm.";
        error["supported"] = Json::arrayValue;
        error["supported"].append("BFS");
        error["supported"].append("A*");
        error["supported"].append("DFS");
        error["supported"].append("DIJKSTRA");
        error["supported"].append("FLOYD-WARSHALL");
        error["supported"].append("KRUSKAL");
        error["supported"].append("PRIM");
        auto resp = drogon::HttpResponse::newHttpJsonResponse(error);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    auto findLeg = [&](Cell from, Cell target) {
        if (from.x == target.x && from.y == target.y)
            return std::vector<Cell>{from};
        if (algorithm == "BFS")
            return bfsPath(grid, from, target);
        if (algorithm == "A*")
            return astarPath(grid, from, target);
        if (algorithm == "DFS")
            return dfsPath(grid, from, target);
        if (algorithm == "DIJKSTRA")
            return dijkstraPath(grid, from, target);
        if (algorithm == "FLOYD-WARSHALL")
            return floydWarshallPath(grid, from, target);
        if (algorithm == "KRUSKAL")
            return kruskalPath(grid, from, target);
        return primPath(grid, from, target);
    };

    std::vector<Cell> path;
    std::vector<std::vector<Cell>> targetPaths;
    Cell current = start;
    Cell unreachableTarget{-1, -1};
    for (const Cell &target : targets)
    {
        std::vector<Cell> leg = findLeg(current, target);
        if (leg.empty())
        {
            unreachableTarget = target;
            break;
        }
        targetPaths.push_back(leg);
        if (path.empty())
            path = leg;
        else
            path.insert(path.end(), std::next(leg.begin()), leg.end());
        current = target;
    }

    const bool allTargetsReached = unreachableTarget.x == -1;

    // ==========================
    // Response
    // ==========================
    Json::Value result;
    result["success"] = allTargetsReached;
    result["algorithm"] = algorithm;
    result["steps"] = path.empty() ? 0 : static_cast<int>(path.size()) - 1;
    result["path"] = pathAsJson(path);
    result["targets"] = pathAsJson(targets);
    result["targetPaths"] = Json::arrayValue;
    for (const std::vector<Cell> &targetPath : targetPaths)
        result["targetPaths"].append(pathAsJson(targetPath));
    result["graph"] = graphAsJson(grid);
    result["message"] = allTargetsReached
        ? "Routes computed successfully."
        : "No route found to target " + coordinate(unreachableTarget) + ".";
    result["algorithmLog"] = createAlgorithmLog(algorithm, path, allTargetsReached);

    auto resp = drogon::HttpResponse::newHttpJsonResponse(result);
    callback(resp);
}
