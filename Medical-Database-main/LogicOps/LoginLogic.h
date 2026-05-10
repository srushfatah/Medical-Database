#pragma once
#include <string>

// This struct holds the result of a login or register attempt.
struct LoginResult
{
    bool        success    = false;
    int         patientID  = 0;
    std::string role;
    std::string username;
    std::string error;
};

class LoginLogic
{
public:
    static LoginResult login(const std::string& username,
                             const std::string& password);

    static LoginResult registerUser(const std::string& firstName,
                                    const std::string& lastName,
                                    const std::string& username,
                                    const std::string& password,
                                    const std::string& role,
                                    int patientID = 0);
};