#pragma once
#include "OrdersLogic.h"
#include <string>
#include <vector>

class OrdersDB
{
public:
    static std::vector<OrderInfo> getAllOrders();
    static std::vector<OrderInfo> searchOrders(const std::string& query);
    static bool insertOrder(int patientID,
                            const std::string& orderType,
                            const std::string& orderName,
                            const std::string& dose,
                            const std::string& frequency,
                            const std::string& orderingProvider,
                            const std::string& orderStatus);
};
