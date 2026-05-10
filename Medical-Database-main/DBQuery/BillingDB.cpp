#include "BillingDB.h"
#include <drogon/drogon.h>
#include <iostream>

// This function takes the rows from the database and puts them into BillingInfo structs.
static std::vector<BillingInfo> rowsToBilling(const drogon::orm::Result& result)
{
    std::vector<BillingInfo> records;
    for (auto& row : result)
    {
        BillingInfo b;
        b.billingID     = row["BillingID"].as<long long>();
        b.patientID     = row["PatientID"].as<int>();
        b.roomCharge    = row["RoomCharge"].as<double>();
        b.visitCharge   = row["VisitCharge"].as<double>();
        b.rxCharge      = row["RxCharge"].as<double>();
        b.labCharge     = row["LabCharge"].as<double>();
        b.imagingCharge = row["ImagingCharge"].as<double>();
        b.totalCharge   = row["TotalCharge"].as<double>();
        b.insurancePaid = row["InsurancePaid"].as<double>();
        b.patientPaid   = row["PatientPaid"].as<double>();
        b.balanceDue    = row["BalanceDue"].as<double>();
        records.push_back(b);
    }
    return records;
}

// Getting all billing records from the database with the newest ones showing first.
std::vector<BillingInfo> BillingDB::getAllBilling()
{
    try
    {
        auto db = drogon::app().getDbClient();
        auto result = db->execSqlSync(
            "SELECT BillingID, PatientID, RoomCharge, VisitCharge, RxCharge, "
            "LabCharge, ImagingCharge, TotalCharge, InsurancePaid, PatientPaid, BalanceDue "
            "FROM patient_Billing ORDER BY BillingID DESC LIMIT 200"
        );
        return rowsToBilling(result);
    }
    catch (const drogon::orm::DrogonDbException& e)
    {
        std::cerr << "BillingDB::getAllBilling error: " << e.base().what() << std::endl;
        return {};
    }
}

// This function searches for billing records. If the query is a number, it searches by PatientID.
// If it's text, it tries to match against the PatientID as a string.
std::vector<BillingInfo> BillingDB::searchBilling(const std::string& query)
{
    try
    {
        auto db = drogon::app().getDbClient();

        bool isNumber = !query.empty() && query.find_first_not_of("0123456789") == std::string::npos;

        if (isNumber)
        {
            auto result = db->execSqlSync(
                "SELECT BillingID, PatientID, RoomCharge, VisitCharge, RxCharge, "
                "LabCharge, ImagingCharge, TotalCharge, InsurancePaid, PatientPaid, BalanceDue "
                "FROM patient_Billing "
                "WHERE PatientID = ? "
                "ORDER BY BillingID DESC LIMIT 200",
                std::stoi(query)
            );
            return rowsToBilling(result);
        }

        std::string searchTerm = "%" + query + "%";
        auto result = db->execSqlSync(
            "SELECT BillingID, PatientID, RoomCharge, VisitCharge, RxCharge, "
            "LabCharge, ImagingCharge, TotalCharge, InsurancePaid, PatientPaid, BalanceDue "
            "FROM patient_Billing "
            "WHERE CAST(PatientID AS CHAR) LIKE ? "
            "ORDER BY BillingID DESC LIMIT 200",
            searchTerm
        );
        return rowsToBilling(result);
    }
    catch (const drogon::orm::DrogonDbException& e)
    {
        std::cerr << "BillingDB::searchBilling error: " << e.base().what() << std::endl;
        return {};
    }
}

// This function inserts a new billing record, or if one already exists for this PatientID,
// it overwrites it with the new values instead of creating a duplicate.
bool BillingDB::upsertBilling(int patientID,
                              double roomCharge, double visitCharge,
                              double rxCharge, double labCharge, double imagingCharge,
                              double totalCharge, double insurancePaid,
                              double patientPaid, double balanceDue)
{
    try
    {
        auto db = drogon::app().getDbClient();
        db->execSqlSync(
            "INSERT INTO patient_Billing "
            "(PatientID, RoomCharge, VisitCharge, RxCharge, LabCharge, ImagingCharge, "
            "TotalCharge, InsurancePaid, PatientPaid, BalanceDue) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "
            "ON DUPLICATE KEY UPDATE "
            "RoomCharge = VALUES(RoomCharge), VisitCharge = VALUES(VisitCharge), "
            "RxCharge = VALUES(RxCharge), LabCharge = VALUES(LabCharge), "
            "ImagingCharge = VALUES(ImagingCharge), TotalCharge = VALUES(TotalCharge), "
            "InsurancePaid = VALUES(InsurancePaid), PatientPaid = VALUES(PatientPaid), "
            "BalanceDue = VALUES(BalanceDue)",
            patientID, roomCharge, visitCharge, rxCharge, labCharge, imagingCharge,
            totalCharge, insurancePaid, patientPaid, balanceDue
        );
        return true;
    }
    catch (const drogon::orm::DrogonDbException& e)
    {
        std::cerr << "BillingDB::upsertBilling error: " << e.base().what() << std::endl;
        return false;
    }
}
