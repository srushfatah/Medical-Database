#include "AdmitLogic.h"
#include "AdmitDB.h"

// This function admits a new patient. It checks that the age is valid (between 0 and 120),
// then inserts the record and returns the new PatientID.
AdmitResult AdmitLogic::admitPatient(const std::string& patientName,
                                     const std::string& gender,
                                     int age,
                                     const std::string& admitDate,
                                     const std::string& admitReason,
                                     const std::string& roomNumber,
                                     const std::string& attendingPhysician,
                                     const std::string& dischargeDate)
{
    AdmitResult result;

    if (age < 0 || age > 120)
    {
        result.error = "Age must be between 0 and 120.";
        return result;
    }

    int id = AdmitDB::insertPatient(patientName, gender, age, admitDate,
                                    admitReason, roomNumber, attendingPhysician, dischargeDate);
    if (id <= 0)
    {
        result.error = "Failed to admit patient. Please try again.";
        return result;
    }

    result.success   = true;
    result.patientID = id;
    return result;
}

// This function sets the discharge date for an existing patient.
AdmitResult AdmitLogic::dischargePatient(int patientID, const std::string& dischargeDate)
{
    AdmitResult result;

    bool updated = AdmitDB::updateDischargeDate(patientID, dischargeDate);
    if (!updated)
    {
        result.error = "Patient not found or discharge failed.";
        return result;
    }

    result.success   = true;
    result.patientID = patientID;
    return result;
}

// This function sends the search query to the database layer to find matching patients.
std::vector<PatientInfo> AdmitLogic::searchPatients(const std::string& query)
{
    return AdmitDB::searchPatients(query);
}
