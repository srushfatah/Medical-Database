#include "LoginDB.h"
#include <drogon/drogon.h>
#include <iostream>

// This function looks up a user by their username and returns their info.
// If the user doesn't exist, the returned struct will have found set to false.
// PatientID can be null for non-patient roles, so that gets checked.
User LoginDB::getUserByUsername(const std::string& username)
{
    User user;
    try
    {
        auto db     = drogon::app().getDbClient();
        auto result = db->execSqlSync(
            "SELECT LoginID, PatientID, FirstName, LastName, Username, PasswordHash, Role "
            "FROM users WHERE Username = ?",
            username
        );

        if (result.size() > 0)
        {
            auto row          = result[0];
            user.loginID      = row["LoginID"].as<long long>();
            user.patientID    = row["PatientID"].isNull() ? 0 : row["PatientID"].as<int>();
            user.firstName    = row["FirstName"].as<std::string>();
            user.lastName     = row["LastName"].as<std::string>();
            user.username     = row["Username"].as<std::string>();
            user.passwordHash = row["PasswordHash"].as<std::string>();
            user.role         = row["Role"].as<std::string>();
            user.found        = true;
        }
    }
    catch (const drogon::orm::DrogonDbException& e)
    {
        std::cerr << "LoginDB::getUserByUsername error: " << e.base().what() << std::endl;
    }
    return user;
}

// This function inserts a new user into the database. The password should already be hashed
// before calling this. If a PatientID is provided, it gets linked to the account.
bool LoginDB::insertUser(const std::string& firstName,
                         const std::string& lastName,
                         const std::string& username,
                         const std::string& passwordHash,
                         const std::string& role,
                         int patientID)
{
    try
    {
        auto db = drogon::app().getDbClient();

        if (patientID > 0)
        {
            db->execSqlSync(
                "INSERT INTO users (FirstName, LastName, Username, PasswordHash, Role, PatientID) "
                "VALUES (?, ?, ?, ?, ?, ?)",
                firstName, lastName, username, passwordHash, role, patientID
            );
        }
        else
        {
            db->execSqlSync(
                "INSERT INTO users (FirstName, LastName, Username, PasswordHash, Role) "
                "VALUES (?, ?, ?, ?, ?)",
                firstName, lastName, username, passwordHash, role
            );
        }
        return true;
    }
    catch (const drogon::orm::DrogonDbException& e)
    {
        std::cerr << "LoginDB::insertUser error: " << e.base().what() << std::endl;
        return false;
    }
}