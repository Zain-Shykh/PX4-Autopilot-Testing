# Part 1: Scope Record & Baseline Inventory (Member 3)

## 1. Scope Boundary Allocation
- **Primary Subsystems**:
  - `src/modules/flight_mode_manager/` (Flight Mode Manager setpoint generator, flight task arbiter, command dispatcher)
  - `src/lib/battery/` (Multi-cell battery status calculation, voltage-based & coulomb-counting SoC estimators, warning ladder, fault detection)
  - `src/modules/battery_status/` (Analog battery ADC scaling, voltage dividers, current measurement and channel selection)
- **Source Files Analyzed**:
  - `src/modules/flight_mode_manager/FlightModeManager.hpp` & `FlightModeManager.cpp`
  - `src/lib/battery/battery.h` & `battery.cpp`
  - `src/modules/battery_status/analog_battery.h` & `analog_battery.cpp`

## 2. Test Suites Implemented
- `src/modules/flight_mode_manager/FlightModeManagerTest.cpp`: 15 Functional GTest test cases
- `src/lib/battery/BatteryTest.cpp`: 14 Functional GTest test cases
- **Total Test Cases**: 29 / 29 Passing Deterministically
