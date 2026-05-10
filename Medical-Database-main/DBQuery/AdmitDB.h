#pragma once
#include "AdmitLogic.h"
#include <string>
#include <vector>

class AdmitDB
{
public:
    // Returns the new PatientID, or 0 on failure
    static int insertPatient(const std::string& patientName,
                             const std::string& gender,
                             int age,
                             const std::string& admitDate,
                             const std::string& admitReason,
                             const std::string& roomNumber,
                             const std::string& attendingPhysician,
                             const std::string& dischargeDate);

    // Returns true if the update succeeded
    static bool updateDischargeDate(int patientID, const std::string& dischargeDate);

    // Search by name or room number
    static std::vector<PatientInfo> searchPatients(const std::string& query);
};
