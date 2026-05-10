#pragma once
#include "User.h"
#include <string>

class LoginDB
{
public:
    // Returns a User object — check user.found to see if the query matched anything
    static User getUserByUsername(const std::string& username);

    // Returns true if insert succeeded, false if it failed
    static bool insertUser(const std::string& firstName,
                           const std::string& lastName,
                           const std::string& username,
                           const std::string& password,
                           const std::string& role,
                           int patientID = 0);
};