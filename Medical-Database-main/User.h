#pragma once
#include <string>

struct User
{
    long long   loginID    = 0;
    int         patientID  = 0;
    std::string firstName;
    std::string lastName;
    std::string username;
    std::string passwordHash;
    std::string role;
    bool        found      = false;
};