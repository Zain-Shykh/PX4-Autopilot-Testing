# Member 3 Deliverables — Scope 3 (FlightModeManager & Battery)

**Assigned Scope**: 
- src/modules/flight_mode_manager/ (FlightModeManager)
- src/lib/battery/ and src/modules/battery_status/ (Battery & AnalogBattery)

## Deliverables Summary
1. work_plan_member3.md: Individual task plan and timeline.
2. Report.md: Consolidated technical report.
3. Testing_Workbook.xlsx: 2-sheet Excel testing workbook (Test Inventory & MC-DC Evidence).
4. sheet1_test_inventory.csv: CSV export of 23 functional test cases.
5. sheet2_mcdc_evidence.csv: CSV export of DO-178C MC/DC truth table and independence pairs.
6. member3_tests.patch: Standalone patch for Scope 3 test suites and CMake integrations.
7. source_files/: Student-authored test files and header modifications.
8. coverage/: LCOV tracefiles and interactive HTML coverage reports.
9. logs/: Verbose execution logs.

## Quick Test & Coverage Execution
Test project /home/hamza_atif/SQE_Assignment2/PX4-Autopilot-Testing/build/px4_sitl_test
    Start   4: functional-Battery
1/2 Test   #4: functional-Battery ...............   Passed    0.16 sec
    Start 106: functional-FlightModeManager
2/2 Test #106: functional-FlightModeManager .....   Passed    0.10 sec

100% tests passed, 0 tests failed out of 2

Total Test time (real) =   0.39 sec
Capturing coverage data from build/px4_sitl_test
geninfo cmd: '/usr/bin/geninfo build/px4_sitl_test --output-filename coverage/lcov_raw.info --ignore-errors mismatch --ignore-errors empty --memory 0'
Found gcov version: 13.3.0
Using intermediate gcov format
