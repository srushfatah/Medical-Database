#pragma once
#include <drogon/HttpController.h>

class Admit : public drogon::HttpController<Admit>
{
public:
    METHOD_LIST_BEGIN
        ADD_METHOD_TO(Admit::admit,     "/api/patients/admit",     drogon::Post);
        ADD_METHOD_TO(Admit::discharge, "/api/patients/discharge", drogon::Post);
        ADD_METHOD_TO(Admit::search,    "/api/patients/search",    drogon::Get);
    METHOD_LIST_END

    void admit(const drogon::HttpRequestPtr& req,
               std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void discharge(const drogon::HttpRequestPtr& req,
                   std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void search(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);
};
