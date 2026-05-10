#pragma once
#include "BillingLogic.h"
#include <string>
#include <vector>

class BillingDB
{
public:
    static std::vector<BillingInfo> getAllBilling();
    static std::vector<BillingInfo> searchBilling(const std::string& query);
    static bool upsertBilling(int patientID,
                              double roomCharge, double visitCharge,
                              double rxCharge, double labCharge, double imagingCharge,
                              double totalCharge, double insurancePaid,
                              double patientPaid, double balanceDue);
};
