#include "LabsController.h"
#include "LabsLogic.h"
#include <drogon/drogon.h>

// This function takes the lab data and converts it into a JSON array to send back in the response.
static Json::Value labsToJson(const std::vector<LabResultInfo>& labs)
{
    Json::Value arr(Json::arrayValue);
    for (auto& l : labs)
    {
        Json::Value obj;
        if (l.labResultID > 0)
            obj["LabResultID"] = (Json::Int64)l.labResultID;
        else
            obj["LabResultID"] = Json::nullValue;
        obj["PatientID"]      = l.patientID;
        obj["LabType"]        = l.labType;
        obj["ResultName"]     = l.resultName;
        obj["ResultValue"]    = l.resultValue;
        obj["Unit"]           = l.unit;
        obj["ReferenceRange"] = l.referenceRange;
        obj["Flag"]           = l.flag;
        obj["DateTime"]       = l.dateTime;
        obj["LabStatus"]      = l.labStatus;
        arr.append(obj);
    }
    return arr;
}

// This function returns lab results based on the user's role. Patients only see their own labs,
// doctors and nurses get an empty list until they search, and admins see everything.
void LabsController::getAll(const drogon::HttpRequestPtr& req,
                            std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto session = req->session();
    auto role = session->getOptional<std::string>("role");
    std::string r = role.value_or("");

    // If the user is a patient, only show their own labs.
    if (r == "patient")
    {
        auto pid = session->getOptional<int>("patientID");
        if (pid && *pid > 0)
        {
            auto labs = LabsLogic::searchLabs(std::to_string(*pid));
            auto resp = drogon::HttpResponse::newHttpJsonResponse(labsToJson(labs));
            callback(resp);
            return;
        }
        // If the patient doesn't have a PatientID linked, return an empty list.
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

    // Admins see all lab records.
    auto labs = LabsLogic::getAllLabs();
    auto resp = drogon::HttpResponse::newHttpJsonResponse(labsToJson(labs));
    callback(resp);
}

// This function searches for lab results. If the user is a patient, it always returns
// only their own data no matter what they search. Everyone else gets normal search results.
void LabsController::search(const drogon::HttpRequestPtr& req,
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
            auto labs = LabsLogic::searchLabs(std::to_string(*pid));
            auto resp = drogon::HttpResponse::newHttpJsonResponse(labsToJson(labs));
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
    auto labs = LabsLogic::searchLabs(q);
    auto resp = drogon::HttpResponse::newHttpJsonResponse(labsToJson(labs));
    callback(resp);
}

// This function orders a new lab. It requires a PatientID and LabType to be provided.
void LabsController::create(const drogon::HttpRequestPtr& req,
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
    std::string labType          = (*json)["LabType"].asString();
    std::string orderingProvider = (*json)["OrderingProvider"].asString();
    std::string labStatus        = (*json)["LabStatus"].asString();

    if (patientID <= 0 || labType.empty())
    {
        Json::Value err;
        err["error"] = "Patient ID and lab type are required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    auto result = LabsLogic::orderLab(patientID, labType, orderingProvider, labStatus);

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
