#include "BillingController.h"
#include "BillingLogic.h"
#include <drogon/drogon.h>

// This function takes the billing data and converts it into a JSON array to send back in the response.
static Json::Value billingToJson(const std::vector<BillingInfo>& records)
{
    Json::Value arr(Json::arrayValue);
    for (auto& b : records)
    {
        Json::Value obj;
        obj["BillingID"]     = (Json::Int64)b.billingID;
        obj["PatientID"]     = b.patientID;
        obj["RoomCharge"]    = b.roomCharge;
        obj["VisitCharge"]   = b.visitCharge;
        obj["RxCharge"]      = b.rxCharge;
        obj["LabCharge"]     = b.labCharge;
        obj["ImagingCharge"] = b.imagingCharge;
        obj["TotalCharge"]   = b.totalCharge;
        obj["InsurancePaid"] = b.insurancePaid;
        obj["PatientPaid"]   = b.patientPaid;
        obj["BalanceDue"]    = b.balanceDue;
        arr.append(obj);
    }
    return arr;
}

// This function returns billing records based on the user's role. Patients only see their
// own billing, while admins and front desk users can see everything.
void BillingController::getAll(const drogon::HttpRequestPtr& req,
                               std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto session = req->session();
    auto role = session->getOptional<std::string>("role");
    std::string r = role.value_or("");

    // If the user is a patient, only show their own billing.
    if (r == "patient")
    {
        auto pid = session->getOptional<int>("patientID");
        if (pid && *pid > 0)
        {
            auto records = BillingLogic::searchBilling(std::to_string(*pid));
            auto resp = drogon::HttpResponse::newHttpJsonResponse(billingToJson(records));
            callback(resp);
            return;
        }
        Json::Value arr(Json::arrayValue);
        auto resp = drogon::HttpResponse::newHttpJsonResponse(arr);
        callback(resp);
        return;
    }

    // Admins and front desk see all billing records.
    auto records = BillingLogic::getAllBilling();
    auto resp = drogon::HttpResponse::newHttpJsonResponse(billingToJson(records));
    callback(resp);
}

// This function searches billing records. If the user is a patient, it always returns
// only their own data. Admins and front desk users get normal search results.
void BillingController::search(const drogon::HttpRequestPtr& req,
                               std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto session = req->session();
    auto role = session->getOptional<std::string>("role");
    std::string r = role.value_or("");

    // Patients always only see their own billing.
    if (r == "patient")
    {
        auto pid = session->getOptional<int>("patientID");
        if (pid && *pid > 0)
        {
            auto records = BillingLogic::searchBilling(std::to_string(*pid));
            auto resp = drogon::HttpResponse::newHttpJsonResponse(billingToJson(records));
            callback(resp);
            return;
        }
        Json::Value arr(Json::arrayValue);
        auto resp = drogon::HttpResponse::newHttpJsonResponse(arr);
        callback(resp);
        return;
    }

    // Admins and front desk get normal search results.
    std::string q = req->getParameter("q");
    auto records = BillingLogic::searchBilling(q);
    auto resp = drogon::HttpResponse::newHttpJsonResponse(billingToJson(records));
    callback(resp);
}

// This function saves or updates a billing record. It pulls all the charge amounts from
// the request, makes sure the PatientID is valid, and sends it to BillingLogic which
// calculates the total charge and balance due.
void BillingController::save(const drogon::HttpRequestPtr& req,
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

    int    patientID     = (*json)["PatientID"].asInt();
    double roomCharge    = (*json)["RoomCharge"].asDouble();
    double visitCharge   = (*json)["VisitCharge"].asDouble();
    double rxCharge      = (*json)["RxCharge"].asDouble();
    double labCharge     = (*json)["LabCharge"].asDouble();
    double imagingCharge = (*json)["ImagingCharge"].asDouble();
    double insurancePaid = (*json)["InsurancePaid"].asDouble();
    double patientPaid   = (*json)["PatientPaid"].asDouble();

    if (patientID <= 0)
    {
        Json::Value err;
        err["error"] = "Patient ID is required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    auto result = BillingLogic::saveBilling(patientID, roomCharge, visitCharge,
                                           rxCharge, labCharge, imagingCharge,
                                           insurancePaid, patientPaid);

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
