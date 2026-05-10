#pragma once
#include <drogon/HttpController.h>

class Orders : public drogon::HttpController<Orders>
{
public:
    METHOD_LIST_BEGIN
        ADD_METHOD_TO(Orders::getAll,  "/api/orders",        drogon::Get);
        ADD_METHOD_TO(Orders::search,  "/api/orders/search", drogon::Get);
        ADD_METHOD_TO(Orders::create,  "/api/orders",        drogon::Post);
    METHOD_LIST_END

    void getAll(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void search(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void create(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);
};
