#pragma once
#include <drogon/HttpController.h>

class Login : public drogon::HttpController<Login>
{
public:
    METHOD_LIST_BEGIN
        ADD_METHOD_TO(Login::handleLogin,    "/api/login",    drogon::Post);
        ADD_METHOD_TO(Login::handleRegister, "/api/register", drogon::Post);
        ADD_METHOD_TO(Login::handleLogout,   "/api/logout",     drogon::Post);
        ADD_METHOD_TO(Login::handleSession,  "/api/session",   drogon::Get);
        ADD_METHOD_TO(Login::handleDevLogin, "/api/dev-login", drogon::Post);
    METHOD_LIST_END

    void handleLogin(const drogon::HttpRequestPtr& req,
                     std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void handleRegister(const drogon::HttpRequestPtr& req,
                        std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void handleLogout(const drogon::HttpRequestPtr& req,
                      std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void handleSession(const drogon::HttpRequestPtr& req,
                       std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void handleDevLogin(const drogon::HttpRequestPtr& req,
                        std::function<void(const drogon::HttpResponsePtr&)>&& callback);
};