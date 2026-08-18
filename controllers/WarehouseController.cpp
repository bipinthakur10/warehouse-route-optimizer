#include "WarehouseController.h"
#include <drogon/drogon.h>
#include <drogon/HttpResponse.h>
#include <jsoncpp/json/json.h>

void WarehouseController::getWarehouseInfo(const drogon::HttpRequestPtr &req,
                                           std::function<void(const drogon::HttpResponsePtr &)> &&callback) {
    Json::Value info;
    info["name"] = "Warehouse Robot Route Optimizer";
    info["supportedAlgorithms"] = Json::Value(Json::arrayValue);
    info["supportedAlgorithms"].append("BFS");
    info["supportedAlgorithms"].append("DFS");
    info["supportedAlgorithms"].append("A*");
    info["supportedAlgorithms"].append("Dijkstra");
    info["supportedAlgorithms"].append("Floyd-Warshall");
    info["supportedAlgorithms"].append("Kruskal");
    info["supportedAlgorithms"].append("Prim");
    info["description"] = "Finds efficient robot paths around warehouse obstacles";

    auto resp = drogon::HttpResponse::newHttpJsonResponse(info);
    callback(resp);
}
