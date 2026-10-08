# Part 1: Scope 3 Architectural Deep-Dive & Test Basis

### 2.2 Scope 3 Deep-Dive Architectural & Behavioral Analysis

#### 2.2.1 `FlightModeManager` (`src/modules/flight_mode_manager/`)
`FlightModeManager` acts as the master operational switchboard for autonomous and manual flight modes. It inherits from `ModuleBase`, `px4::WorkItemScheduled`, and `ModuleParams`.

- **Core Responsibilities**:
  1. Synchronously ingests vehicle status (`vehicle_status_s`), control mode flags (`vehicle_control_mode_s`), landing states (`vehicle_land_detected_s`), and commands (`vehicle_command_s`).
  2. Evaluates requested flight mode transitions against arming status, failsafe flags, navigation capabilities, and flight task availability.
  3. Manages lifetime, switching, activation, and error fallback of dynamic `FlightTask` objects.
  4. Enforces vehicle command freshness constraints (200 ms timeout window).

- **Critical State Dependencies & Compound Decisions**:
  - `isNavStateSwitchingAllowed()`: Validates that transitions between manual, offboard, mission, and failsafe states only occur if the required sensor estimators (e.g. valid position, altitude) are healthy.
  - `start_flight_task()`: Evaluates compound conditions mapping navigation states to task availability without active failsafes.
  - `tryApplyCommandIfAny()`: Enforces real-time command freshness (age <= 200 ms and valid vehicle command).

- **Test Level Justification**:
  - *GTest Unit*: Inadequate because `FlightModeManager` relies heavily on uORB topic subscriptions and parameter tree lookups.
  - *SITL End-to-End*: Excessive execution time (minutes per scenario), non-deterministic physics timing, and inability to isolate branch conditions.
  - *GTest Functional (Selected)*: Optimal. Executes compiled C++ firmware logic natively, utilizes PX4 in-memory uORB pub/sub simulator, and achieves deterministic, sub-millisecond setup-run-check execution without external simulation dependencies.

---

#### 2.2.2 `Battery` & `AnalogBattery` (`src/lib/battery/` & `src/modules/battery_status/`)
The `Battery` library and `AnalogBattery` driver provide vital energy monitoring, dynamic state-of-charge (SoC) estimation, internal resistance tracking, and hierarchical warning generation.

- **Core Responsibilities**:
  1. Computes multi-cell battery pack voltage, individual cell voltages, and instant/integrated current draw.
  2. Implements a Recursive Least Squares (RLS) adaptive estimator to calculate internal cell resistance and load-drop-corrected Open Circuit Voltage (OCV).
  3. Evaluates battery capacity, remaining percentage, and remaining flight time.
  4. Manages strict, non-oscillating battery warning state transitions: Normal -> Low (Warning) -> Critical (Failsafe Return) -> Emergency (Immediate Land).

- **Critical State Dependencies & Compound Decisions**:
  - `updateBatteryStatus()` warning threshold compound decision:
    Requires `_connected && _battery_initialized` before calling `determineWarning()`, which compares state of charge with strict emergency, critical, and low thresholds. There is no compound voltage/SoC warning decision in this source.
  - Hysteresis & Filtering: Low-pass filtering on voltage and current to prevent momentary motor throttle bursts from triggering false emergency failsafes.

---

