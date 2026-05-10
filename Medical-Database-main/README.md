Medical Management Database System:

This project is a full-stack hospital managment system. The application supports patient admitting, medical orders, lab and test tracking, and billing. It uses role-based access control so that admins, doctors, nurses, patients, and front desk staff each see only what they are authorized to. The backend is written in C++ using the Drogon web framework, with MySQL handling the database. The frontend uses HTML/CSS/JavaScript.

----------------------------------------------------------------------------------------------------------------------
[Installation]:

Make sure you have git installed and check the setting that allows you to use it on different terminals (aside from Git Bash).

Make sure you also have Cmake & a C++ compiler installed.


----------------------------------------------------------------------------------------------------------------------
Part 1 (Installing vcpkg globally):

You may already have vcpkg installed if you have Visual Studio, but I recommend installing it once more following the instructions I'm about to give, because this will install it directly to the C drive and allow for other programs to easily use vcpkg without having to make a long path, and cmake was not working when I tried using it with the vcpkg I had installed with visual studio.

[Windows]:
Open up Command Prompt (make sure it's Command Prompt specifically) and run these commands:

cd C:\
git clone https://github.com/microsoft/vcpkg
cd vcpkg
.\bootstrap-vcpkg.bat

[Then test it by typing]: vcpkg.exe --version
(if an error message appears, just make sure you see the vcpkg folder in the C drive and everything should still work)

[Mac]:
Open Terminal and run the following:

cd ~ 
git clone https://github.com/microsoft/vcpkg
cd vcpkg
./bootstrap-vcpkg.sh

You can test it with this (if an error message shows up it'll probably be fine if the code from above ran successfully):
./vcpkg --version

----------------------------------------------------------------------------------------------------------------------
Part 2 (RESTART COMMAND PROMPT/TERMINAL AFTER DOING THIS STEP):

[Windows (using Command Prompt)]:

setx VCPKG_ROOT "C:\vcpkg"

[Mac (using Terminal)]:

echo 'export VCPKG_ROOT=~/vcpkg' >> ~/.zshrc

Then restart Terminal before continuing.


----------------------------------------------------------------------------------------------------------------------
Part 3 (getting the GitHub repo):

In a terminal such as command prompt or vscode, go to the folder where you want to store the backend files by typing "cd" then paste the path to the folder (you can copy the path by right clicking a folder and clicking "copy as path").
Next, type this:

git clone https://github.com/Cel4h/4560-Medical-Database
cd 4560-Medical-Database


----------------------------------------------------------------------------------------------------------------------
Part 4 (building the project; WARNING: It can take a long time to build, like 10-30 minutes):

Open config.json and replace "your-password-here" with your MySQL password
Also, replace "root" with whatever your MySQL username is, root is just the default username, yours is probably root if you don't remember changing it

VERY IMPORTANT before doing these next commands in command prompt/terminal, make sure you point the directory to the project folder, so in command prompt you'd type "cd (path to 4560-Medical-Database)"

[Windows (using Command Prompt)]:
paste these lines of code and execute one at a time:

cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake

cmake --build build


Next, copy the config.json file by just right clicking and hitting "copy", then paste it into the build->Debug folder

Next, copy the entire frontend folder from inside 4560-Medical-Database and paste it into build->Debug. So the frontend folder should end up at 4560-Medical-Database->build->Debug->frontend

[Mac (using Terminal)]:

IMPORTANT: If you have Oracle MySQL installed at /usr/local/mysql, the build may fail due to a
header conflict. If it does, run the following before building:
sudo mv /usr/local/mysql/include /usr/local/mysql/include.bak
Then rebuild. You can restore it afterwards with:
sudo mv /usr/local/mysql/include.bak /usr/local/mysql/include

cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build build

Next, copy config.json into the build folder:
cp config.json build/

Next, copy the entire frontend folder into the build folder:
cp -r frontend build/

After that, open up MySQL Workbench and open up setup.sql in it (located in the 4560-Medical-Database folder. Also make sure you are using the medicaltestdb database, if you don't have one made just type "CREATE DATABASE medicaltestdb;" then: "USE medicaltestdb;") and run it. If it gives you an error, you are likely running it on safe mode, which you can disable by going to edit at the top, then preferences->SQL Editor-> then uncheck "Safe Updates" at the bottom and restart the server. You can do that by either restarting your computer, or if you are on windows, you can press Windows+R, running services.MSC, then scrolling down to MySQL and restarting it there. You may also be able to do it from the workbench itself.

----------------------------------------------------------------------------------------------------------------------
Part 5 (run the server):

[Windows]:
Before trying to run the server, go to 4560-Medical-Database->build->Debug and check if there is a folder called "lib". If that folder is missing, then you must do the following:
	Open up command prompt and cd into the project folder (something like "cd ./4560-Medical-Database")
	once there, type in this command and run it: 

    dir /s /b build\caching_sha2_password.dll
	
    If it ran correctly, it should print the path to the "caching_sha2_password.dll" file, copy that path for now.
	type this into cmd and run it: 
    
    copy "(the path you just copied)" build\Debug\

[Mac]:
On Mac, the executable is built directly in the build/ folder (not a Debug subfolder).
To run the server, open Terminal, cd into the project folder, and run:
	./build/MedicalBackend

Note: The caching_sha2_password file is not required on Mac.


----------------------------------------------------------------------------------------------------------------------
How to Run:

Go into the repo folder (should be called "4560-Medical-Database") and go to build->Debug then double click on the file named "MedicalBackend.exe". This will run the server.

Go on a browser and type "http://localhost:8848/index.html"

To close the server, just close the terminal window running MedicalBackend, or press CTRL+C in that same terminal.


----------------------------------------------------------------------------------------------------------------------
Tech Stack:

-Backend: C++ with the Drogon web framework

-Database: MySQL

-Frontend: HTML, CSS, JavaScript

-Authentication: SHA-256 password hashing via OpenSSL

-Session Management: Server-sdie sessions through Drogon

----------------------------------------------------------------------------------------------------------------------
Project Structure:

-Controllers/ : Handles incoming API requests and sends back responses

-LogicOps/ : Business logic layer that sits between the controllers and the database

-DBQuery/ : Database queries and direct interaction with MySQL

-frontend/ : HTML pages, CSS styling, and JavaScript for the browser

-main.cpp : Entry point that sets up middleware, CORS, and role-based access control

-setup.sql : Creates the database, tables, and populates sample data

----------------------------------------------------------------------------------------------------------------------
Additional Notes:
-If you order a lab or a test using a doctor, nurse, or admin account, the record for that order in the Lab/Test Results section will appear empty. This is intentional, as the application does not offer a way to input lab/test results. If this were implemented in a real-world setting, lab/test specialists would have their own application to input test/lab results, which would then get sent to this application. Once the result comes in, the previously-empty record in the lab/test results section would no longer appear empty.

-Doctors and nurses see an empty table on the labs, tests, and orders pages until they search by Patient ID.

-Patients can only see their own data across all pages.

-The Patient ID field on the registration page only appears when the "Patient" role is selected.
    When an Admin or Front Desk admits a new patient, a new patientID get generated automatically, so if a user wants to set up a patient account tied to a specific patientID, they'd need to get it from an Admin or Front Desk staff member first

-Drogon connects to a database by looking at the config.json file, so we could easily implement this application into a cloud-based database just by changing the information in that config file and changing the queries in the DBQuery section to match the query language of the cloud database.

----------------------------------------------------------------------------------------------------------------------
Contributers:

Srush Fatah & Luis Ruiz - Frontend

Josh Heath - Backend

Caleb Lykens - SQL/Database Setup
