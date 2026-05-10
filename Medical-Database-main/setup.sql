-- ============================================================
--  setup.sql
--  Run this once in MySQL Workbench to set up everything.
--  Replace 'your_password_here' with your MySQL root password.
-- ============================================================

-- 1. Create and select the database
DROP DATABASE IF EXISTS medicaltestdb;
CREATE DATABASE IF NOT EXISTS medicaltestdb;
USE medicaltestdb;

-- ============================================================
--  CREATE TABLES  (CreateFirst3Tables.sql)
-- ============================================================

-- Drop in dependency order
DROP TABLE IF EXISTS Test_Results;
DROP TABLE IF EXISTS Lab_Results;
DROP TABLE IF EXISTS Tests;
DROP TABLE IF EXISTS Labs;
DROP TABLE IF EXISTS patient_Billing;
DROP TABLE IF EXISTS patient_Orders;
DROP TABLE IF EXISTS users;
DROP TABLE IF EXISTS patient_Admit;

-- 1) Admitting / demographics table
CREATE TABLE patient_Admit (
  PatientID        INT AUTO_INCREMENT PRIMARY KEY,
  PatientName      VARCHAR(120) NOT NULL,
  Gender           CHAR(1) NOT NULL CHECK (Gender IN ('M','F','O')),
  Age              INT NOT NULL CHECK (Age BETWEEN 0 AND 120),

  AdmitDate        DATE NOT NULL,
  DischargeDate    DATE NULL,
  AdmitReason      VARCHAR(120) NOT NULL,

  RoomNumber       VARCHAR(10) NOT NULL,
  AttendingPhysician VARCHAR(120) NOT NULL
);

-- 2) Orders table (many per patient)
CREATE TABLE patient_Orders (
  OrderID          BIGINT AUTO_INCREMENT PRIMARY KEY,
  PatientID        INT NOT NULL REFERENCES patient_Admit(PatientID) ON DELETE CASCADE,

  OrderDateTime    TIMESTAMP NOT NULL,
  OrderType        VARCHAR(40) NOT NULL,   -- e.g. Medication, Lab, Imaging
  OrderName        VARCHAR(120) NOT NULL,  -- e.g. Amoxicillin 500mg, CBC, X-Ray Chest
  Dose             VARCHAR(40) NULL,
  Frequency        VARCHAR(40) NULL,
  OrderingProvider VARCHAR(120) NOT NULL,
  OrderStatus      VARCHAR(20) NOT NULL CHECK (OrderStatus IN ('Ordered','Completed','Canceled'))
);

-- 3) Billing table (one per patient, totals can be derived or stored)
CREATE TABLE patient_Billing (
  BillingID        BIGINT AUTO_INCREMENT PRIMARY KEY,
  PatientID        INT NOT NULL UNIQUE REFERENCES patient_Admit(PatientID) ON DELETE CASCADE,

  RoomCharge       NUMERIC(10,2) NOT NULL,
  VisitCharge      NUMERIC(10,2) NOT NULL,
  RxCharge         NUMERIC(10,2) NOT NULL,
  LabCharge        NUMERIC(10,2) NOT NULL,
  ImagingCharge    NUMERIC(10,2) NOT NULL,

  TotalCharge      NUMERIC(10,2) NOT NULL,
  InsurancePaid    NUMERIC(10,2) NOT NULL,
  PatientPaid      NUMERIC(10,2) NOT NULL,
  BalanceDue       NUMERIC(10,2) NOT NULL
);

CREATE TABLE users (
    LoginID        BIGINT AUTO_INCREMENT PRIMARY KEY,
    PatientID      INT NULL UNIQUE,

    FirstName      VARCHAR(50) NOT NULL,
    LastName       VARCHAR(50) NOT NULL,
    Username       VARCHAR(50) NOT NULL UNIQUE,

    PasswordHash   VARCHAR(255) NOT NULL,
    Role           ENUM('admin', 'doctor', 'nurse', 'patient', 'front_desk') NOT NULL,

    CreatedAt      DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    LastLoginAt    DATETIME NULL,

    IsActive       TINYINT(1) NOT NULL DEFAULT 1,

    FailedAttempts INT NOT NULL DEFAULT 0,
    LockedUntil    DATETIME NULL,

    CONSTRAINT fk_login_patient
        FOREIGN KEY (PatientID) REFERENCES patient_Admit(PatientID)
        ON DELETE CASCADE
);

-- ============================================================
--  CREATE TABLES  (Create Labs_Tests.sql)
-- ============================================================

-- LAB ORDERS (3-4 common lab types)
CREATE TABLE Labs (
  LabID BIGINT AUTO_INCREMENT PRIMARY KEY,
  PatientID INT NOT NULL,
  LabDateTime DATETIME NOT NULL,
  LabType ENUM('CBC','BMP','A1C','Lipid Panel') NOT NULL,
  OrderingProvider VARCHAR(120) NOT NULL,
  LabStatus ENUM('Ordered','Collected','Completed','Canceled') NOT NULL,

  CONSTRAINT fk_labs_patient
    FOREIGN KEY (PatientID) REFERENCES patient_Admit(PatientID)
    ON DELETE CASCADE
);

-- LAB RESULTS (one-to-many details per lab)
CREATE TABLE Lab_Results (
  LabResultID BIGINT AUTO_INCREMENT PRIMARY KEY,
  LabID BIGINT NOT NULL,
  PatientID INT NOT NULL,
  ResultName VARCHAR(80) NOT NULL,          -- e.g., WBC, Hemoglobin, Glucose
  ResultValue VARCHAR(40) NOT NULL,         -- store as string for simplicity (e.g., "7.2", "Negative")
  Unit VARCHAR(20) NULL,                    -- e.g., "K/uL", "mg/dL"
  ReferenceRange VARCHAR(40) NULL,          -- e.g., "4.0-11.0"
  Flag ENUM('Normal','High','Low','Critical') NOT NULL DEFAULT 'Normal',

  CONSTRAINT fk_labresults_lab
    FOREIGN KEY (LabID) REFERENCES Labs(LabID)
    ON DELETE CASCADE,

  CONSTRAINT fk_labresults_patient
    FOREIGN KEY (PatientID) REFERENCES patient_Admit(PatientID)
    ON DELETE CASCADE
);

-- DIAGNOSTIC TESTS (3-4 common imaging/diagnostic tests)
CREATE TABLE Tests (
  TestID BIGINT AUTO_INCREMENT PRIMARY KEY,
  PatientID INT NOT NULL,
  TestDateTime DATETIME NOT NULL,
  TestType ENUM('X-Ray Chest','CT Head','EKG','Ultrasound Abdomen') NOT NULL,
  OrderingProvider VARCHAR(120) NOT NULL,
  TestStatus ENUM('Ordered','In Progress','Completed','Canceled') NOT NULL,

  CONSTRAINT fk_tests_patient
    FOREIGN KEY (PatientID) REFERENCES patient_Admit(PatientID)
    ON DELETE CASCADE
);

-- TEST RESULTS (mostly narrative findings)
CREATE TABLE Test_Results (
  TestResultID BIGINT AUTO_INCREMENT PRIMARY KEY,
  TestID BIGINT NOT NULL,
  PatientID INT NOT NULL,
  Impression VARCHAR(255) NOT NULL,       -- short summary
  Findings TEXT NULL,                      -- optional details

  CONSTRAINT fk_testresults_test
    FOREIGN KEY (TestID) REFERENCES Tests(TestID)
    ON DELETE CASCADE,

  CONSTRAINT fk_testresults_patient
    FOREIGN KEY (PatientID) REFERENCES patient_Admit(PatientID)
    ON DELETE CASCADE
);

-- ============================================================
--  POPULATE TABLES  (Populate Users Table.sql)
-- ============================================================

SET SESSION cte_max_recursion_depth = 100000;

INSERT INTO users (FirstName, LastName, Username, PasswordHash, Role)
WITH RECURSIVE seq AS (
    SELECT 1 AS n
    UNION ALL
    SELECT n + 1 FROM seq WHERE n < 100000
)
SELECT
    CONCAT('First', LPAD(n, 6, '0')) AS FirstName,
    CONCAT('Last', LPAD(n, 6, '0')) AS LastName,
    CONCAT('user', LPAD(n, 6, '0')) AS Username,
    SHA2(CONCAT('password', n), 256) AS PasswordHash,
    CASE MOD(n, 5)
        WHEN 0 THEN 'admin'
        WHEN 1 THEN 'doctor'
        WHEN 2 THEN 'nurse'
        WHEN 3 THEN 'patient'
        ELSE 'front_desk'
    END AS Role
FROM seq;

-- ============================================================
--  POPULATE TABLES  (Populate Admit Table.sql)
-- ============================================================

SET SESSION cte_max_recursion_depth = 100000;
INSERT INTO patient_Admit
    (PatientID, PatientName, Gender, Age, AdmitDate, DischargeDate, AdmitReason, RoomNumber, AttendingPhysician)
WITH RECURSIVE seq AS (
    SELECT 1 AS n
    UNION ALL
    SELECT n + 1 FROM seq WHERE n < 100000
)
SELECT
    n AS PatientID,
    CONCAT('Patient ', LPAD(n, 6, '0')) AS PatientName,
    CASE MOD(n, 3)
        WHEN 0 THEN 'M'
        WHEN 1 THEN 'F'
        ELSE 'O'
    END AS Gender,
    1 + MOD(n * 37, 90) AS Age,
    DATE_SUB(CURDATE(), INTERVAL MOD(n * 13, 365) DAY) AS AdmitDate,
    CASE
        WHEN MOD(n, 5) = 0 THEN NULL
        ELSE DATE_ADD(
            DATE_SUB(CURDATE(), INTERVAL MOD(n * 13, 365) DAY),
            INTERVAL MOD(n * 7, 10) DAY
        )
    END AS DischargeDate,
    CASE MOD(n, 6)
        WHEN 0 THEN 'Observation'
        WHEN 1 THEN 'Chest Pain'
        WHEN 2 THEN 'Infection'
        WHEN 3 THEN 'Post-Op'
        WHEN 4 THEN 'Injury'
        ELSE 'Routine Visit'
    END AS AdmitReason,
    CONCAT('R', LPAD((MOD(n * 19, 450) + 100), 3, '0')) AS RoomNumber,
    CASE MOD(n, 5)
        WHEN 0 THEN 'Dr. Rivera'
        WHEN 1 THEN 'Dr. Nguyen'
        WHEN 2 THEN 'Dr. Patel'
        WHEN 3 THEN 'Dr. Johnson'
        ELSE 'Dr. Smith'
    END AS AttendingPhysician
FROM seq;

-- ============================================================
--  LINK PATIENT-ROLE USERS TO PATIENT RECORDS
-- ============================================================

SET @pid := 0;
UPDATE users SET PatientID = (@pid := @pid + 1)
WHERE Role = 'patient'
ORDER BY LoginID;

-- ============================================================
--  POPULATE TABLES  (Populate Orders Table.sql)
-- ============================================================

SET SESSION cte_max_recursion_depth = 10000;
DROP TEMPORARY TABLE IF EXISTS num3;
CREATE TEMPORARY TABLE num3 (i INT PRIMARY KEY);
INSERT INTO num3 VALUES (1),(2),(3);

INSERT INTO patient_Orders
    (PatientID, OrderDateTime, OrderType, OrderName, Dose, Frequency, OrderingProvider, OrderStatus)
SELECT
    a.PatientID,

    -- place orders near admit time
    DATE_ADD(a.AdmitDate, INTERVAL (n3.i * 6 + MOD(a.PatientID * 11, 6)) HOUR) AS OrderDateTime,

    CASE MOD(n3.i, 3)
        WHEN 1 THEN 'Medication'
        WHEN 2 THEN 'Lab'
        ELSE 'Imaging'
    END AS OrderType,

    CASE MOD(n3.i, 3)
        WHEN 1 THEN
            CASE MOD(a.PatientID, 5)
                WHEN 0 THEN 'Amoxicillin'
                WHEN 1 THEN 'Lisinopril'
                WHEN 2 THEN 'Metformin'
                WHEN 3 THEN 'Ibuprofen'
                ELSE 'Atorvastatin'
            END
        WHEN 2 THEN
            CASE MOD(a.PatientID, 4)
                WHEN 0 THEN 'CBC'
                WHEN 1 THEN 'BMP'
                WHEN 2 THEN 'Lipid Panel'
                ELSE 'A1C'
            END
        ELSE
            CASE MOD(a.PatientID, 3)
                WHEN 0 THEN 'X-Ray Chest'
                WHEN 1 THEN 'CT Head'
                ELSE 'Ultrasound Abdomen'
            END
    END AS OrderName,

    CASE
        WHEN MOD(n3.i, 3) = 1 THEN
            CASE MOD(a.PatientID, 4)
                WHEN 0 THEN '500mg'
                WHEN 1 THEN '10mg'
                WHEN 2 THEN '850mg'
                ELSE '200mg'
            END
        ELSE NULL
    END AS Dose,

    CASE
        WHEN MOD(n3.i, 3) = 1 THEN
            CASE MOD(a.PatientID, 3)
                WHEN 0 THEN 'BID'
                WHEN 1 THEN 'Daily'
                ELSE 'PRN'
            END
        ELSE NULL
    END AS Frequency,

    a.AttendingPhysician AS OrderingProvider,

    CASE MOD(a.PatientID + n3.i, 10)
        WHEN 0 THEN 'Canceled'
        WHEN 1 THEN 'Ordered'
        ELSE 'Completed'
    END AS OrderStatus
FROM patient_Admit a
JOIN num3 n3;

-- ============================================================
--  POPULATE TABLES  (Populate Billing Table.sql)
-- ============================================================

SET SESSION cte_max_recursion_depth = 10000;
INSERT INTO patient_Billing
    (PatientID, RoomCharge, VisitCharge, RxCharge, LabCharge, ImagingCharge,
     TotalCharge, InsurancePaid, PatientPaid, BalanceDue)
SELECT
    a.PatientID,

    -- Room charge: days stayed * daily rate
    (
      GREATEST(
        1,
        DATEDIFF(COALESCE(a.DischargeDate, DATE_ADD(a.AdmitDate, INTERVAL 1 DAY)), a.AdmitDate)
      )
      * (350 + MOD(a.PatientID, 200))
    ) AS RoomCharge,

    -- Visit charge: flat-ish
    (200 + MOD(a.PatientID, 500)) AS VisitCharge,

    -- Rx/Lab/Imaging derived from orders (exclude canceled)
    COALESCE(SUM(CASE WHEN o.OrderStatus <> 'Canceled' AND o.OrderType = 'Medication'
                      THEN (10 + MOD(a.PatientID, 40)) ELSE 0 END), 0) AS RxCharge,

    COALESCE(SUM(CASE WHEN o.OrderStatus <> 'Canceled' AND o.OrderType = 'Lab'
                      THEN (25 + MOD(a.PatientID, 60)) ELSE 0 END), 0) AS LabCharge,

    COALESCE(SUM(CASE WHEN o.OrderStatus <> 'Canceled' AND o.OrderType = 'Imaging'
                      THEN (150 + MOD(a.PatientID, 200)) ELSE 0 END), 0) AS ImagingCharge,

    -- placeholders for totals (we'll update below)
    0, 0, 0, 0
FROM patient_Admit a
LEFT JOIN patient_Orders o
    ON o.PatientID = a.PatientID
GROUP BY a.PatientID;

-- ============================================================
--  POPULATE TABLES  (Populate Labs_tests.sql)
-- ============================================================

DROP TEMPORARY TABLE IF EXISTS num2;
CREATE TEMPORARY TABLE num2 (i INT PRIMARY KEY);
INSERT INTO num2 VALUES (1),(2);

INSERT INTO Labs (PatientID, LabDateTime, LabType, OrderingProvider, LabStatus)
SELECT
  a.PatientID,
  DATE_ADD(a.AdmitDate, INTERVAL (n2.i * 4 + MOD(a.PatientID, 6)) HOUR) AS LabDateTime,
  CASE MOD(a.PatientID + n2.i, 4)
    WHEN 0 THEN 'CBC'
    WHEN 1 THEN 'BMP'
    WHEN 2 THEN 'A1C'
    ELSE 'Lipid Panel'
  END AS LabType,
  a.AttendingPhysician,
  CASE MOD(a.PatientID + n2.i, 10)
    WHEN 0 THEN 'Canceled'
    WHEN 1 THEN 'Ordered'
    WHEN 2 THEN 'Collected'
    ELSE 'Completed'
  END AS LabStatus
FROM patient_Admit a
JOIN num2 n2;

INSERT INTO Tests (PatientID, TestDateTime, TestType, OrderingProvider, TestStatus)
SELECT
  a.PatientID,
  DATE_ADD(a.AdmitDate, INTERVAL (n2.i * 8 + MOD(a.PatientID, 6)) HOUR) AS TestDateTime,
  CASE MOD(a.PatientID + n2.i, 4)
    WHEN 0 THEN 'X-Ray Chest'
    WHEN 1 THEN 'CT Head'
    WHEN 2 THEN 'EKG'
    ELSE 'Ultrasound Abdomen'
  END AS TestType,
  a.AttendingPhysician,
  CASE MOD(a.PatientID + n2.i, 10)
    WHEN 0 THEN 'Canceled'
    WHEN 1 THEN 'Ordered'
    WHEN 2 THEN 'In Progress'
    ELSE 'Completed'
  END AS TestStatus
FROM patient_Admit a
JOIN num2 n2;

INSERT INTO Lab_Results
  (LabID, PatientID, ResultName, ResultValue, Unit, ReferenceRange, Flag)
SELECT
  l.LabID,
  l.PatientID,
  r.ResultName,
  r.ResultValue,
  r.Unit,
  r.ReferenceRange,
  r.Flag
FROM Labs l
JOIN (
  -- CBC
  SELECT 'CBC' AS LabType, 'WBC' AS ResultName,
         '7.2' AS ResultValue, 'K/uL' AS Unit, '4.0-11.0' AS ReferenceRange, 'Normal' AS Flag
  UNION ALL
  SELECT 'CBC', 'Hemoglobin', '13.8', 'g/dL', '12.0-17.5', 'Normal'
  UNION ALL
  SELECT 'CBC', 'Platelets', '250', 'K/uL', '150-450', 'Normal'

  -- BMP
  UNION ALL
  SELECT 'BMP', 'Glucose', '104', 'mg/dL', '70-110', 'Normal'
  UNION ALL
  SELECT 'BMP', 'Sodium', '139', 'mmol/L', '135-145', 'Normal'
  UNION ALL
  SELECT 'BMP', 'Creatinine', '1.0', 'mg/dL', '0.6-1.3', 'Normal'

  -- A1C
  UNION ALL
  SELECT 'A1C', 'Hemoglobin A1C', '5.8', '%', '4.0-5.6', 'High'

  -- Lipid Panel
  UNION ALL
  SELECT 'Lipid Panel', 'LDL', '120', 'mg/dL', '< 100', 'High'
  UNION ALL
  SELECT 'Lipid Panel', 'HDL', '48', 'mg/dL', '> 40', 'Normal'
  UNION ALL
  SELECT 'Lipid Panel', 'Triglycerides', '160', 'mg/dL', '< 150', 'High'
) r
  ON r.LabType = l.LabType
WHERE l.LabStatus <> 'Canceled';

INSERT INTO Test_Results (TestID, PatientID, Impression, Findings)
SELECT
  t.TestID,
  t.PatientID,
  CASE t.TestType
    WHEN 'X-Ray Chest' THEN 'No acute cardiopulmonary abnormality.'
    WHEN 'CT Head' THEN 'No acute intracranial hemorrhage.'
    WHEN 'EKG' THEN 'Normal sinus rhythm.'
    ELSE 'No acute abdominal abnormality.'
  END AS Impression,
  CASE t.TestType
    WHEN 'X-Ray Chest' THEN 'Lungs clear. No pleural effusion. Heart size normal.'
    WHEN 'CT Head' THEN 'Ventricles normal size. No mass effect. No midline shift.'
    WHEN 'EKG' THEN 'Rate 72 bpm. PR/QRS/QT within normal limits.'
    ELSE 'Liver, gallbladder, kidneys unremarkable. No free fluid.'
  END AS Findings
FROM Tests t
WHERE t.TestStatus <> 'Canceled';

-- ============================================================
--  CHECK TABLES  (Check Tables.sql)
-- ============================================================

UPDATE patient_Billing
SET TotalCharge = RoomCharge + VisitCharge + RxCharge + LabCharge + ImagingCharge;

UPDATE patient_Billing
SET
  InsurancePaid = ROUND(TotalCharge * 0.75, 2),
  PatientPaid   = ROUND(TotalCharge * 0.10, 2),
  BalanceDue    = ROUND(TotalCharge - (TotalCharge * 0.75) - (TotalCharge * 0.10), 2);

-- Reset auto-increment counters after bulk inserts (InnoDB over-allocates during INSERT...SELECT)
ALTER TABLE patient_Admit AUTO_INCREMENT = 1;
ALTER TABLE patient_Orders AUTO_INCREMENT = 1;
ALTER TABLE patient_Billing AUTO_INCREMENT = 1;
ALTER TABLE users AUTO_INCREMENT = 1;
ALTER TABLE Labs AUTO_INCREMENT = 1;
ALTER TABLE Lab_Results AUTO_INCREMENT = 1;
ALTER TABLE Tests AUTO_INCREMENT = 1;
ALTER TABLE Test_Results AUTO_INCREMENT = 1;

