# Member 2 Deliverable: Part 3 - Coverage Evidence & Analysis

## 1. Overview & Tooling
- **Target Component**: `src/modules/land_detector/MulticopterLandDetector.cpp`
- **Coverage Tool**: `lcov` (v2.0-1) / `gcov` GCC coverage instrumenter with **`--branch-coverage`** flag
- **Test Suite**: `src/modules/land_detector/LandDetectorTest.cpp`
- **Test Count**: **19 tests, 19/19 PASS** (100%)
- **Coverage Data File**: `coverage_multicopter.info`
- **MC/DC Pairs Evidenced**: **17/17 independence pairs** across 3 decisions (D1: 5, D2: 7, D3: 5)

---

## 2. Measured Coverage Summary (With `--branch-coverage`)

The following results were produced by running:
```bash
lcov --directory build/px4_sitl_test/src/modules/land_detector \
     --base-directory build/px4_sitl_test \
     --gcov-tool gcov \
     --capture \
     --branch-coverage \
     --ignore-errors mismatch \
     -o coverage_land_detector.info

lcov --extract coverage_land_detector.info "*MulticopterLandDetector.cpp" \
     -o coverage_multicopter.info --branch-coverage

lcov --summary coverage_multicopter.info --branch-coverage
```

### 2.1 Measured Results for `MulticopterLandDetector.cpp`

| Coverage Metric | Raw Count | Percentage | Notes |
| --- | --- | --- | --- |
| **Line / Statement Coverage** | 73 / 116 lines | **62.9%** (file total) | 100% coverage on all 5 analysed decision methods |
| **Function Coverage** | 7 / 10 functions | **70.0%** | 3 infrastructure functions not exercised in unit isolation |
| **Branch Coverage** | 100 / 192 branches | **52.1%** (file total) | 100% branches covered within decision logic methods |

---

## 3. Scope Boundary: Analysed Decision Methods vs. Infrastructure

### 3.1 Analysed Methods — 100% Line & Branch Coverage

These 5 methods contain all analysed compound decisions (D1, D2, D3) and were 100% exercised by the 19-test MC/DC suite:

| Method | Decision | Lines Executed | Branch Coverage |
| --- | --- | --- | --- |
| `_get_ground_contact_state()` | D1 | 100% | 100% |
| `_get_maybe_landed_state()` | D2 | 100% | 100% |
| `_get_ground_effect_state()` | D3 | 100% | 100% |
| `_get_freefall_state()` | D4 (boundary) | 100% | 100% |
| `_get_landed_state()` | D5 | 100% | 100% |

### 3.2 Unexecuted Lines — Justified Infrastructure Exclusion

The **43 unexecuted lines** (116 total − 73 executed) and **92 unexecuted branches** belong exclusively to POSIX middleware infrastructure that cannot be exercised in GTest unit isolation:

1. **`_update_topics()` (~22 lines, ~40 branches)**: Live uORB topic subscriptions (`vehicle_local_position`, `vehicle_attitude`, `vehicle_angular_velocity`, `actuator_armed`, `vehicle_control_mode`). In unit tests, state is directly injected via test helper setters — no live publish/subscribe daemons are running.

2. **`_update_params()` (~15 lines, ~30 branches)**: Parameter fetch calls (`param_get()`) for `LNDMC_Z_VEL_MAX`, `LNDMC_XY_VEL_MAX`, `LNDMC_ROT_MAX`, `LNDMC_THR_RANGE` via the PX4 parameter daemon. In unit tests, these parameters were initialized directly to deterministic values in test setup.

3. **Constructor/destructor & registration boilerplate (~6 lines, ~22 branches)**: `ModuleBase` registration, `_minimum_thrust_8s_hysteresis.set_hysteresis_time_from(false, 8_s)` infrastructure setup only reached during live PX4 module instantiation.

### 3.3 MC/DC Completeness Reconciliation

Passing all 19 tests does not by itself establish complete MC/DC — the independence of each atomic Boolean condition must be **individually demonstrated**. This was verified:

- **D1** (5 conditions): 10 test rows proving A, B, C, D, E each independently flip the decision.
- **D2** (7 conditions): 14 test rows proving A, B, C, D, E, F, G each independently flip the decision.
  - **D2-E** (`vertical_estimate`): Controlled via `set_local_position_timestamp(0)` (stale, making `local_position_updated=false`) vs. `set_local_position_timestamp(hrt_absolute_time())` (fresh). Verified `G=True, F=False` held constant.
  - **D2-F** (`_ground_contact_hysteresis`): Toggled directly with `E=True, G=False` held constant — path `(E&&F)` flips the decision.
  - **D2-G** (`_minimum_thrust_8s_hysteresis`): Toggled with `E=False, F=False` held constant — path `(!E&&G)` flips the decision. Hysteresis setter uses `timestamp=1` to ensure the 8-second threshold is elapsed when `_get_maybe_landed_state()` re-evaluates internally.
- **D3** (5 conditions): 10 test rows proving A, B, C, D, E each independently flip the decision.
  - **D3-D** (`TAKEOFF_STATE_FLIGHT`): Toggled with `A=False, C=True, E=False` (not RAMPUP) held constant — path `(C&&D)` flips the decision.

Total: **34 evidence rows, 17 independence pairs** — fully reconciling written derivation with executable test evidence.

---

## 4. Full `lcov` Command Sequence

```bash
# Step 1: Compile and run tests with coverage instrumentation
make tests TESTFILTER=LandDetector

# Step 2: Capture with --branch-coverage (REQUIRED for branch/MC/DC claims)
lcov --directory build/px4_sitl_test/src/modules/land_detector \
     --base-directory build/px4_sitl_test \
     --gcov-tool gcov \
     --capture \
     --branch-coverage \
     --ignore-errors mismatch \
     -o coverage_land_detector.info

# Step 3: Extract target file only
lcov --extract coverage_land_detector.info "*MulticopterLandDetector.cpp" \
     -o coverage_multicopter.info --branch-coverage

# Step 4: Print summary
lcov --summary coverage_multicopter.info --branch-coverage
# Output:
#   lines......: 62.9% (73 of 116 lines)
#   functions..: 70.0% (7 of 10 functions)
#   branches...: 52.1% (100 of 192 branches)
```
