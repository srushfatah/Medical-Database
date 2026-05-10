#include "Login.h"
#include "LoginLogic.h"
#include <drogon/drogon.h>

// This function handles logging in. It takes the username and password from the request,
// checks them against the database, and if they match, it stores the user's info in the session.
// The session gets cleared first so old login data doesn't stick around.
void Login::handleLogin(const drogon::HttpRequestPtr& req,
                        std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto json = req->getJsonObject();
    if (!json)
    {
        Json::Value err;
        err["error"] = "Invalid request body.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    std::string username = (*json)["username"].asString();
    std::string password = (*json)["password"].asString();

    if (username.empty() || password.empty())
    {
        Json::Value err;
        err["error"] = "Username and password are required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    LoginResult result = LoginLogic::login(username, password);

    Json::Value response;
    if (result.success)
    {
        // Clearing the session before inserting new data because Drogon won't overwrite existing keys.
        auto session = req->session();
        session->clear();
        session->insert("role", result.role);
        session->insert("username", result.username);
        session->insert("patientID", result.patientID);

        response["success"]  = true;
        response["role"]     = result.role;
        response["username"] = result.username;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
        resp->setStatusCode(drogon::k200OK);
        callback(resp);
    }
    else
    {
        response["error"] = result.error;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
        resp->setStatusCode(drogon::k401Unauthorized);
        callback(resp);
    }
}

// This function handles registering a new user. It makes sure all the fields are filled in,
// then sends everything to LoginLogic which hashes the password and creates the account.
void Login::handleRegister(const drogon::HttpRequestPtr& req,
                           std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto json = req->getJsonObject();
    if (!json)
    {
        Json::Value err;
        err["error"] = "Invalid request body.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    std::string firstName = (*json)["firstName"].asString();
    std::string lastName  = (*json)["lastName"].asString();
    std::string username  = (*json)["username"].asString();
    std::string password  = (*json)["password"].asString();
    std::string role      = (*json)["role"].asString();

    if (firstName.empty() || lastName.empty() ||
        username.empty()  || password.empty() || role.empty())
    {
        Json::Value err;
        err["error"] = "All fields are required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    int patientID = (*json)["patientID"].asInt();

    LoginResult result = LoginLogic::registerUser(firstName, lastName,
                                                   username, password, role, patientID);

    Json::Value response;
    if (result.success)
    {
        response["success"]  = true;
        response["username"] = result.username;
        response["role"]     = result.role;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
        resp->setStatusCode(drogon::k200OK);
        callback(resp);
    }
    else
    {
        response["error"] = result.error;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
    }
}

// This function handles logging out by clearing the session data.
void Login::handleLogout(const drogon::HttpRequestPtr& req,
                         std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto session = req->session();
    session->clear();

    Json::Value response;
    response["success"] = true;
    auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
    callback(resp);
}

// This function returns whether the user is currently logged in or not.
// The frontend calls this on page load to check the session and get the user's role.
void Login::handleSession(const drogon::HttpRequestPtr& req,
                          std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto session = req->session();
    auto role = session->getOptional<std::string>("role");
    auto username = session->getOptional<std::string>("username");

    Json::Value response;
    if (role && username)
    {
        response["loggedIn"] = true;
        response["role"]     = *role;
        response["username"] = *username;
    }
    else
    {
        response["loggedIn"] = false;
    }
    auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
    callback(resp);
}

// This function is for the dev bypass buttons on the login page. It creates a real
// session so you can quickly log in as any role during testing. If the role is patient,
// it looks up a real PatientID from the database so the patient's data shows up correctly.
void Login::handleDevLogin(const drogon::HttpRequestPtr& req,
                           std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto json = req->getJsonObject();
    if (!json)
    {
        Json::Value err;
        err["error"] = "Invalid request body.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    std::string role = (*json)["role"].asString();
    std::string username = (*json)["username"].asString();

    // If the role is patient, look up a real PatientID from the users table.
    int patientID = 0;
    if (role == "patient")
    {
        try
        {
            auto db = drogon::app().getDbClient();
            auto result = db->execSqlSync(
                "SELECT PatientID FROM users WHERE Role = 'patient' AND PatientID IS NOT NULL LIMIT 1"
            );
            if (result.size() > 0)
                patientID = result[0]["PatientID"].as<int>();
        }
        catch (...) {}
    }

    auto session = req->session();
    session->clear();
    session->insert("role", role);
    session->insert("username", username);
    session->insert("patientID", patientID);

    Json::Value response;
    response["success"]  = true;
    response["role"]     = role;
    response["username"] = username;
    auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
    callback(resp);
}