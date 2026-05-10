#include "OrdersLogic.h"
#include "OrdersDB.h"

// Getting all orders from the database.
std::vector<OrderInfo> OrdersLogic::getAllOrders()
{
    return OrdersDB::getAllOrders();
}

std::vector<OrderInfo> OrdersLogic::searchOrders(const std::string& query)
{
    return OrdersDB::searchOrders(query);
}

// This function inserts a new order into the database. It will fail if the PatientID doesn't exist.
OrderResult OrdersLogic::createOrder(int patientID,
                                     const std::string& orderType,
                                     const std::string& orderName,
                                     const std::string& dose,
                                     const std::string& frequency,
                                     const std::string& orderingProvider,
                                     const std::string& orderStatus)
{
    OrderResult result;

    bool inserted = OrdersDB::insertOrder(patientID, orderType, orderName,
                                          dose, frequency, orderingProvider, orderStatus);
    if (!inserted)
    {
        result.error = "Failed to create order. Make sure the Patient ID exists.";
        return result;
    }

    result.success = true;
    return result;
}
