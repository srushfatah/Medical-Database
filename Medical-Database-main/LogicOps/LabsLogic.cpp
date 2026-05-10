#include "LabsLogic.h"
#include "LabsDB.h"

// Getting all lab results from the database.
std::vector<LabResultInfo> LabsLogic::getAllLabs()
{
    return LabsDB::getAllLabs();
}

std::vector<LabResultInfo> LabsLogic::searchLabs(const std::string& query)
{
    return LabsDB::searchLabs(query);
}

// This function inserts a new lab order into the database. It will fail if the PatientID doesn't exist.
LabOrderResult LabsLogic::orderLab(int patientID,
                                   const std::string& labType,
                                   const std::string& orderingProvider,
                                   const std::string& labStatus)
{
    LabOrderResult result;

    bool inserted = LabsDB::insertLab(patientID, labType, orderingProvider, labStatus);
    if (!inserted)
    {
        result.error = "Failed to order lab. Make sure the Patient ID exists.";
        return result;
    }

    result.success = true;
    return result;
}
