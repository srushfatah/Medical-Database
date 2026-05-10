#pragma once
#include <string>
#include <vector>

struct OrderInfo
{
    long long   orderID          = 0;
    int         patientID        = 0;
    std::string orderType;
    std::string orderName;
    std::string dose;
    std::string frequency;
    std::string orderingProvider;
    std::string orderStatus;
    std::string orderDateTime;
};

struct OrderResult
{
    bool        success = false;
    std::string error;
};

class OrdersLogic
{
public:
    static std::vector<OrderInfo> getAllOrders();
    static std::vector<OrderInfo> searchOrders(const std::string& query);
    static OrderResult createOrder(int patientID,
                                   const std::string& orderType,
                                   const std::string& orderName,
                                   const std::string& dose,
                                   const std::string& frequency,
                                   const std::string& orderingProvider,
                                   const std::string& orderStatus);
};
