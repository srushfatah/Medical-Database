#include "TestsDB.h"
#include <drogon/drogon.h>
#include <iostream>

// This function takes the rows from the database and puts them into TestResultInfo structs.
// Some columns might be null if a test hasn't gotten results back yet, so those get checked first.
static std::vector<TestResultInfo> rowsToTests(const drogon::orm::Result& result)
{
    std::vector<TestResultInfo> tests;
    for (auto& row : result)
    {
        TestResultInfo t;
        t.testResultID = row["TestResultID"].isNull() ? 0 : row["TestResultID"].as<long long>();
        t.patientID    = row["PatientID"].as<int>();
        t.testType     = row["TestType"].as<std::string>();
        t.impression   = row["Impression"].isNull() ? "" : row["Impression"].as<std::string>();
        t.findings     = row["Findings"].isNull() ? "" : row["Findings"].as<std::string>();
        t.dateTime     = row["TestDateTime"].as<std::string>();
        t.testStatus   = row["TestStatus"].as<std::string>();
        tests.push_back(t);
    }
    return tests;
}

// Getting all test results from the database with the newest ones showing first.
std::vector<TestResultInfo> TestsDB::getAllTests()
{
    try
    {
        auto db = drogon::app().getDbClient();
        auto result = db->execSqlSync(
            "SELECT tr.TestResultID, t.PatientID, t.TestType, "
            "tr.Impression, tr.Findings, t.TestDateTime, t.TestStatus "
            "FROM Tests t "
            "LEFT JOIN Test_Results tr ON tr.TestID = t.TestID "
            "ORDER BY t.TestID DESC LIMIT 200"
        );
        return rowsToTests(result);
    }
    catch (const drogon::orm::DrogonDbException& e)
    {
        std::cerr << "TestsDB::getAllTests error: " << e.base().what() << std::endl;
        return {};
    }
}

// This function searches for tests. If the query is a number, it searches by PatientID.
// If it's text, it searches by the test type.
std::vector<TestResultInfo> TestsDB::searchTests(const std::string& query)
{
    try
    {
        auto db = drogon::app().getDbClient();

        bool isNumber = !query.empty() && query.find_first_not_of("0123456789") == std::string::npos;

        if (isNumber)
        {
            auto result = db->execSqlSync(
                "SELECT tr.TestResultID, t.PatientID, t.TestType, "
                "tr.Impression, tr.Findings, t.TestDateTime, t.TestStatus "
                "FROM Tests t "
                "LEFT JOIN Test_Results tr ON tr.TestID = t.TestID "
                "WHERE t.PatientID = ? "
                "ORDER BY t.TestID DESC LIMIT 200",
                std::stoi(query)
            );
            return rowsToTests(result);
        }

        std::string searchTerm = "%" + query + "%";
        auto result = db->execSqlSync(
            "SELECT tr.TestResultID, t.PatientID, t.TestType, "
            "tr.Impression, tr.Findings, t.TestDateTime, t.TestStatus "
            "FROM Tests t "
            "LEFT JOIN Test_Results tr ON tr.TestID = t.TestID "
            "WHERE t.TestType LIKE ? "
            "ORDER BY t.TestID DESC LIMIT 200",
            searchTerm
        );
        return rowsToTests(result);
    }
    catch (const drogon::orm::DrogonDbException& e)
    {
        std::cerr << "TestsDB::searchTests error: " << e.base().what() << std::endl;
        return {};
    }
}

// This function inserts a new test order into the database with the current date and time.
bool TestsDB::insertTest(int patientID,
                         const std::string& testType,
                         const std::string& orderingProvider,
                         const std::string& testStatus)
{
    try
    {
        auto db = drogon::app().getDbClient();
        db->execSqlSync(
            "INSERT INTO Tests (PatientID, TestDateTime, TestType, OrderingProvider, TestStatus) "
            "VALUES (?, NOW(), ?, ?, ?)",
            patientID, testType, orderingProvider, testStatus
        );
        return true;
    }
    catch (const drogon::orm::DrogonDbException& e)
    {
        std::cerr << "TestsDB::insertTest error: " << e.base().what() << std::endl;
        return false;
    }
}
