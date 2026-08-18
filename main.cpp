#include <drogon/drogon.h>
#include <drogon/HttpResponse.h>
#include <drogon/HttpTypes.h>

#include <filesystem>
#include <functional>
#include <iostream>

#include <jsoncpp/json/json.h>

#include "controllers/RouteController.h"
#include "controllers/WarehouseController.h"

int main(){
    std::filesystem::path projectRoot = std::filesystem::path(__FILE__).parent_path();
    std::filesystem::path configPath = projectRoot / "config.json";

    if (!std::filesystem::exists(configPath)) {
        std::cerr << "Config file not found: " << configPath << std::endl;
        return 1;
    }

    std::filesystem::current_path(projectRoot);
    std::filesystem::create_directories(projectRoot / "database");

    drogon::app().loadConfigFile(configPath.string());
    // Serve the HTML, CSS, and JavaScript from the same server as the API.
    // This avoids a second frontend server and eliminates CORS/port mismatches.
    drogon::app().setDocumentRoot((projectRoot / "frontend").string())
                 .setHomePage("index.html");

   // Enable CORS
drogon::app().registerPostHandlingAdvice(
    [](const drogon::HttpRequestPtr &req,
       const drogon::HttpResponsePtr &resp)
    {
        if (resp)
        {
            resp->addHeader("Access-Control-Allow-Origin", "*");
            resp->addHeader("Access-Control-Allow-Headers", "Content-Type");
            resp->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        }
    });


    drogon::app().registerHandler("/warehouse",
        [](const drogon::HttpRequestPtr &req,
           std::function<void (const drogon::HttpResponsePtr &)> &&callback)
        {
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
        });

    drogon::app().registerHandler(
    "/route",
    [](const drogon::HttpRequestPtr &req,
       std::function<void(const drogon::HttpResponsePtr &)> &&callback)
    {
        if (req->method() == drogon::Options)
        {
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->addHeader("Access-Control-Allow-Origin", "*");
            resp->addHeader("Access-Control-Allow-Headers", "Content-Type");
            resp->addHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
            callback(resp);
            return;
        }

        // Forward POST requests to RouteController
        RouteController controller;
        controller.computeRoute(req, std::move(callback));
    });

    
    drogon::app().run();
}
