# Work Distribution & Execution Blueprint: PX4-Autopilot Structural Testing (Assignment 02)

---

## 1. Executive Summary & Purpose

This document serves as the master work distribution blueprint and starting point for our 3-member team to complete **Assignment #02: Structural Testing and Coverage Analysis of PX4 Autopilot (v1.17.0)** within a **1-Day (24-Hour) Parallel Sprint**.

Because of the strict 1-day deadline, all work must be decoupled so that team members can work simultaneously on separate codebase modules and documentation sections without creating merge conflicts or blocking dependencies.

### Primary Objectives & Assignment Constraints
- **System Under Test (SUT)**: PX4-Autopilot Baseline Tag `v1.17.0` (Commit Hash: `d6f12ad1c4f70ad3230afd7d86e971421e02fef4`).
- **Equal Work Split**: Work is divided into 3 equal tracks with clear individual module ownership and shared integration responsibilities.
- **Coverage Target**: 100% Statement Coverage & 100% Decision/Branch Coverage on selected business/control logic, alongside a rigorous **MC/DC (Modified Condition/Decision Coverage)** derivation on a designated critical component.
- **Deliverables**:
  1. Detailed Comprehensive Technical Report (`Report.md` / PDF export).
  2. Testing Workbook (`Testing_Workbook.xlsx` - **strictly max 2 sheets**: Sheet 1 `Test Inventory`, Sheet 2 `MC/DC Evidence`).
  3. Student-Authored Test C++ Source Files & CMake Modifications.
  4. Git Patch File (`px4_assignment2_tests.patch` against `v1.17.0`).
  5. Baseline and Final `lcov` / `gcov` Coverage Reports (HTML + Machine-Readable).
  6. AI Assistance Record (identifying material AI usage and assumptions introduced).

---

## 2. PX4-Autopilot Quickstart & Environment Setup Guide

This section provides complete, step-by-step instructions for team members who have just cloned the repository and need to build, test, and extract coverage.

### 2.1 Repository Setup & Verification
Run the following commands in your terminal to verify your local workspace is locked to the required baseline:

```bash
# Navigate to repository root
cd /home/zain-shykh/Desktop/SQE_ASSIGNMENTS/assign_2/PX4-Autopilot

# Verify tag and exact commit hash
git status
git rev-parse HEAD
# Expected Output: d6f12ad1c4f70ad3230afd7d86e971421e02fef4
```

### 2.2 System Prerequisites & Toolchain Setup (Linux / WSL2 Ubuntu 22.04/24.04)
Ensure all required build tools, compilers, and coverage tools are installed:

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake ninja-build g++ gcov lcov \
    python3 python3-pip python3-setuptools git
```

### 2.3 Building PX4 SITL & Running Baseline Unit Tests

#### Step A: Build PX4 SITL Default Target
```bash
# Build PX4 Software-In-The-Loop (SITL) firmware
make px4_sitl_default
```

#### Step B: Build and Execute Existing Baseline Unit Tests
```bash
# Build and run unit test executable suite
make tests
```
*Note: Test executables and output binaries are generated under `build/px4_sitl_test/`.*

#### Step C: Running Specific Tests using `TESTFILTER`
To run a specific test target without running the entire suite:
```bash
make tests TESTFILTER=<TestClassName>
# Example: make tests TESTFILTER=AttitudeEstimatorQ
```

### 2.4 Capturing Structural Coverage (`gcov` / `lcov`)

#### Step A: Run Baseline Coverage Collection
```bash
# Clean previous builds and run tests in Coverage mode
make tests_coverage
```
This target performs the following operations automatically:
1. Cleans existing build directories.
2. Compiles with `PX4_CMAKE_BUILD_TYPE=Coverage` (adds `-fprofile-arcs -ftest-coverage` flags).
3. Executes the test suite.
4. Generates an `lcov` tracefile at `coverage/lcov.info`.

#### Step B: Generating HTML Coverage Reports
To view coverage in an interactive web browser interface:
```bash
# Generate HTML report from lcov.info
genhtml coverage/lcov.info --output-directory coverage/html_report

# Open in browser (or serve locally)
xdg-open coverage/html_report/index.html
```

---

## 3. High-Level Work Division & Architecture Overview

The work is split into **Shared/Collaborative Responsibilities** (where all 3 members contribute) and **Individual Ownership Tracks** (where each member owns a dedicated PX4 C++ module, test suite, and report section).

```
+-----------------------------------------------------------------------------------+
|                            SHARED COLLABORATIVE PHASES                            |
|  - Phase 1: Environment Setup & Baseline Coverage Capture                         |
|  - Phase 4: Final Quality Judgment, Report Consolidation, Patch & Submission      |
+--------------------------+--------------------------+-----------------------------+
                           |                          |
                           v                          v
+--------------------------+  +-----------------------+  +--------------------------+
|    MEMBER 1 TRACK        |  |    MEMBER 2 TRACK     |  |    MEMBER 3 TRACK        |
| Estimation & Mathematics |  | Safety & MC/DC Lead   |  | Flight Mode & Controls   |
+--------------------------+  +-----------------------+  +--------------------------+
| - Scope 1:               |  | - Scope 2 (MC/DC):    |  | - Scope 3:               |
|   attitude_estimator_q   |  |   land_detector       |  |   flight_mode_manager    |
|   & hysteresis lib       |  |   (Multicopter Land)  |  |   & battery_status       |
| - GTest Unit Suite       |  | - MC/DC Truth Matrix  |  | - GTest Functional Suite |
| - Sheet 1 Workbook Lead  |  | - Sheet 2 MC/DC Lead  |  | - Coverage Pipeline Lead |
+--------------------------+  +-----------------------+  +--------------------------+
```

---

## 4. Member-Specific Roles, Scope & Detailed Responsibilities

### 👤 MEMBER 1: System Integrator & Estimation/State Logic Lead
*(Evaluated against Rubric: Part 1 - Repository Analysis [20 Marks, CLO3] & Part 3A - Implementation [15 Marks, CLO2])*

#### Primary Scope & Production Code Allocation
1. **Target Area 1A**: `src/modules/attitude_estimator_q/` (`AttitudeEstimatorQ` C++ class)
   - *Responsibility*: Attitude estimation using quaternions, complementary filter gains, gyro bias correction, and sensor fusion logic.
   - *Test Level*: GTest Unit Test (`src/modules/attitude_estimator_q/AttitudeEstimatorQTest.cpp`).
2. **Target Area 1B**: `src/lib/hysteresis/` (`Hysteresis` template class)
   - *Responsibility*: State debouncing and hysteresis logic for mode transitions, optical flow checks, and sensor valid flags.
   - *Test Level*: GTest Unit Test.

#### Task List & Step-by-Step Instructions
- [ ] **Part 1 Setup**: Record Git hash (`d6f12ad1c4f70ad3230afd7d86e971421e02fef4`), OS, compiler/toolchain details, and exact build/test commands in report section 1.
- [ ] **Scope Analysis**: Analyze `AttitudeEstimatorQ` and `Hysteresis` decision logic; document state variables, inputs, parameters, and decision paths.
- [ ] **Test Derivation**: Design test cases for 100% Statement and Decision/Branch coverage for `attitude_estimator_q` and `hysteresis`. Include boundary cases (zero division, quaternion normalization limits, edge state transitions).
- [ ] **C++ Implementation**: Implement student-authored GTest unit tests under `src/modules/attitude_estimator_q/` and `src/lib/hysteresis/`. Update relevant `CMakeLists.txt` for test registration.
- [ ] **Workbook Leadership**: Responsible for managing and integrating **Sheet 1 (Test Inventory)** of `Testing_Workbook.xlsx` (Test ID, component/function, purpose/scenario, key controlled input/state, expected result, execution result, coverage target, test-file reference).

---

### 👤 MEMBER 2: Lead Structural Analyst & MC/DC Specialist
*(Evaluated against Rubric: Part 2 - Structural Test Derivation & MC/DC [30 Marks, CLO2] & Part 4 - Quality Judgment [15 Marks, CLO3])*

#### Primary Scope & Production Code Allocation
1. **Target Area 2 (MC/DC Designated Critical Component)**: `src/modules/land_detector/` (`LandDetector` & `MulticopterLandDetector`)
   - *Responsibility*: Flight-critical land detection, ground contact, freefall detection, maybe-landed states, and thrust/velocity threshold checks.
   - *Critical Justification*: Land detection directly governs motor disarm, failsafe triggers, and flight phase switching. Incorrect land state can cause catastrophic drone crashes or motor runaway.
   - *Test Level*: GTest Unit / Functional Test (`src/modules/land_detector/LandDetectorTest.cpp`).

#### Task List & Step-by-Step Instructions
- [ ] **Part 2 Lead (MC/DC Derivation)**:
  - Extract all non-trivial compound decisions from `MulticopterLandDetector::detect_land()` and related condition checks (e.g., altitude rate, acceleration, thrust vector, time hysteresis).
  - Define atomic Boolean conditions ($A, B, C, D, \dots$) for each compound decision.
  - Construct MC/DC Independence Pair matrices demonstrating each atomic condition's independent effect on the final decision outcome.
  - **Condition Value Rule**: Ensure test set demonstrates every atomic condition taking both True and False values AND overall decision taking both True and False outcomes.
- [ ] **Sheet 2 Workbook Leadership**: Build and format **Sheet 2 (MC/DC Evidence)** in `Testing_Workbook.xlsx` with complete Boolean truth tables, independence pairs, demonstrated conditions, and test references. (Strictly no extra sheets permitted!).
- [ ] **C++ Test Suite**: Write comprehensive GTest suite for `LandDetector` reaching 100% branch/statement coverage and executing all MC/DC test condition pairs.
- [ ] **Part 4 Quality Judgment Lead**: Draft the 300–400 word Final Quality Judgment section summarizing structural test coverage confidence and residual risk without overclaiming whole-repository quality.

---

### 👤 MEMBER 3: Environment, Flight Control & Coverage Pipeline Lead
*(Evaluated against Rubric: Part 3B - Coverage Measurement & Gap Analysis [20 Marks, CLO2/CLO3])*

#### Primary Scope & Production Code Allocation
1. **Target Area 3A**: `src/modules/flight_mode_manager/` (`FlightModeManager` & Tasks)
   - *Responsibility*: Flight mode switching logic (Manual, Altitude, Position, Auto RTL, Land, Mission), mode fallbacks, and arming checks.
   - *Test Level*: GTest Functional Test with uORB state mocks.
2. **Target Area 3B**: `src/modules/battery_status/` (`BatteryStatus`)
   - *Responsibility*: Battery voltage/current filtering, state-of-charge calculation, low battery warning levels, and emergency failsafe triggers.
   - *Test Level*: GTest Unit / Functional Test.

#### Task List & Step-by-Step Instructions
- [ ] **Environment & Build Pipeline Lead**: Standardize build instructions, verify `lcov` coverage script execution, and capture baseline vs final coverage HTML/LCOV outputs.
- [ ] **Scope Analysis & Test Design**: Document decision logic, uORB dependencies, and parameters for `FlightModeManager` and `BatteryStatus`. Derive tests targeting 100% statement/decision coverage.
- [ ] **C++ Implementation**: Implement GTest functional/unit tests under `src/modules/flight_mode_manager/` and `src/modules/battery_status/`.
- [ ] **Part 4 Coverage Gap & Defect Analysis Lead**:
  - Run final `lcov` report and perform line-by-line gap analysis.
  - Document all uncovered statements/branches with technical justifications (e.g., defensive assertions, uninstantiated templates, hardware platform guards). "Hardware dependent" alone is NOT acceptable justification.
  - Record any confirmed defects/blockers found during testing (affected source location, reproduction conditions, expected vs actual result).

---

## 5. Phase-by-Phase 1-Day (24-Hour) Execution Timeline

To complete the assignment within 1 day, follow this hourly milestone plan:

| Time Slot | Phase | Focus & Milestones | Shared vs Individual |
| --- | --- | --- | --- |
| **Hours 00:00 - 04:00** | **Phase 1: Setup & Baseline Freeze** | - All members clone repo, verify commit hash `d6f12ad1c4f70ad...`<br>- Build SITL and run `make tests`<br>- Member 3 captures baseline coverage.<br>- All members write their individual `work_plan.md`. | **Shared Setup**, then Individual Plan Creation |
| **Hours 04:00 - 10:00** | **Phase 2: Code Analysis & MC/DC Proofs** | - Member 1 analyzes `AttitudeEstimatorQ` & `Hysteresis`.<br>- Member 2 analyzes `LandDetector` decision tree & derives MC/DC Boolean matrices.<br>- Member 3 analyzes `FlightModeManager` & `BatteryStatus`.<br>- Populate draft rows in `Testing_Workbook.xlsx`. | **100% Parallel Individual Work** |
| **Hours 10:00 - 18:00** | **Phase 3: C++ Test Coding & Coverage Iteration** | - Members write C++ GTest code for their respective modules.<br>- Register tests in `CMakeLists.txt` & run `make tests TESTFILTER=...`.<br>- Member 3 runs full `make tests_coverage` to measure achieved branch/statement coverage.<br>- Refine tests to reach target coverage. | **Parallel Coding**, Shared Coverage Measurement at Hr 17 |
| **Hours 18:00 - 24:00** | **Phase 4: Consolidation, Defects & Submission** | - Member 1 consolidates Sheet 1 & creates Git Patch file.<br>- Member 2 completes Sheet 2 & Final Quality Judgment (300-400 words).<br>- Member 3 compiles Coverage Gap section & AI Assistance Record.<br>- Merge sections into final `Report.md` & verify LMS checklist. | **Shared Consolidation & Verification** |

---

## 6. Comprehensive Task Division Matrix

| Task ID | Task Description | Assigned Member | Scope / Artifact | Output Deliverable | Est. Hours |
| --- | --- | --- | --- | --- | --- |
| `TSK-01` | Git Baseline & Local Build Verification | **Member 1** (Lead) | PX4 Core Repo | Report Sec 1 (Build Env & Commit Hash) | 1.0 h |
| `TSK-02` | Baseline Coverage Run & HTML Extraction | **Member 3** (Lead) | `coverage/lcov.info` | Baseline LCOV Data & Screenshots | 1.5 h |
| `TSK-03` | Member 1 Individual `work_plan_member1.md` | **Member 1** | Root Directory | `work_plan_member1.md` | 1.0 h |
| `TSK-04` | Member 2 Individual `work_plan_member2.md` | **Member 2** | Root Directory | `work_plan_member2.md` | 1.0 h |
| `TSK-05` | Member 3 Individual `work_plan_member3.md` | **Member 3** | Root Directory | `work_plan_member3.md` | 1.0 h |
| `TSK-06` | Scope Analysis & Decision Tree Extraction (Scope 1) | **Member 1** | `attitude_estimator_q`, `hysteresis` | Report Part 1.1 | 2.5 h |
| `TSK-07` | Scope Analysis & MC/DC Compound Decision Mapping | **Member 2** | `land_detector` | Report Part 1.2 & Part 2 MC/DC | 3.5 h |
| `TSK-08` | Scope Analysis & uORB State Mapping (Scope 3) | **Member 3** | `flight_mode_manager`, `battery_status` | Report Part 1.3 | 2.5 h |
| `TSK-09` | MC/DC Independence Pair Derivation & Matrix Construction | **Member 2** | `LandDetector` decisions | Workbook Sheet 2 (`MC/DC Evidence`) | 3.0 h |
| `TSK-10` | C++ GTest Suite Implementation (Scope 1) | **Member 1** | `AttitudeEstimatorQTest.cpp`, `HysteresisTest.cpp` | Compiling GTest Source Code | 4.0 h |
| `TSK-11` | C++ GTest Suite Implementation (Scope 2 MC/DC) | **Member 2** | `LandDetectorTest.cpp` | Compiling GTest Source Code | 4.5 h |
| `TSK-12` | C++ GTest Suite Implementation (Scope 3) | **Member 3** | `FlightModeManagerTest.cpp`, `BatteryStatusTest.cpp` | Compiling GTest Source Code | 4.0 h |
| `TSK-13` | Test Inventory Consolidation | **Member 1** | All Test Suites | Workbook Sheet 1 (`Test Inventory`) | 2.0 h |
| `TSK-14` | Final Coverage Measurement & Line/Branch Gap Analysis | **Member 3** | `coverage/lcov.info` | Report Part 3 (Coverage & Gap Analysis) | 2.5 h |
| `TSK-15` | Defect Investigation & Final Quality Judgment (300-400w) | **Member 2** | Scope Findings | Report Part 4 (Quality Judgment) | 2.0 h |
| `TSK-16` | Git Patch Generation (`.patch`) & AI Record | **Member 1** | `git diff v1.17.0` | `px4_assignment2_tests.patch` & AI Record | 1.0 h |
| `TSK-17` | Master Submission Bundle Verification | **All Members** | LMS Checklist | Final Zip / Folder Bundle | 1.0 h |

---

## 7. Guidelines for Members to Create Individual `work_plan.md` Files

Each team member **MUST** create their own detailed `work_plan.md` file in the repository root based on this distribution document:
- **Member 1**: `work_plan_member1.md`
- **Member 2**: `work_plan_member2.md`
- **Member 3**: `work_plan_member3.md`

### Required Structure for Individual Work Plan Files
Each member's `work_plan.md` must include:
1. **Personal Goal & Scope Assignment**: Stating assigned modules, test levels, and expected deliverables.
2. **Granular Hourly Schedule**: Hour-by-hour breakdown for the 24-hour sprint.
3. **Specific File Paths & Function Targets**: Exact C++ source files, classes, methods, and decision points to be analyzed and tested.
4. **Target Test Cases Table**: Draft list of test case IDs (e.g., `TC_M1_01`, `TC_M1_02`), target methods, inputs, and expected outcomes.
5. **Personal Verification Commands**: Terminal commands to compile and execute their specific test suite.

---

## 8. Final Submission Checklist, Strict Formatting Rules & Viva Guide

### 8.1 File Naming & Formatting Constraints
- All submitted files **must** follow the strict naming convention: `<Rollnumber1_Rollnumber2_Rollnumber3_Section>.<extension>` (unless specified otherwise by the LMS).
- The testing workbook (`.xlsx`) **must contain strictly no more than 2 sheets**:
  - Sheet 1: `Test Inventory`
  - Sheet 2: `MC/DC Evidence`
  - *(Do NOT create separate scope, coverage gap, or defect sheets; explain gaps and defects in prose in the report).*
- **No Manufactured Failures**: Execution statuses (`PASS`, `FAIL`, `BLOCKED`) must accurately reflect execution outcomes. Do not manufacture artificial failures.

### 8.2 Deliverables Checklist
- [ ] **Concise Technical Report** (`Report.md` or PDF): Containing Group details, Environment specs, Commit hash `d6f12ad1c4f70ad3230afd7d86e971421e02fef4`, Scope justification, MC/DC derivation, Coverage analysis, Remaining gaps, and 300–400 word Final Quality Judgment.
- [ ] **Student-Authored Test Files**: Located in proper `src/` module directories with modified `CMakeLists.txt`.
- [ ] **Git Patch File**: `px4_assignment2_tests.patch` produced via `git diff v1.17.0 > px4_assignment2_tests.patch`.
- [ ] **Testing Workbook (`Testing_Workbook.xlsx`)**: Exactly 2 sheets (`Test Inventory` & `MC/DC Evidence`).
- [ ] **Coverage Evidence**: `lcov` HTML report folder & summary files.
- [ ] **Brief AI-Assistance Record**: Documenting material AI use (navigation, build troubleshooting, test scaffolding) and any key assumptions introduced.

### 8.3 Viva & Oral Defense Preparation Guide
During the viva, any member may be individually questioned on any part of the project. Ensure every team member can answer the following:
1. **Baseline & Commands**: How do you build PX4 SITL and run only your student-authored test from the command line?
2. **Source-to-Test Mapping**: Pick any test case in Sheet 1 and point to the exact lines of C++ code in `src/modules/...` that it exercises.
3. **MC/DC Explanation**: Explain an MC/DC independence pair from Sheet 2. How does changing condition $A$ while holding $B, C$ constant flip the overall decision result?
4. **Test Level Justification**: Why did you choose GTest Unit vs GTest Functional vs SITL for your target module?
5. **Coverage Gap Justification**: Point to an uncovered line in the HTML coverage report and explain why it is unreachable in local execution.
