#pragma once
#include <drogon/HttpController.h>
#include <functional>

class RouteController : public drogon::HttpController<RouteController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(RouteController::computeRoute, "/route", drogon::Post);
    METHOD_LIST_END

    void computeRoute(const drogon::HttpRequestPtr &req,
                      std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};
