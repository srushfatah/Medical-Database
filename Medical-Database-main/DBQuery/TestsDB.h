#pragma once
#include "TestsLogic.h"
#include <string>
#include <vector>

class TestsDB
{
public:
    static std::vector<TestResultInfo> getAllTests();
    static std::vector<TestResultInfo> searchTests(const std::string& query);
    static bool insertTest(int patientID,
                           const std::string& testType,
                           const std::string& orderingProvider,
                           const std::string& testStatus);
};
