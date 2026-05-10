#pragma once
#include <drogon/HttpController.h>

class TestsController : public drogon::HttpController<TestsController>
{
public:
    METHOD_LIST_BEGIN
        ADD_METHOD_TO(TestsController::getAll,  "/api/tests",        drogon::Get);
        ADD_METHOD_TO(TestsController::search,  "/api/tests/search", drogon::Get);
        ADD_METHOD_TO(TestsController::create,  "/api/tests",        drogon::Post);
    METHOD_LIST_END

    void getAll(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    void search(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    void create(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);
};
