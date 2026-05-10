#pragma once
#include <drogon/HttpController.h>

class LabsController : public drogon::HttpController<LabsController>
{
public:
    METHOD_LIST_BEGIN
        ADD_METHOD_TO(LabsController::getAll,  "/api/labs",        drogon::Get);
        ADD_METHOD_TO(LabsController::search,  "/api/labs/search", drogon::Get);
        ADD_METHOD_TO(LabsController::create,  "/api/labs",        drogon::Post);
    METHOD_LIST_END

    void getAll(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    void search(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    void create(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);
};
