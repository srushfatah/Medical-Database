#include "TestsController.h"
#include "TestsLogic.h"
#include <drogon/drogon.h>

// This function takes the test data and converts it into a JSON array to send back in the response.
static Json::Value testsToJson(const std::vector<TestResultInfo>& tests)
{
    Json::Value arr(Json::arrayValue);
    for (auto& t : tests)
    {
        Json::Value obj;
        if (t.testResultID > 0)
            obj["TestResultID"] = (Json::Int64)t.testResultID;
        else
            obj["TestResultID"] = Json::nullValue;
        obj["PatientID"]    = t.patientID;
        obj["TestType"]     = t.testType;
        obj["Impression"]   = t.impression;
        obj["Findings"]     = t.findings;
        obj["DateTime"]     = t.dateTime;
        obj["TestStatus"]   = t.testStatus;
        arr.append(obj);
    }
    return arr;
}

// This function returns test results based on the user's role. Patients only see their own tests,
// doctors and nurses get an empty list until they search, and admins see everything.
void TestsController::getAll(const drogon::HttpRequestPtr& req,
                             std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto session = req->session();
    auto role = session->getOptional<std::string>("role");
    std::string r = role.value_or("");

    // If the user is a patient, only show their own tests.
    if (r == "patient")
    {
        auto pid = session->getOptional<int>("patientID");
        if (pid && *pid > 0)
        {
            auto tests = TestsLogic::searchTests(std::to_string(*pid));
            auto resp = drogon::HttpResponse::newHttpJsonResponse(testsToJson(tests));
            callback(resp);
            return;
        }
        Json::Value arr(Json::arrayValue);
        auto resp = drogon::HttpResponse::newHttpJsonResponse(arr);
        callback(resp);
        return;
    }

    // Doctors and nurses have to search instead of seeing all records at once.
    if (r == "doctor" || r == "nurse")
    {
        Json::Value arr(Json::arrayValue);
        auto resp = drogon::HttpResponse::newHttpJsonResponse(arr);
        callback(resp);
        return;
    }

    // Admins see all test records.
    auto tests = TestsLogic::getAllTests();
    auto resp = drogon::HttpResponse::newHttpJsonResponse(testsToJson(tests));
    callback(resp);
}

// This function searches for test results. If the user is a patient, it always returns
// only their own data no matter what they search. Everyone else gets normal search results.
void TestsController::search(const drogon::HttpRequestPtr& req,
                             std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto session = req->session();
    auto role = session->getOptional<std::string>("role");
    std::string r = role.value_or("");

    // Patients always only see their own data.
    if (r == "patient")
    {
        auto pid = session->getOptional<int>("patientID");
        if (pid && *pid > 0)
        {
            auto tests = TestsLogic::searchTests(std::to_string(*pid));
            auto resp = drogon::HttpResponse::newHttpJsonResponse(testsToJson(tests));
            callback(resp);
            return;
        }
        Json::Value arr(Json::arrayValue);
        auto resp = drogon::HttpResponse::newHttpJsonResponse(arr);
        callback(resp);
        return;
    }

    // Everyone else gets normal search results.
    std::string q = req->getParameter("q");
    auto tests = TestsLogic::searchTests(q);
    auto resp = drogon::HttpResponse::newHttpJsonResponse(testsToJson(tests));
    callback(resp);
}

// This function orders a new test. It requires a PatientID and TestType to be provided.
void TestsController::create(const drogon::HttpRequestPtr& req,
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

    int         patientID        = (*json)["PatientID"].asInt();
    std::string testType         = (*json)["TestType"].asString();
    std::string orderingProvider = (*json)["OrderingProvider"].asString();
    std::string testStatus       = (*json)["TestStatus"].asString();

    if (patientID <= 0 || testType.empty())
    {
        Json::Value err;
        err["error"] = "Patient ID and test type are required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    auto result = TestsLogic::orderTest(patientID, testType, orderingProvider, testStatus);

    Json::Value response;
    if (result.success)
    {
        response["success"] = true;
        auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
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
