#include "AdmitDB.h"
#include <drogon/drogon.h>
#include <iostream>

// This function inserts a new patient into the patient_Admit table. If a discharge date
// is provided it gets included, otherwise it's left out. Returns the new PatientID.
int AdmitDB::insertPatient(const std::string& patientName,
                           const std::string& gender,
                           int age,
                           const std::string& admitDate,
                           const std::string& admitReason,
                           const std::string& roomNumber,
                           const std::string& attendingPhysician,
                           const std::string& dischargeDate)
{
    try
    {
        auto db = drogon::app().getDbClient();

        if (dischargeDate.empty())
        {
            db->execSqlSync(
                "INSERT INTO patient_Admit "
                "(PatientName, Gender, Age, AdmitDate, AdmitReason, RoomNumber, AttendingPhysician) "
                "VALUES (?, ?, ?, ?, ?, ?, ?)",
                patientName, gender, age, admitDate, admitReason, roomNumber, attendingPhysician
            );
        }
        else
        {
            db->execSqlSync(
                "INSERT INTO patient_Admit "
                "(PatientName, Gender, Age, AdmitDate, DischargeDate, AdmitReason, RoomNumber, AttendingPhysician) "
                "VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
                patientName, gender, age, admitDate, dischargeDate, admitReason, roomNumber, attendingPhysician
            );
        }

        auto result = db->execSqlSync("SELECT LAST_INSERT_ID() AS id");
        return result[0]["id"].as<int>();
    }
    catch (const drogon::orm::DrogonDbException& e)
    {
        std::cerr << "AdmitDB::insertPatient error: " << e.base().what() << std::endl;
        return 0;
    }
}

// This function updates the discharge date for an existing patient. Returns false if the patient wasn't found.
bool AdmitDB::updateDischargeDate(int patientID, const std::string& dischargeDate)
{
    try
    {
        auto db = drogon::app().getDbClient();
        auto result = db->execSqlSync(
            "UPDATE patient_Admit SET DischargeDate = ? WHERE PatientID = ?",
            dischargeDate, patientID
        );
        return result.affectedRows() > 0;
    }
    catch (const drogon::orm::DrogonDbException& e)
    {
        std::cerr << "AdmitDB::updateDischargeDate error: " << e.base().what() << std::endl;
        return false;
    }
}

// This function searches for patients by matching the query against their name or room number.
// It returns up to 100 results with the newest ones first.
std::vector<PatientInfo> AdmitDB::searchPatients(const std::string& query)
{
    std::vector<PatientInfo> patients;
    try
    {
        auto db = drogon::app().getDbClient();
        std::string searchTerm = "%" + query + "%";

        auto result = db->execSqlSync(
            "SELECT PatientID, PatientName, Gender, Age, AdmitDate, DischargeDate, "
            "AdmitReason, RoomNumber, AttendingPhysician "
            "FROM patient_Admit "
            "WHERE PatientName LIKE ? OR RoomNumber LIKE ? "
            "ORDER BY PatientID DESC LIMIT 100",
            searchTerm, searchTerm
        );

        for (auto& row : result)
        {
            PatientInfo p;
            p.patientID          = row["PatientID"].as<int>();
            p.patientName        = row["PatientName"].as<std::string>();
            p.gender             = row["Gender"].as<std::string>();
            p.age                = row["Age"].as<int>();
            p.admitDate          = row["AdmitDate"].as<std::string>();
            p.dischargeDate      = row["DischargeDate"].isNull() ? "" : row["DischargeDate"].as<std::string>();
            p.admitReason        = row["AdmitReason"].as<std::string>();
            p.roomNumber         = row["RoomNumber"].as<std::string>();
            p.attendingPhysician = row["AttendingPhysician"].as<std::string>();
            patients.push_back(p);
        }
    }
    catch (const drogon::orm::DrogonDbException& e)
    {
        std::cerr << "AdmitDB::searchPatients error: " << e.base().what() << std::endl;
    }
    return patients;
}
