#include "BillingLogic.h"
#include "BillingDB.h"

// Getting all billing records from the database.
std::vector<BillingInfo> BillingLogic::getAllBilling()
{
    return BillingDB::getAllBilling();
}

std::vector<BillingInfo> BillingLogic::searchBilling(const std::string& query)
{
    return BillingDB::searchBilling(query);
}

// This function saves a billing record. It adds up all the charges to get the total, then
// subtracts payments to get the balance due. If a billing record already exists for this
// PatientID, it overwrites it instead of creating a duplicate.
BillingSaveResult BillingLogic::saveBilling(int patientID,
                                           double roomCharge,
                                           double visitCharge,
                                           double rxCharge,
                                           double labCharge,
                                           double imagingCharge,
                                           double insurancePaid,
                                           double patientPaid)
{
    BillingSaveResult result;

    double totalCharge = roomCharge + visitCharge + rxCharge + labCharge + imagingCharge;
    double balanceDue  = totalCharge - insurancePaid - patientPaid;

    bool saved = BillingDB::upsertBilling(patientID, roomCharge, visitCharge, rxCharge,
                                          labCharge, imagingCharge, totalCharge,
                                          insurancePaid, patientPaid, balanceDue);
    if (!saved)
    {
        result.error = "Failed to save billing. Make sure the Patient ID exists.";
        return result;
    }

    result.success = true;
    return result;
}
