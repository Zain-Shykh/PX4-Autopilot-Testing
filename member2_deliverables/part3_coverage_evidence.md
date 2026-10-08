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
| **Line / Statement Coverage** | 71 / 116 lines | **61.2%** (file total) | Clean post-test capture; not 100% method coverage |
| **Function Coverage** | 7 / 10 functions | **70.0%** | 3 infrastructure functions not exercised in unit isolation |
| **Branch Coverage** | 94 / 192 branches | **49.0%** (file total) | Clean post-test capture; includes selected methods and infrastructure |

---

## 3. Scope Boundary: Analysed Decision Methods vs. Infrastructure

### 3.1 Analysed Methods

These five methods contain the selected decisions (D1-D3) and two simple state checks (D4-D5). The suite invokes each method, but invocation is not equivalent to complete line/branch coverage. LCOV records uncovered alternatives at lines 180, 260, 269, 276, and 280. The table identifies the tested logic, not a claim of complete coverage:

| Method | Decision | Lines Executed | Branch Coverage |
| --- | --- | --- | --- |
| `_get_ground_contact_state()` | D1 | Invoked | Selected truth paths exercised |
| `_get_maybe_landed_state()` | D2 | Invoked | Selected truth paths exercised |
| `_get_ground_effect_state()` | D3 | Invoked | Selected truth paths exercised |
| `_get_freefall_state()` | D4 (threshold) | Invoked | Boundary outcomes exercised |
| `_get_landed_state()` | D5 | Invoked | Main outcomes exercised |

### 3.2 Unexecuted Lines and Branches — Gap Analysis

The **43 unexecuted lines** (116 total − 73 executed) and **92 unexecuted branches** are not exclusively middleware infrastructure. They include unexecuted topic/parameter paths and alternative branches inside the selected methods:

1. **`_update_topics()` (~22 lines, ~40 branches)**: Live uORB subscriptions for thrust setpoint, control mode, hover-thrust estimate, and takeoff status. In unit tests, state is directly injected via test helper setters — no live publish/subscribe flow is exercised.

2. **`_update_params()` (~15 lines, ~30 branches)**: Parameter fetches and the `LNDMC_Z_VEL_MAX` consistency correction via the PX4 parameter system. In unit tests, parameters were not exercised through this update path.

3. **Selected-method alternatives**: invalid or stale local-position data, velocity-validity fallbacks, distance-estimate handling, hover-thrust validity, and short-circuit outcomes remain partly uncovered.
4. **Constructor/destructor & registration boilerplate**: constructor setup and hysteresis-factor paths are not fully represented by this unit fixture.

### 3.3 MC/DC Completeness Reconciliation

Passing all 19 tests does not by itself establish complete MC/DC — the independence of each atomic Boolean condition must be **individually demonstrated**. The matrix demonstrates the following selected pairs:

- **D1** (5 conditions): 10 test rows proving A, B, C, D, E each independently flip the decision.
- **D2** (7 conditions): 14 test rows proving A, B, C, D, E, F, G each independently flip the decision.
  - **D2-E** (`vertical_estimate`): Controlled via `set_local_position_timestamp(0)` (stale, making `local_position_updated=false`) vs. `set_local_position_timestamp(hrt_absolute_time())` (fresh). Verified `G=True, F=False` held constant.
  - **D2-F** (`_ground_contact_hysteresis`): Toggled directly with `E=True, G=False` held constant — path `(E&&F)` flips the decision.
  - **D2-G** (`_minimum_thrust_8s_hysteresis`): Toggled with `E=False, F=False` held constant — path `(!E&&G)` flips the decision. Hysteresis setter uses `timestamp=1` to ensure the 8-second threshold is elapsed when `_get_maybe_landed_state()` re-evaluates internally.
- **D3** (5 conditions): 10 test rows proving A, B, C, D, E each independently flip the decision. The B pair directly controls `_horizontal_movement`, so changing B cannot also change A through `_get_ground_contact_state()`.
  - **D3-D** (`TAKEOFF_STATE_FLIGHT`): Toggled with `A=False, C=True, E=False` (not RAMPUP) held constant — path `(C&&D)` flips the decision.

Total: **34 evidence rows, 17 independence pairs** for D1-D3. This is complete for the selected matrix only, not for every compound decision in the class.

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
#   lines......: 61.2% (71 of 116 lines)
#   functions..: 70.0% (7 of 10 functions)
#   branches...: 49.0% (94 of 192 branches)
```
