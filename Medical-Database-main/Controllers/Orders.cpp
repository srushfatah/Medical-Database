#include "Orders.h"
#include "OrdersLogic.h"
#include <drogon/drogon.h>

// This function takes the order data and converts it into a JSON array to send back in the response.
static Json::Value ordersToJson(const std::vector<OrderInfo>& orders)
{
    Json::Value arr(Json::arrayValue);
    for (auto& o : orders)
    {
        Json::Value obj;
        obj["OrderID"]          = (Json::Int64)o.orderID;
        obj["PatientID"]        = o.patientID;
        obj["OrderType"]        = o.orderType;
        obj["OrderName"]        = o.orderName;
        obj["Dose"]             = o.dose;
        obj["Frequency"]        = o.frequency;
        obj["OrderingProvider"] = o.orderingProvider;
        obj["OrderStatus"]      = o.orderStatus;
        obj["OrderDateTime"]    = o.orderDateTime;
        arr.append(obj);
    }
    return arr;
}

// This function returns all orders. Doctors and nurses get an empty list since they need to
// search by patient, while admins and front desk users can see everything.
void Orders::getAll(const drogon::HttpRequestPtr& req,
                    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    auto session = req->session();
    auto role = session->getOptional<std::string>("role");
    std::string r = role.value_or("");

    // Doctors and nurses have to search instead of seeing all records at once.
    if (r == "doctor" || r == "nurse")
    {
        Json::Value arr(Json::arrayValue);
        auto resp = drogon::HttpResponse::newHttpJsonResponse(arr);
        callback(resp);
        return;
    }

    // Admins and front desk see everything.
    auto orders = OrdersLogic::getAllOrders();
    auto resp = drogon::HttpResponse::newHttpJsonResponse(ordersToJson(orders));
    callback(resp);
}

// This function searches for orders based on whatever the user types in.
void Orders::search(const drogon::HttpRequestPtr& req,
                    std::function<void(const drogon::HttpResponsePtr&)>&& callback)
{
    std::string q = req->getParameter("q");
    auto orders = OrdersLogic::searchOrders(q);
    auto resp = drogon::HttpResponse::newHttpJsonResponse(ordersToJson(orders));
    callback(resp);
}

// This function creates a new order. It makes sure PatientID, OrderType, and OrderName
// are all provided, then sends it to OrdersLogic to handle the insert.
void Orders::create(const drogon::HttpRequestPtr& req,
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
    std::string orderType        = (*json)["OrderType"].asString();
    std::string orderName        = (*json)["OrderName"].asString();
    std::string dose             = (*json)["Dose"].asString();
    std::string frequency        = (*json)["Frequency"].asString();
    std::string orderingProvider = (*json)["OrderingProvider"].asString();
    std::string orderStatus      = (*json)["OrderStatus"].asString();

    if (patientID <= 0 || orderType.empty() || orderName.empty())
    {
        Json::Value err;
        err["error"] = "Patient ID, order type, and order name are required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    auto result = OrdersLogic::createOrder(patientID, orderType, orderName,
                                           dose, frequency, orderingProvider, orderStatus);

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
