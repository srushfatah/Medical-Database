#pragma once
#include <string>
#include <vector>

struct BillingInfo
{
    long long billingID     = 0;
    int       patientID     = 0;
    double    roomCharge    = 0;
    double    visitCharge   = 0;
    double    rxCharge      = 0;
    double    labCharge     = 0;
    double    imagingCharge = 0;
    double    totalCharge   = 0;
    double    insurancePaid = 0;
    double    patientPaid   = 0;
    double    balanceDue    = 0;
};

struct BillingSaveResult
{
    bool        success = false;
    std::string error;
};

class BillingLogic
{
public:
    static std::vector<BillingInfo> getAllBilling();
    static std::vector<BillingInfo> searchBilling(const std::string& query);
    static BillingSaveResult saveBilling(int patientID,
                                        double roomCharge,
                                        double visitCharge,
                                        double rxCharge,
                                        double labCharge,
                                        double imagingCharge,
                                        double insurancePaid,
                                        double patientPaid);
};
