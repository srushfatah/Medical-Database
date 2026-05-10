#pragma once
#include <string>
#include <vector>

struct TestResultInfo
{
    long long   testResultID = 0;
    int         patientID    = 0;
    std::string testType;
    std::string impression;
    std::string findings;
    std::string dateTime;
    std::string testStatus;
};

struct TestOrderResult
{
    bool        success = false;
    std::string error;
};

class TestsLogic
{
public:
    static std::vector<TestResultInfo> getAllTests();
    static std::vector<TestResultInfo> searchTests(const std::string& query);
    static TestOrderResult orderTest(int patientID,
                                     const std::string& testType,
                                     const std::string& orderingProvider,
                                     const std::string& testStatus);
};
