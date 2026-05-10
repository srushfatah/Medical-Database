#pragma once
#include <string>
#include <vector>

struct LabResultInfo
{
    long long   labResultID = 0;
    int         patientID   = 0;
    std::string labType;
    std::string resultName;
    std::string resultValue;
    std::string unit;
    std::string referenceRange;
    std::string flag;
    std::string dateTime;
    std::string labStatus;
};

struct LabOrderResult
{
    bool        success = false;
    std::string error;
};

class LabsLogic
{
public:
    static std::vector<LabResultInfo> getAllLabs();
    static std::vector<LabResultInfo> searchLabs(const std::string& query);
    static LabOrderResult orderLab(int patientID,
                                   const std::string& labType,
                                   const std::string& orderingProvider,
                                   const std::string& labStatus);
};
