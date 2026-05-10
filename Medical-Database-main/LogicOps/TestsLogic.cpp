#include "TestsLogic.h"
#include "TestsDB.h"

// Getting all test results from the database.
std::vector<TestResultInfo> TestsLogic::getAllTests()
{
    return TestsDB::getAllTests();
}

std::vector<TestResultInfo> TestsLogic::searchTests(const std::string& query)
{
    return TestsDB::searchTests(query);
}

// This function inserts a new test order into the database. It will fail if the PatientID doesn't exist.
TestOrderResult TestsLogic::orderTest(int patientID,
                                      const std::string& testType,
                                      const std::string& orderingProvider,
                                      const std::string& testStatus)
{
    TestOrderResult result;

    bool inserted = TestsDB::insertTest(patientID, testType, orderingProvider, testStatus);
    if (!inserted)
    {
        result.error = "Failed to order test. Make sure the Patient ID exists.";
        return result;
    }

    result.success = true;
    return result;
}
