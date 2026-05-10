#include "OrdersDB.h"
#include <drogon/drogon.h>
#include <iostream>

// This function takes the rows from the database and puts them into OrderInfo structs.
// Dose and Frequency can be null, so those get checked first.
static std::vector<OrderInfo> rowsToOrders(const drogon::orm::Result& result)
{
    std::vector<OrderInfo> orders;
    for (auto& row : result)
    {
        OrderInfo o;
        o.orderID          = row["OrderID"].as<long long>();
        o.patientID        = row["PatientID"].as<int>();
        o.orderType        = row["OrderType"].as<std::string>();
        o.orderName        = row["OrderName"].as<std::string>();
        o.dose             = row["Dose"].isNull() ? "" : row["Dose"].as<std::string>();
        o.frequency        = row["Frequency"].isNull() ? "" : row["Frequency"].as<std::string>();
        o.orderingProvider = row["OrderingProvider"].as<std::string>();
        o.orderStatus      = row["OrderStatus"].as<std::string>();
        o.orderDateTime    = row["OrderDateTime"].as<std::string>();
        orders.push_back(o);
    }
    return orders;
}

// Getting all orders from the database with the newest ones showing first.
std::vector<OrderInfo> OrdersDB::getAllOrders()
{
    try
    {
        auto db = drogon::app().getDbClient();
        auto result = db->execSqlSync(
            "SELECT OrderID, PatientID, OrderType, OrderName, Dose, Frequency, "
            "OrderingProvider, OrderStatus, OrderDateTime "
            "FROM patient_Orders ORDER BY OrderID DESC LIMIT 200"
        );
        return rowsToOrders(result);
    }
    catch (const drogon::orm::DrogonDbException& e)
    {
        std::cerr << "OrdersDB::getAllOrders error: " << e.base().what() << std::endl;
        return {};
    }
}

// This function searches for orders. If the query is a number, it searches by PatientID.
// If it's text, it searches by the order type and order name.
std::vector<OrderInfo> OrdersDB::searchOrders(const std::string& query)
{
    try
    {
        auto db = drogon::app().getDbClient();

        // Checking if the query is a number or text to decide how to search.
        bool isNumber = !query.empty() && query.find_first_not_of("0123456789") == std::string::npos;

        if (isNumber)
        {
            auto result = db->execSqlSync(
                "SELECT OrderID, PatientID, OrderType, OrderName, Dose, Frequency, "
                "OrderingProvider, OrderStatus, OrderDateTime "
                "FROM patient_Orders "
                "WHERE PatientID = ? "
                "ORDER BY OrderID DESC LIMIT 200",
                std::stoi(query)
            );
            return rowsToOrders(result);
        }

        std::string searchTerm = "%" + query + "%";
        auto result = db->execSqlSync(
            "SELECT OrderID, PatientID, OrderType, OrderName, Dose, Frequency, "
            "OrderingProvider, OrderStatus, OrderDateTime "
            "FROM patient_Orders "
            "WHERE OrderType LIKE ? OR OrderName LIKE ? "
            "ORDER BY OrderID DESC LIMIT 200",
            searchTerm, searchTerm
        );
        return rowsToOrders(result);
    }
    catch (const drogon::orm::DrogonDbException& e)
    {
        std::cerr << "OrdersDB::searchOrders error: " << e.base().what() << std::endl;
        return {};
    }
}

// This function inserts a new order into the database with the current date and time.
bool OrdersDB::insertOrder(int patientID,
                           const std::string& orderType,
                           const std::string& orderName,
                           const std::string& dose,
                           const std::string& frequency,
                           const std::string& orderingProvider,
                           const std::string& orderStatus)
{
    try
    {
        auto db = drogon::app().getDbClient();
        db->execSqlSync(
            "INSERT INTO patient_Orders "
            "(PatientID, OrderDateTime, OrderType, OrderName, Dose, Frequency, OrderingProvider, OrderStatus) "
            "VALUES (?, NOW(), ?, ?, ?, ?, ?, ?)",
            patientID, orderType, orderName, dose, frequency, orderingProvider, orderStatus
        );
        return true;
    }
    catch (const drogon::orm::DrogonDbException& e)
    {
        std::cerr << "OrdersDB::insertOrder error: " << e.base().what() << std::endl;
        return false;
    }
}
