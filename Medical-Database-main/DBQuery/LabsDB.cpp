#include "LabsDB.h"
#include <drogon/drogon.h>
#include <iostream>

// This function takes the rows from the database and puts them into LabResultInfo structs.
// Some columns might be null if a lab hasn't gotten results back yet, so those get checked first.
static std::vector<LabResultInfo> rowsToLabs(const drogon::orm::Result& result)
{
    std::vector<LabResultInfo> labs;
    for (auto& row : result)
    {
        LabResultInfo l;
        l.labResultID    = row["LabResultID"].isNull() ? 0 : row["LabResultID"].as<long long>();
        l.patientID      = row["PatientID"].as<int>();
        l.labType        = row["LabType"].as<std::string>();
        l.resultName     = row["ResultName"].isNull() ? "" : row["ResultName"].as<std::string>();
        l.resultValue    = row["ResultValue"].isNull() ? "" : row["ResultValue"].as<std::string>();
        l.unit           = row["Unit"].isNull() ? "" : row["Unit"].as<std::string>();
        l.referenceRange = row["ReferenceRange"].isNull() ? "" : row["ReferenceRange"].as<std::string>();
        l.flag           = row["Flag"].isNull() ? "" : row["Flag"].as<std::string>();
        l.dateTime       = row["LabDateTime"].as<std::string>();
        l.labStatus      = row["LabStatus"].as<std::string>();
        labs.push_back(l);
    }
    return labs;
}

// Getting all lab results from the database with the newest ones showing first.
std::vector<LabResultInfo> LabsDB::getAllLabs()
{
    try
    {
        auto db = drogon::app().getDbClient();
        auto result = db->execSqlSync(
            "SELECT lr.LabResultID, l.PatientID, l.LabType, lr.ResultName, "
            "lr.ResultValue, lr.Unit, lr.ReferenceRange, lr.Flag, l.LabDateTime, l.LabStatus "
            "FROM Labs l "
            "LEFT JOIN Lab_Results lr ON lr.LabID = l.LabID "
            "ORDER BY l.LabID DESC LIMIT 200"
        );
        return rowsToLabs(result);
    }
    catch (const drogon::orm::DrogonDbException& e)
    {
        std::cerr << "LabsDB::getAllLabs error: " << e.base().what() << std::endl;
        return {};
    }
}

// This function searches for labs. If the query is a number, it searches by PatientID.
// If it's text, it searches by the lab type.
std::vector<LabResultInfo> LabsDB::searchLabs(const std::string& query)
{
    try
    {
        auto db = drogon::app().getDbClient();

        bool isNumber = !query.empty() && query.find_first_not_of("0123456789") == std::string::npos;

        if (isNumber)
        {
            auto result = db->execSqlSync(
                "SELECT lr.LabResultID, l.PatientID, l.LabType, lr.ResultName, "
                "lr.ResultValue, lr.Unit, lr.ReferenceRange, lr.Flag, l.LabDateTime, l.LabStatus "
                "FROM Labs l "
                "LEFT JOIN Lab_Results lr ON lr.LabID = l.LabID "
                "WHERE l.PatientID = ? "
                "ORDER BY l.LabID DESC LIMIT 200",
                std::stoi(query)
            );
            return rowsToLabs(result);
        }

        std::string searchTerm = "%" + query + "%";
        auto result = db->execSqlSync(
            "SELECT lr.LabResultID, l.PatientID, l.LabType, lr.ResultName, "
            "lr.ResultValue, lr.Unit, lr.ReferenceRange, lr.Flag, l.LabDateTime, l.LabStatus "
            "FROM Labs l "
            "LEFT JOIN Lab_Results lr ON lr.LabID = l.LabID "
            "WHERE l.LabType LIKE ? "
            "ORDER BY l.LabID DESC LIMIT 200",
            searchTerm
        );
        return rowsToLabs(result);
    }
    catch (const drogon::orm::DrogonDbException& e)
    {
        std::cerr << "LabsDB::searchLabs error: " << e.base().what() << std::endl;
        return {};
    }
}

// This function inserts a new lab order into the database with the current date and time.
bool LabsDB::insertLab(int patientID,
                       const std::string& labType,
                       const std::string& orderingProvider,
                       const std::string& labStatus)
{
    try
    {
        auto db = drogon::app().getDbClient();
        db->execSqlSync(
            "INSERT INTO Labs (PatientID, LabDateTime, LabType, OrderingProvider, LabStatus) "
            "VALUES (?, NOW(), ?, ?, ?)",
            patientID, labType, orderingProvider, labStatus
        );
        return true;
    }
    catch (const drogon::orm::DrogonDbException& e)
    {
        std::cerr << "LabsDB::insertLab error: " << e.base().what() << std::endl;
        return false;
    }
}
