#pragma once
#include "LabsLogic.h"
#include <string>
#include <vector>

class LabsDB
{
public:
    static std::vector<LabResultInfo> getAllLabs();
    static std::vector<LabResultInfo> searchLabs(const std::string& query);
    static bool insertLab(int patientID,
                          const std::string& labType,
                          const std::string& orderingProvider,
                          const std::string& labStatus);
};
