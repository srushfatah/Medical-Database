#include "LoginLogic.h"
#include "LoginDB.h"
#include <drogon/drogon.h>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>

// This function takes a plain text string and hashes it using SHA-256.
// It's used for hashing passwords when storing them and when checking them during login.
static std::string sha256(const std::string& input)
{
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), hash);
    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    return ss.str();
}

// This function handles logging in. It looks up the user in the database and compares
// the hashed password. If it matches, it returns the user's info so the session can be set up.
LoginResult LoginLogic::login(const std::string& username,
                               const std::string& password)
{
    LoginResult result;

    User user = LoginDB::getUserByUsername(username);

    // Using the same error message whether the username doesn't exist or
    // the password is wrong, so it doesn't give away which one is incorrect.
    if (!user.found || user.passwordHash != sha256(password))
    {
        result.error = "Invalid username or password.";
        return result;
    }

    result.success   = true;
    result.patientID = user.patientID;
    result.role      = user.role;
    result.username  = user.username;
    return result;
}

// This function registers a new user. It replaces spaces in the role with underscores so it
// matches the database, checks if the username is taken, hashes the password, and inserts
// the new user. If a PatientID is provided, it checks that it exists in patient_Admit first.
LoginResult LoginLogic::registerUser(const std::string& firstName,
                                      const std::string& lastName,
                                      const std::string& username,
                                      const std::string& password,
                                      const std::string& role,
                                      int patientID)
{
    LoginResult result;

    // Replacing spaces with underscores in the role so it matches the database format.
    std::string normalizedRole = role;
    for (auto& c : normalizedRole)
        if (c == ' ') c = '_';

    // Checking if the username is already taken.
    User existing = LoginDB::getUserByUsername(username);
    if (existing.found)
    {
        result.error = "Username is already taken.";
        return result;
    }

    // If a PatientID was provided, make sure it actually exists in the patient_Admit table.
    if (patientID > 0)
    {
        try
        {
            auto db = drogon::app().getDbClient();
            auto check = db->execSqlSync(
                "SELECT PatientID FROM patient_Admit WHERE PatientID = ?",
                patientID
            );
            if (check.size() == 0)
            {
                result.error = "Patient ID not found. Please check with front desk.";
                return result;
            }
        }
        catch (...)
        {
            result.error = "Could not verify Patient ID. Please try again.";
            return result;
        }
    }

    std::string hashed = sha256(password);
    bool inserted = LoginDB::insertUser(firstName, lastName, username, hashed, normalizedRole, patientID);
    if (!inserted)
    {
        result.error = "Failed to create account. Please try again.";
        return result;
    }

    result.success  = true;
    result.username = username;
    result.role     = normalizedRole;
    return result;
}