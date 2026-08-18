#pragma once
#include <drogon/HttpController.h>
#include <functional>

class WarehouseController : public drogon::HttpController<WarehouseController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(WarehouseController::getWarehouseInfo, "/warehouse", drogon::Get);
    METHOD_LIST_END

    void getWarehouseInfo(const drogon::HttpRequestPtr &req,
                          std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};
