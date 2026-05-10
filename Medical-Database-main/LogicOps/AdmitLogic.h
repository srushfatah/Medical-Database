#pragma once
#include <string>
#include <vector>

struct AdmitResult
{
    bool        success   = false;
    int         patientID = 0;
    std::string error;
};

struct PatientInfo
{
    int         patientID  = 0;
    std::string patientName;
    std::string gender;
    int         age        = 0;
    std::string admitDate;
    std::string dischargeDate;
    std::string admitReason;
    std::string roomNumber;
    std::string attendingPhysician;
};

class AdmitLogic
{
public:
    static AdmitResult admitPatient(const std::string& patientName,
                                    const std::string& gender,
                                    int age,
                                    const std::string& admitDate,
                                    const std::string& admitReason,
                                    const std::string& roomNumber,
                                    const std::string& attendingPhysician,
                                    const std::string& dischargeDate);

    static AdmitResult dischargePatient(int patientID, const std::string& dischargeDate);

    static std::vector<PatientInfo> searchPatients(const std::string& query);
};
