#include "Admit.h"
#include "AdmitLogic.h"
#include <drogon/drogon.h>

// This function admits a new patient. It pulls all the patient info from the request,
// makes sure the required fields are filled in, and then sends it off to AdmitLogic
// to insert into the database. It returns the new PatientID if it works.
void Admit::admit(const drogon::HttpRequestPtr& req,
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

    std::string patientName        = (*json)["PatientName"].asString();
    std::string gender             = (*json)["Gender"].asString();
    int         age                = (*json)["Age"].asInt();
    std::string admitDate          = (*json)["AdmitDate"].asString();
    std::string admitReason        = (*json)["AdmitReason"].asString();
    std::string roomNumber         = (*json)["RoomNumber"].asString();
    std::string attendingPhysician = (*json)["AttendingPhysician"].asString();
    std::string dischargeDate      = (*json)["DischargeDate"].isNull() ? "" : (*json)["DischargeDate"].asString();

    if (patientName.empty() || gender.empty() || admitDate.empty() ||
        admitReason.empty() || roomNumber.empty() || attendingPhysician.empty())
    {
        Json::Value err;
        err["error"] = "All required fields must be provided.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    auto result = AdmitLogic::admitPatient(patientName, gender, age, admitDate,
                                           admitReason, roomNumber, attendingPhysician, dischargeDate);

    Json::Value response;
    if (result.success)
    {
        response["success"] = true;
        response["PatientID"] = result.patientID;
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

// This function discharges a patient by setting their discharge date.
void Admit::discharge(const drogon::HttpRequestPtr& req,
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

    int         patientID     = (*json)["PatientID"].asInt();
    std::string dischargeDate = (*json)["DischargeDate"].asString();

    if (patientID <= 0 || dischargeDate.empty())
    {
        Json::Value err;
        err["error"] = "Patient ID and discharge date are required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    auto result = AdmitLogic::dischargePatient(patientID, dischargeDate);

    Json::Value response;
    if (result.success)
    {
        response["success"] = true;
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

// This function searches for patients by name or room number and returns the matches as JSON.
void Admit::search(const drogon::HttpRequestPtr& req,
                   std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    std::string q = req->getParameter("q");

    auto patients = AdmitLogic::searchPatients(q);

    Json::Value response;
    Json::Value patientsArray(Json::arrayValue);
    for (auto& p : patients)
    {
        Json::Value obj;
        obj["PatientID"]          = p.patientID;
        obj["PatientName"]        = p.patientName;
        obj["Gender"]             = p.gender;
        obj["Age"]                = p.age;
        obj["AdmitDate"]          = p.admitDate;
        obj["DischargeDate"]      = p.dischargeDate;
        obj["AdmitReason"]        = p.admitReason;
        obj["RoomNumber"]         = p.roomNumber;
        obj["AttendingPhysician"] = p.attendingPhysician;
        patientsArray.append(obj);
    }
    response["patients"] = patientsArray;

    auto resp = drogon::HttpResponse::newHttpJsonResponse(response);
    resp->setStatusCode(drogon::k200OK);
    callback(resp);
}
