#pragma once
#include <drogon/HttpController.h>

class BillingController : public drogon::HttpController<BillingController>
{
public:
    METHOD_LIST_BEGIN
        ADD_METHOD_TO(BillingController::getAll,  "/api/billing",        drogon::Get);
        ADD_METHOD_TO(BillingController::search,  "/api/billing/search", drogon::Get);
        ADD_METHOD_TO(BillingController::save,    "/api/billing",        drogon::Post);
    METHOD_LIST_END

    void getAll(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    void search(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    void save(const drogon::HttpRequestPtr& req,
              std::function<void(const drogon::HttpResponsePtr&)>&& callback);
};
