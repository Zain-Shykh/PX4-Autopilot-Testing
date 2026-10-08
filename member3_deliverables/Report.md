# Scope 3 Technical Verification Report: Flight Mode Manager & Battery Management Subsystems

**Course**: Software Quality Engineering (CS-4001 / SE-4001)  
**Institution**: National University of Computer and Emerging Sciences (NUCES-FAST), Islamabad  
**Department**: Department of Software Engineering  
**Baseline Commit**: `d6f12ad1c4f70ad3230afd7d86e971421e02fef4` (PX4-Autopilot `v1.17.0`)  
**Target Architecture**: POSIX SITL (`px4_sitl_default` / `px4_sitl_test`)  
**Scope Allocation**: Member 3 (`src/modules/flight_mode_manager/`, `src/lib/battery/`, `src/modules/battery_status/`)  
**Test Suite Execution**: **29 / 29 Tests Passed (100%)**  

---

## Executive Summary & Deliverables Index

This report presents a rigorous, structural software quality evaluation and automated unit/functional test suite for the mission-critical **Flight Mode Manager (FMM)** and **Battery Management Subsystems** of the PX4 Autopilot platform.

### Scope 3 Deliverables Checklist
1. **Testing Workbook**: `Testing_Workbook.xlsx` (Contains *Test Inventory* with 29 test cases and *MC-DC Evidence* decision mapping).
2. **Student-Authored C++ Test Suites**:
   - `src/modules/flight_mode_manager/FlightModeManagerTest.cpp` (15 Functional GTest cases)
   - `src/lib/battery/BatteryTest.cpp` (14 Functional GTest cases)
3. **Build System Registration**:
   - `src/modules/flight_mode_manager/CMakeLists.txt` (Registered functional GTest target `functional-FlightModeManager`)
   - `src/lib/battery/CMakeLists.txt` (Registered functional GTest target `functional-Battery`)
4. **Coverage Artifacts**:
   - Baseline Coverage: `coverage/baseline/` (0.0% line coverage capture)
   - Final Achieved Coverage: `coverage/final/` (**89.8% Statement/Line** [465/518 lines], **88.6% Function** [39/44 functions])
   - `coverage/summary.json` & `coverage/uncovered_obligations.csv`
5. **Execution Logs & Patch**:
   - `logs/ctest_execution_log.txt` (Deterministic 29/29 pass log)
   - `member3_tests.patch` (Consolidated patch against release baseline)

---

## 1. System Environment & Execution Setup Guide

### 1.1 Host & Toolchain Specifications
- **Operating System**: Ubuntu Linux (x86_64) / WSL2
- **Compiler**: GCC / G++ 13.3.0
- **Build Engine**: CMake 3.28+ & Ninja
- **Testing Framework**: GoogleTest (GTest) 1.14.0 integrated via PX4 Functional Test Runner
- **Coverage Engine**: `gcov` 13.3.0 & `lcov` 2.0+ (flags: `-fprofile-arcs -ftest-coverage -O0 -g`)
- **Python Environment**: Python 3.12 (with `openpyxl`)

### 1.2 Reproduction Commands
```bash
# Build test binaries
ninja -C build/px4_sitl_test functional-Battery functional-FlightModeManager

# Execute test suite via CTest
ctest --test-dir build/px4_sitl_test -R 'Battery|FlightModeManager' --output-on-failure -V
```

---

## 2. Test Architecture & Design Decisions

### 2.1 Flight Mode Manager Subsystem (`src/modules/flight_mode_manager/`)
`FlightModeManager` inherits from `ModuleBase<FlightModeManager>`, `px4::WorkItem`, and `ModuleParams`. It serves as the central setpoint generation arbiter, activating specialized flight tasks based on vehicle status, navigation states, armed state, and vehicle commands.

#### Test Fixture Design & Introspection Helpers
- **Fixture Class**: `TestFlightModeManager` derives from `FlightModeManager` and utilizes the declared friend relationship (`friend class TestFlightModeManager;`) to access internal state and private methods (`start_flight_task()`, `handleCommand()`, `switchTask()`, `Run()`).
- **`updateSubscriptions()` Helper**: Clearly documented as a test fixture introspection helper that polls test-published uORB topics (`_vehicle_control_mode_sub`, `_vehicle_land_detected_sub`, `_vehicle_status_sub`) into local subscription buffers before asserting state transitions.
- **Command Age Boundary**: Validates strict predicate `hrt_absolute_time() < cmd.timestamp + 200_ms`. An expired command is rejected and remains in the buffer until overwritten or explicitly cleared.

### 2.2 Battery Management Subsystem (`src/lib/battery/` & `src/modules/battery_status/`)
The battery subsystem computes state of charge (SoC), remaining flight time, overvoltage/undervoltage faults, and cell voltage scaling using dual complementary estimation:
1. **Configured Internal Resistance & Load Drop**: When `_params.r_internal >= 0.0f`, the static load-drop formula $V_{\text{ocv}} = V + I \cdot R_{\text{internal}}$ is evaluated directly.
2. **Recursive Least Squares (RLS) Estimator**: When `_params.r_internal < 0.0f`, the experimental 2-state RLS filter tracks internal resistance and open-circuit voltage dynamically under varying current loads.
3. **Coulomb Counting & Integration Cap**: `sumDischarged()` caps elapsed $\Delta t$ at `2.0s` to prevent spurious integration jumps during scheduler delays or simulation pauses.

---

## 3. Structural Coverage Results & Line-by-Line Gap Investigation

### 3.1 Coverage Summary
| Source File | Executable Lines | Lines Covered | Statement Coverage | Functions Covered | Function Coverage |
| :--- | :---: | :---: | :---: | :---: | :---: |
| `src/lib/battery/battery.cpp` | 231 | 228 | **98.7%** | 19 / 19 | **100.0%** |
| `src/modules/battery_status/analog_battery.cpp` | 54 | 45 | **83.3%** | 6 / 7 | **85.7%** |
| `src/modules/flight_mode_manager/FlightModeManager.cpp` | 233 | 192 | **82.4%** | 14 / 18 | **77.8%** |
| **Total Scope 3 Subsystem** | **518** | **465** | **89.8%** | **39 / 44** | **88.6%** |

### 3.2 Concrete Line-by-Line Coverage Gap Investigation (Problem 10)
Rather than asserting broad "hardware-only" claims, the remaining unreached lines are investigated directly against the concrete source:

1. **`src/lib/battery/battery.cpp` (Lines 64–66)**:
   - *Construct*: Out-of-bounds battery index validation (`index > 9 || index < 1`).
   - *Feasibility*: Defensive guard. PX4 multi-battery instances are indexed 1..9. The unit tests instantiate standard primary battery instance 1.
2. **`src/modules/battery_status/analog_battery.cpp` (Lines 115–121)**:
   - *Construct*: `BOARD_BATTERY_ADC_VOLTAGE_FILTER_S` and `BOARD_BATTERY_ADC_CURRENT_FILTER_S` preprocessor blocks.
   - *Feasibility*: These alpha-filter time constants are defined only in target board headers for physical FMU hardware (e.g. `px4_fmu-v5`) and are omitted under POSIX SITL compilation.
3. **`src/modules/battery_status/analog_battery.cpp` (Lines 140–145)**:
   - *Construct*: `BOARD_BRICK_VALID_LIST` array lookup in `is_valid()`.
   - *Feasibility*: Board brick validation list macro is hardware-target specific; defaults to `true` under POSIX.
4. **`src/modules/flight_mode_manager/FlightModeManager.cpp` (Lines 38–70)**:
   - *Construct*: `task_spawn()` and `instantiate()` static trampolines.
   - *Feasibility*: Used by NuttX/POSIX shell CLI to spawn FlightModeManager as a detached OS task thread (`px4_task_spawn_cmd`). In-process unit test harness instantiates the C++ class directly.
5. **`src/modules/flight_mode_manager/FlightModeManager.cpp` (Lines 180–188)**:
   - *Construct*: `!defined(CONSTRAINED_FLASH)` conditional checks and dynamic task allocation error traps.
   - *Feasibility*: Memory allocation on POSIX SITL always succeeds; exercising the task allocation failure branch requires mock allocator fault injection.

---

## 4. Final Quality Judgment (Supported Group Evaluation)

The structural testing and verification activities performed on PX4 Autopilot's Flight Mode Manager and Battery Management Subsystems demonstrate high structural integrity and predictable deterministic behavior under the evaluated execution envelopes. Achieving an overall statement coverage of **89.8% (465/518 lines)** and function coverage of **88.6% (39/44 functions)** across Scope 3 establishes strong confidence in primary operational pathways, mode-switch state machines, ADC conversion mathematics, and fault escalations. 

Critically, isolating test fixture subscriptions and eliminating un-reset parameter state guarantees test independence and deterministic pass rates across shuffled runs. The remaining coverage gaps have been thoroughly investigated and attributed to compiler-gated hardware macros, defensive index guards, and OS daemon startup wrappers rather than unverified flight logic. 

However, full flight readiness certification cannot rely solely on unit-level isolation. Several architectural limitations require ongoing verification:
1. **Asynchronous Scheduling Dynamics**: Unit tests execute task switches synchronously; real-time interaction with the PX4 work queue (`wq:nav_and_controllers`) under severe thread preemption must be continuously validated in hardware-in-the-loop (HITL) environments.
2. **Sensor Noise & ADC Degradation**: While static ADC scaling and standard load drops are verified, degraded sensor bus packets, ADC quantization drift, and battery cell internal degradation require extended statistical testing under real flight loads.

In conclusion, the Scope 3 subsystems meet rigorous software quality standards for the SITL operational profile, with verified fault-handling fallbacks and robust state transitions.
