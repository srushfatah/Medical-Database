cmake_minimum_required(VERSION 3.20)
project(MedicalBackend)

set(CMAKE_CXX_STANDARD 17)

if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
endif()

find_package(Drogon CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

add_executable(MedicalBackend
    main.cpp
    Controllers/Login.cpp
    Controllers/Admit.cpp
    Controllers/Orders.cpp
    Controllers/LabsController.cpp
    Controllers/TestsController.cpp
    Controllers/BillingController.cpp
    LogicOps/LoginLogic.cpp
    LogicOps/AdmitLogic.cpp
    LogicOps/OrdersLogic.cpp
    LogicOps/LabsLogic.cpp
    LogicOps/TestsLogic.cpp
    LogicOps/BillingLogic.cpp
    DBQuery/LoginDB.cpp
    DBQuery/AdmitDB.cpp
    DBQuery/OrdersDB.cpp
    DBQuery/LabsDB.cpp
    DBQuery/TestsDB.cpp
    DBQuery/BillingDB.cpp
)

target_include_directories(MedicalBackend PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Controllers
    ${CMAKE_CURRENT_SOURCE_DIR}/LogicOps
    ${CMAKE_CURRENT_SOURCE_DIR}/DBQuery
)

target_link_libraries(MedicalBackend PRIVATE Drogon::Drogon OpenSSL::Crypto)