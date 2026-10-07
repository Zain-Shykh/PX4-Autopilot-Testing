# Part 1 — Repository Analysis & Structural Scope Record
## Scope Item 2: `src/modules/land_detector/` (`MulticopterLandDetector`)

---

## 1. Production Scope Identification

- **Component Name**: `MulticopterLandDetector` (inheriting from base class `LandDetector`)
- **Primary Source Files**:
  - `src/modules/land_detector/MulticopterLandDetector.h`
  - `src/modules/land_detector/MulticopterLandDetector.cpp` (Lines 159–305)
- **PX4 Test Level Chosen**: GTest Functional Test (`src/modules/land_detector/LandDetectorTest.cpp`)
- **Evaluated Methods**:
  - `_get_ground_contact_state()` (Lines 159–252)
  - `_get_maybe_landed_state()` (Lines 254–291)
  - `_get_ground_effect_state()` (Lines 299–304)
  - `_get_freefall_state()` (Lines 153–157)
  - `_get_landed_state()` (Lines 293–297)

---

## 2. Component Responsibility & Critical Behavior

`MulticopterLandDetector` is a flight-critical state machine that continuously monitors sensor data, velocity vectors, acceleration norms, and commanded thrust setpoints to determine the physical landing phase of a multicopter drone.

### Key Responsibilities
1. **Ground Contact Detection (`_get_ground_contact_state`)**: Evaluates whether thrust setpoint is low, horizontal velocity is near zero, and vertical speed is below threshold while descending.
2. **Maybe-Landed Detection (`_get_maybe_landed_state`)**: Evaluates whether ground contact has been maintained, angular rotation is below maximum threshold, and thrust setpoint is at minimum.
3. **Landed State Finalization (`_get_landed_state`)**: Determines when maybe-landed hysteresis triggers final landed state, which signals position control to disarm motors.
4. **Freefall Detection (`_get_freefall_state`)**: Detects micro-gravity acceleration conditions ($\|a\| < 2.0\text{ m/s}^2$).
5. **Ground Effect Height Check (`_get_ground_effect_state`)**: Detects aerodynamic cushion effects when operating close to solid ground.

---

## 3. Why Included & Non-Triviality Justification

- **Non-Trivial Business Logic**: Unlike simple getters, setters, or wrappers, `MulticopterLandDetector` contains complex multi-condition Boolean decisions, time debouncing hysteresis state machines, and mathematical norm comparisons.
- **Flight-Criticality**: Erroneous land detection during active flight causes immediate motor shutdown, resulting in catastrophic drone crashes. Failure to detect landing upon touchdown prevents motor disarming, leading to drone tip-over or motor overheating.
- **Exclusion Compliance**: Pure GUI presentation code, visual rendering, interface wrappers, and generated uORB topics are strictly excluded from this scope.

---

## 4. Key Dependencies, State & Control Inputs

- **uORB Message Dependencies**: `vehicle_local_position`, `vehicle_thrust_setpoint`, `vehicle_control_mode`, `takeoff_status`.
- **Parameters**: `LNDMC_Z_VEL_MAX`, `LNDMC_XY_VEL_MAX`, `LNDMC_ROT_MAX`, `LNDMC_ALT_GND`.
- **Controlled Inputs**: Armed status (`_armed`), distance to ground (`dist_bottom`), angular rate ($\omega$), vertical velocity ($V_z$), horizontal velocity ($V_{xy}$), thrust setpoint throttle.
