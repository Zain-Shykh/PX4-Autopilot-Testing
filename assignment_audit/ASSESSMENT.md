# Assignment 02 audit — 8 October 2026

> **Historical audit, before fixes.** The follow-up [priority-fix record](PRIORITY_FIX.md) identifies the corrected MC/DC pairs, battery isolation, refreshed artifacts, and remaining work. Counts and failures below describe the original audited state.

**Verdict: substantial work is implemented, but the combined submission is not ready.** All 78 student test cases compile and pass in their normal order. However, passing tests do not establish the reported MC/DC, several coverage explanations contradict the source, a battery test fails when order changes, and the final workbook/report/patch have not been consolidated.

Member 1 has the strongest and most reproducible evidence. Member 2 has implemented the four previously missing tests, but important MC/DC and coverage errors remain. Member 3 has useful working functional tests and reproducible coverage percentages, but its written analysis needs substantial correction.

This assessment uses the **original six-page [assignment PDF](../SQE-Assignment02.pdf)** as the requirements authority. The work-distribution document is a team plan, not an additional marking rubric. The inspected checkout is `63242fbe0c`; the full identifier and PDF checksum are in [audit_manifest.json](audit_manifest.json). Existing source files and submission artifacts were not edited. The local test build was refreshed, and new review evidence was saved separately here.

## Test-case counts

I counted actual `TEST`/`TEST_F` declarations and matched every case to fresh GTest XML, rather than counting assertions, Boolean matrix rows, or CTest executables.

| Member | Implemented cases | Breakdown | Fresh normal execution |
|---|---:|---|---|
| Member 1 | **36** | 29 attitude-estimator functional tests + 7 new hysteresis unit tests | **36/36 PASS** |
| Member 2 | **19** | 5 ground-contact MC/DC tests + 7 maybe-landed MC/DC tests + 5 ground-effect MC/DC tests + 2 freefall/landed tests | **19/19 PASS** |
| Member 3 | **23** | 12 flight-mode-manager tests + 11 battery/analog-battery tests | **23/23 PASS** |
| **Total** | **78** | Five test executables | **78/78 PASS in normal order** |

The seven pre-existing upstream hysteresis tests are **not** included in the 78. Member 1's inventory has 51 rows because it contains its own 36 cases plus the **older 15-case Member 2 suite**; 51 is not Member 1's contribution. Member 2's 34 MC/DC evidence rows describe 17 pairs, not 34 additional test cases. Member 3's two diagnostic/CLI tests count as implemented tests, but should not be used to make the assessed business/control scope appear larger.

The [complete 78-case inventory](test_case_inventory.csv) lists member, submitted ID where available, test name, current source location, execution result, and test-specific concerns. Git authorship associates the tracks with `huzaimamalik`, `Muhammad Zain`, and `m-hamza-atif`, respectively; the group's formal names and roll numbers still need to be entered in the submission.

## What is correct

- **Correct baseline commit.** `d6f12ad1c4f70ad3230afd7d86e971421e02fef4` exists locally and matches the commit linked by the official [PX4 v1.17.0 release](https://github.com/PX4/PX4-Autopilot/releases/tag/v1.17.0). The missing local tag does not make the selected commit wrong. Record the release baseline separately from the later team commit.
- **Meaningful production scope.** Estimation, hysteresis, multicopter landing, task selection, and battery calculations are appropriate non-trivial targets. Functional GTest is justified for uORB/parameter dependencies; ordinary unit GTest is suitable for hysteresis.
- **Actual executable contributions.** All five student suites rebuilt successfully and all 78 cases passed in a new normal-order run. The tests call PX4 production code. See [build log](evidence/build.log), [execution log](evidence/normal_execution.log), and [XML results](evidence/xml).
- **No observed production-algorithm rewrite to inflate coverage.** The inspected production-source changes are access/testability changes: two friend declarations and Member 3's `private` → `protected` changes. The latter still need explicit justification. The CMake registrations are present and work.
- **Member 1's measurements reproduce.** Its submitted LCOV totals match the audit. The upstream baseline, student-only run, and final run are separated; test/generated sources are excluded from its reported production denominator. Its 238-row gap inventory enumerates 26 missed lines and 212 missed raw branch edges. The Member 1 checksum manifest also validates when checked from its own directory.
- **Member 1's new hysteresis cases add useful distinctions.** Exact deadlines, cancellation at the deadline, changing a pending delay, independent objects, and a large non-overflowing timestamp go beyond simply renaming upstream tests. The report correctly admits that upstream already had complete line coverage.
- **The bias-limit defect is supported.** `_bias_max` remains zero, `ATT_BIAS_MAX` is declared but never copied into it, and the learning code clamps against zero. The same wiring omission exists in the release baseline. `ConfiguredBiasLimitIsNotApplied` passed again. The report properly calls it a characterization test; a passing characterization is not proof that the defective behavior is correct. See [production source](../src/modules/attitude_estimator_q/attitude_estimator_q_main.cpp) and [test](../src/modules/attitude_estimator_q/AttitudeEstimatorQTest.cpp).
- **Member 2's recent additions are real.** D2 E/F/G and D3 D tests now exist, and the current CSV contains 34 rows. The previous integration notes saying those tests are absent are stale.
- **Member 3's principal coverage percentages have real support.** Its earlier branch-enabled HTML and the fresh audit both show 421/518 lines and 303/564 raw branches across the three reported production files. The separate bundled LCOV file represents a different capture, discussed below.

## Member 1 assessment

**The module work is largely sound; the team integration artifacts are stale.**

The estimator cases control time, drain retained messages, and reset parameters; the hysteresis cases supply time explicitly. Assertions include numerical outputs and state changes. The full estimator result is accurately reported as 199/225 lines and 347/556 raw branches; the narrower 184/184 core-method line result is explicitly distinguished from full-file and branch coverage. The three missing hysteresis edges follow an actual Boolean invariant: when requested state differs from current state, only the two opposite Boolean combinations are possible.

The callback/copy failure paths, defensive quaternion guards, compiler exception edges, and wrapper exclusions are disclosed. Their explanations describe the chosen synchronous fixture and assumptions; they should not be broadened into a claim that all such paths are universally unreachable or that the whole estimator has 100% branch coverage.

Required corrections:

1. **Refresh the shared workbook.** It has only 36 Member 1 + 15 older Member 2 inventory rows, and only 26 older MC/DC rows. It omits all 23 Member 3 cases and the four new Member 2 cases: **27 current tests are missing**.
2. **Update the integration notes/report/AI record.** They still describe Member 3 as absent and the four Member 2 tests as missing. Those statements were historical, not current.
3. **Regenerate the combined patch.** `px4_assignment2_tests.patch` reconstructs only the older 15-test land-detector file and has no Member 3 changes. The Member 1-only patch is current and applies cleanly.
4. **Finish the group deliverables.** Member 1's report explicitly describes itself as a handoff. It is not a consolidated report with the required group judgment and identification details.

These are integration shortcomings, not reasons to miscount Member 1's own 36 implemented cases.

## Member 2 assessment

**The decision selection is relevant and the 19 tests run, but complete MC/DC and 100% decision-method coverage are not established.**

### 1. The D3-B test does not isolate condition B

In [LandDetectorTest.cpp](../src/modules/land_detector/LandDetectorTest.cpp), `GroundEffectMCDC_ConditionB_NoHorizontalMovement` sets `_in_descend=true`, then calls `test_get_ground_contact_state()` to refresh horizontal movement before its second assertion.

That production method also sets `_in_descend=false` when climb-rate control is disabled; it is disabled in this fixture. Thus the second vector changes **A and B**, rather than only B:

- Intended: `[T,T,F,F,F] → [T,F,F,F,F]`.
- Actual: `[T,T,F,F,F] → [F,F,F,F,F]`.

The false result can be explained by A alone. This is an invalid independence demonstration even though both assertions pass. The fresh ground-effect coverage is **9/10 branches**, consistent with a missing outcome.

**Correction:** refresh/control the cached movement flag without losing the intended descent value, assert the relevant condition values, and rerun the pair and coverage.

### 2. The D2-E CSV pair changes two conditions

The current [MC/DC CSV](../member2_deliverables/sheet2_mcdc_evidence.csv) records:

```text
[F T T T F F T] -> True
[F T T T T F F] -> False
```

Both **E and G change**. This does not satisfy the report's stated rule of holding the other conditions fixed. The code is intended to keep G true, while the prose and CSV disagree on vector values and pair-label orientation. Reconcile the matrix with actual execution; do not merely add rows and declare 17/17 independence pairs.

### 3. Other recorded atomic values do not match fixture state

Examples:

- D1-A's “all other conditions false” vectors do not match the default skipped distance check, zero thrust, and valid zero vertical velocity.
- D2-A does not make all other conditions false: freefall, rotation, fresh vertical estimate, and initial ground-contact state differ from the recorded vector.
- In D3-C/D/E, setting velocity does not itself update cached `_horizontal_movement`. Several rows record B=false although the cached default makes `!_horizontal_movement` true.

Some of these tests may still establish the intended logical independence with corrected vectors. The submitted matrix is nevertheless inaccurate. Also distinguish a controlled Boolean value from a condition actually evaluated under C++ short-circuiting.

### 4. The selected critical behavior has additional compound guards

Analyzing only the three return expressions misses decisions that directly govern those results. Examples in [MulticopterLandDetector.cpp](../src/modules/land_detector/MulticopterLandDetector.cpp) include:

- Valid vertical velocity versus threshold and the `z_deriv` fallback, lines 177–182.
- Position availability and horizontal-velocity validity, line 194.
- Position/range validity and the configured ground-effect threshold, line 202.
- Hover-thrust freshness/descent retention, line 211.
- Finite descent setpoint and threshold, lines 229–230.
- Maybe-landed/landed gating and the distance-check alternatives, lines 234–246.
- `local_position_updated && vertical_velocity_valid`, line 285.

The PDF requires MC/DC for applicable compound decisions implementing or directly governing the selected critical behavior, not merely its final Boolean expressions. Either add the missing analysis/tests or provide a defensible behavior boundary for each exclusion.

### 5. The 100% method-coverage claim is false

Fresh coverage of exactly the five claimed methods is:

| Method | Executed lines | Raw branch edges |
|---|---:|---:|
| `_get_ground_contact_state()` | **32/40** | **38/68** |
| `_get_maybe_landed_state()` | **18/19** | **25/32** |
| `_get_ground_effect_state()` | **4/4** | **9/10** |
| `_get_freefall_state()` | **2/2** | No branch edges emitted in this capture |
| `_get_landed_state()` | **2/2** | **4/4** |

The full file is **71/116 lines (61.2%) and 95/192 branches (49.5%)** in the fresh normal run, versus the document's 73/116 and 100/192. Missed business-method lines include **182, 189, 199, 227, 229, 230, 234, 235, and 262**. Therefore, missing coverage is not exclusively outside those methods, regardless of compiler exception edges.

The gap explanation also misclassifies `_update_params()`, which contains threshold correction and hover-thrust initialization logic. Functional GTest already initializes uORB and parameters; these dependencies do not categorically require inaccessible daemons. The constructor and its hysteresis setup execute in every fixture, contrary to the explanation that they are only reached in a live module.

### 6. Boundary, artifact, and reporting gaps remain

- `Freefall_AccelThresholdBoundary` tests approximately 1.414 and 9.81 m/s², **not equality at 2.0**. Add below/equal/above cases for relevant strict comparisons, distance, freshness, rotation, and hysteresis timing.
- The 8-second setup uses timestamp 1 and assumes host time exceeds 8 seconds; timestamp zero is assumed stale. These pass locally but are not an explicit controlled-clock proof.
- The private `_params` aggregate remains zero-initialized; the fixture does not load or explicitly set the manual/minimum/hover-throttle parameters as the report suggests. Make the intended thresholds explicit.
- No submitted `coverage_multicopter.info`, land-detector HTML report, or comparable baseline capture was found. Commands and typed percentages are not generated coverage evidence. The audit's new tracefile is review evidence, not a pre-existing submission artifact.
- `make tests TESTFILTER=LandDetector` alone does not guarantee a Coverage configuration on a fresh build; document the build type explicitly.
- The README still says “15/15” in its expected result.
- The purported **322-word** quality judgment has about **250 whitespace-separated words** in its actual three-paragraph body; it falls below 300. It also repeats the incorrect coverage/MC/DC conclusion.

## Member 3 assessment

**The 23 functional tests provide useful execution evidence, but several reported behaviors are not what the source/tests actually implement.**

### 1. The battery MC/DC decision is absent from production

The report and [MC/DC analysis](../member3_deliverables/part2_mcdc_analysis.md) describe:

```text
D = (V_cell <= V_crit AND V_cell > 2.0) OR (SoC <= SoC_crit)
```

This is not the warning decision in the supplied `battery.cpp`. `determineWarning()` at lines 307–321 uses successive **strict `<` state-of-charge comparisons**. `updateBatteryStatus()` gates warning calculation using `_connected && _battery_initialized`.

The abstract truth-table arithmetic is fine for the stated equation, but the equation has no demonstrated source/test mapping here. Its A/B values are also physically coupled through the same voltage: with a critical voltage above 2.0 V, A=false and B=false cannot both hold, yet the claimed full table includes that combination without discussing feasibility.

Furthermore, **Member 3's XLSX MC/DC sheet and CSV contain only headers and zero evidence rows**, despite the report saying both sheets are fully populated.

**Correction:** use the land detector as the group's single required MC/DC component, complete that evidence, and remove the unsupported battery claim. A second MC/DC component is optional, but any retained analysis must use an actual source decision and executable evidence. The assignment does not require a DO-178C Level A compliance claim.

### 2. A battery test is order-dependent — reproduced

Normal execution passes all 11 battery tests. A fresh shuffled run with seed 2027 fails `DisconnectedBatteryState` at `BatteryTest.cpp:128` because `time_remaining_s` is finite rather than NaN. See [failure log](evidence/battery_seed2027.log).

`ComputeRemainingTimeArmedAndFixedWing` sets global `BAT1_CAPACITY=2200`; the fixture only disables autosave and does not reset parameters. A later disconnected test inherits that capacity. `computeRemainingTime()` can compute a result when capacity is positive; its final capacity guard does not require connected state. Thus the failing assertion reflects an uncontrolled fixture assumption, not a demonstrated PX4 defect.

Reproduction from the repository root:

```bash
GCOV_PREFIX=/tmp/sqe-audit-recheck-profiles \
  build/px4_sitl_test/functional-Battery \
  --gtest_shuffle --gtest_random_seed=2027
```

The three-repeat shuffle also fails in later iterations. Flight-mode-manager and land-detector suites passed all three audit shuffle repetitions. Reset/restore relevant parameters and messages, and specify the disconnected test's capacity assumption explicitly before rerunning.

### 3. The coulomb-fusion test does not execute the claimed fusion branch

`StateOfChargeEstimationCoulombFusion` sets capacity directly and then calls `updateParams()`. In normal order, the default capacity parameter overwrites the direct setting with zero. The audit shows **battery.cpp lines 293–300 unexecuted**, i.e. the actual voltage/coulomb fusion body.

The test also advances time by 10 seconds while `updateDt()` caps one update interval to **2 seconds**. The stated “10 A for 10 seconds ≈27.78 mAh” does not describe that single update. Its assertions only require positive discharged charge and non-increasing SoC, which can pass without capacity fusion.

**Correction:** configure capacity through the parameter before construction/update, or set it after the update; control initialization; supply valid sample intervals; and assert the expected integrated charge and fusion result.

### 4. Several assertions do not establish the report's conclusions

- **RLS/load-drop stability:** no test applies the claimed 0→30 A transient or checks covariance convergence/divergence. The report's BAT-09 reference is actually the remaining-time test. BAT-07 preloads a higher filter value for the loaded case, so its increasing-SoC assertion is not an independent proof of resistance-estimator correctness.
- **Fixed-wing remaining time:** the test publishes a flight-phase timestamp advanced ahead of the real clock. Production subtracts it from real `hrt_absolute_time()`; this does not establish the intended fresh level-flight branch. A positive finite answer alone is insufficient.
- **ADC conversion:** the test checks connection and voltage `>0`, rather than the expected divider-scaled voltage and offset-scaled current. It does not establish correct 15 V/10 A conversion.
- **Speed command:** `HandleCommandOrbitAndSpeedChange` only checks that a task remains active after the speed command, not that its cruise speed changed.
- **Landing gear:** `GenerateTrajectorySetpointAndLandingGear` checks setpoint/constraint publication, without checking gear publication or values.
- **Parameter reset:** the position-control test checks the selected task after invalid mode 99, but not that the parameter was reset as the inventory claims.
- **Battery parameters:** BAT-01's inventory promises exact full/empty voltage values, whereas the code only checks inequalities and cell count.

These remain implemented tests, but their reported purpose/expected results need narrowing or stronger assertions.

### 5. The command boundary is misstated

Production uses `now < command.timestamp + 200_ms`, so acceptance is **strictly younger than 200 ms**, not `<=200 ms`. The test exercises a fresh command and a one-second-old command, not the exact boundary. The report also points to FMM-06 instead of FMM-10 for this test. A stale command remains buffered; the test observes it is not applied, not that it is removed from the buffer.

### 6. The coverage-gap explanations contradict source locations

| Written explanation | Actual checked-in source |
|---|---|
| FMM lines 105–118 are custom-command/CLI handlers | They are subscription updates, task selection, command handling, and active-task checks inside `Run()` |
| FMM lines 185–192 are dynamic task execution in `Run()` | They are in `start_flight_task()`, including auto-mode selection |
| FMM lines 245–252 are heap-allocation failure | They finish altitude-control task selection/error handling |
| Battery lines 82–89 clamp voltages above 5.5 V/negative cell counts | They look up capacity, resistance, and source parameters |
| Battery lines 172–179 handle redundant smart-battery priority | They populate output status fields and return the structure |
| Analog battery lines 31–38 poll hardware ADC registers | They are a license-comment ending and includes |

The current CMake already links the generated flight tasks, and functional tests already instantiate them. Missing `Run()` execution is not explained by those tasks being absent. Likewise, `updateBatteryStatusADC()` accepts supplied numeric inputs; the cited lines do not establish a hardware-only barrier.

Feasible missed work includes transition/slow-position modes, task activation/fallback failures, synchronous `Run()`, configured internal resistance, zero cell count, real capacity fusion, nonfinite scaling, current overwrite, and channel choices. Enumerate actual missed obligations and explain each technical limit. Do not present these existing paragraphs as a line-by-line investigation.

### 7. The coverage bundle mixes two captures

- The earlier branch-enabled pages under `coverage/html/lib/` and `coverage/html/modules/` match the report and fresh audit: battery **216/231**, FMM **164/233**, analog battery **41/54**, total **421/518**; branches **303/564**.
- The bundled `lcov_scope3.info` instead has battery **218/231**, no `BRDA`/`BRF`/`BRH` records, and **44 source records**, including tests, generated files, and task dependencies. Its three selected `.cpp` files total **423/518**.
- The top-level HTML is from the broader later capture and shows **1498/2848 lines**, not the three-file denominator. Older branch pages remain alongside it.
- The documented extraction/globbing and `genhtml` commands do not explicitly enable branch coverage and produce the broad capture. The earlier `scope3_branches_clean.info` named in the branch pages is missing.

Therefore the percentages are **not simply fabricated**: they reproduce. The problem is inconsistent provenance and instructions. Keep one clearly named final production-scope capture, matching HTML, explicit branch flags, isolated counters, and a matching baseline. Do not count test/generated-code execution as production coverage.

### 8. Other inaccurate or incomplete details

- The report says GCC/gcov 11.4 and LCOV 1.14; the supplied README capture output says gcov **13.3.0**. Reconcile environment versions using actual logs. The evidence is from `px4_sitl_test`, while the report labels `px4_sitl_default`.
- The course code is **SE3002**, not CS-4001/SE-4001.
- `isNavStateSwitchingAllowed()` does not exist in the selected FMM source. `updateSubscriptions()` is a **test helper**, not the named production method claimed in the report. Production `Run()` performs the three updates directly.
- The class inherits `px4::WorkItem`, not the stated `px4::WorkItemScheduled`.
- FixedWingLandDetector is listed as part of Member 2's evaluated scope without corresponding student evidence.
- All 12 FMM test-file line anchors refer to earlier locations, often a fixture or different case; several later battery anchors are stale too. Use current test names and source locations.
- The `private`→`protected` changes should be explicitly described and justified as testability changes; a narrower friend seam would reduce the access change.
- The judgment's actual body is about **330 words**, so its length is acceptable. Its claims about MC/DC, RLS convergence, tested fallbacks, and robust safety behavior are not supported by the present tests.

## Coverage independently measured in this audit

These are full-file line counts and **raw GCC/LCOV branch-edge counts**, not a separate MC/DC percentage or proof of all source-level decisions. Compiler-generated edges and wrappers are retained. All values below came from the same new normal-order run with isolated counters.

| Member / production file | Lines | Raw branches |
|---|---:|---:|
| M1 — `hysteresis.cpp` | **22/22 — 100%** | **17/20 — 85.0%** |
| M1 — `attitude_estimator_q_main.cpp` | **199/225 — 88.4%** | **347/556 — 62.4%** |
| M2 — `MulticopterLandDetector.cpp` | **71/116 — 61.2%** | **95/192 — 49.5%** |
| M3 — `battery.cpp` | **216/231 — 93.5%** | **152/248 — 61.3%** |
| M3 — `FlightModeManager.cpp` | **164/233 — 70.4%** | **141/288 — 49.0%** |
| M3 — `analog_battery.cpp` | **41/54 — 75.9%** | **10/28 — 35.7%** |

Evidence: [fresh LCOV](evidence/fresh_normal.info), [summary CSV](fresh_coverage_summary.csv), and [all raw missed line/branch obligations](fresh_uncovered_obligations.csv). The last file is an enumeration for this audit, not a claim that every gap is justified. This table is an audit comparison; the assignment asks the submitted report to avoid merely duplicating coverage-tool percentages in extra summary tables.

## Missing or incomplete group requirements

| Original PDF requirement | Current assessment |
|---|---|
| Fixed release, commit, local setup, actual build/test evidence — pp. 1–3 | Baseline and working setup supported; Member 3 environment text needs correction |
| Meaningful source-derived scope and justified test levels — pp. 2–3 | Good selection; important Member 3 descriptions and Member 2 scope claims inaccurate |
| Statement/branch tests and MC/DC for applicable critical decisions — pp. 3–4 | Substantial tests, but incomplete/invalid MC/DC and uninvestigated feasible gaps |
| Deterministic setup–run–check — p. 4 | Normal run passes; battery fixture demonstrably order-dependent |
| Generated baseline and final coverage — pp. 4–5 | Member 1 has both; Member 2 lacks submitted generated coverage; Member 3 lacks a measured baseline and has mixed final artifacts |
| One workbook, at most two sheets, one row per student test — pp. 4–5 | Two partial XLSX files; no single workbook with all 78 cases and current valid MC/DC |
| Patch against v1.17.0 with all required registration/source changes — pp. 2, 5 | Individual M1/M3 patches apply; combined patch is obsolete and omits M3/latest M2 |
| One concise consolidated report — p. 5 | Separate handoff reports/sections; no complete final group report |
| Group quality judgment, 300–400 words — p. 5 | No complete supported group judgment; M2 body too short; M3 scope/content insufficient for group conclusion |
| Group details and roll-number/section filenames — p. 5 | Placeholders/generic filenames remain |
| Brief AI-assistance record — pp. 2, 5 | M1/M3 records present; consolidate material use and assumptions for the whole group, including M2 if applicable |

There is **no prescribed minimum test count**, failed-test count, or defect count. Physical flight is not required. A second MC/DC component, individual work plans, and a particular report PDF format are not additional requirements imposed by the original PDF. Do not lose time inventing failures or adding trivial cases to increase the number.

## Correction order

1. **Repair evidence validity:** remove the nonexistent battery MC/DC analysis; fix D3-B and D2-E; verify every MC/DC vector against runtime state and short-circuit evaluation; cover applicable governing decisions.
2. **Repair test validity:** isolate battery parameters/messages, fix capacity-fusion setup and clock use, strengthen important output assertions, and add the identified boundary/error cases. Rerun normal and shuffled suites.
3. **Repair coverage analysis:** collect isolated student/baseline/final evidence for the agreed production scope, include branch data, and replace incorrect hardware/daemon/line explanations with actual investigated obligations. Keep compiler edges distinct from source decisions.
4. **Consolidate once tests are final:** one workbook with all current cases, one valid MC/DC sheet, one current baseline patch, exact reproduction commands, and one report with group details and a supported 300–400-word judgment.
5. **Validate the final package:** apply the patch to the release baseline, build/run the submitted cases, check counts/source references/coverage match, and use the required filenames. Ensure every member can defend the entire submission.

The main grading risks are **Part 2 (30 marks)** and **coverage/gap analysis (20 marks)**, followed by deterministic test execution and the final judgment. A precise predicted mark would be speculative because the rubric provides category totals rather than item-by-item deductions, and the viva can change the outcome.

## Audit method and limits

Reviewed the original PDF, member reports and CSV/XLSX files, all five student test sources, associated production code/CMake changes, baseline differences, supplied execution/coverage evidence, source copies, and patch contents. Both Member 1 and Member 3 standalone patches apply to reconstructed baseline files and reproduce their current source copies. The old combined patch applies but reconstructs a stale land-detector suite; see [patch checks](evidence/patch_checks.json).

Fresh commands included:

```bash
cmake --build build/px4_sitl_test --target \
  unit-HysteresisAssignment functional-AttitudeEstimatorQ \
  functional-LandDetector functional-Battery functional-FlightModeManager -j 4

GCOV_PREFIX=/tmp/sqe-assignment-audit/normal/profiles \
GTEST_OUTPUT=xml:/tmp/sqe-assignment-audit/normal/xml/ \
ctest --test-dir build/px4_sitl_test \
  -R '^(unit-HysteresisAssignment|functional-AttitudeEstimatorQ|functional-LandDetector|functional-Battery|functional-FlightModeManager)$' \
  --timeout 30 -V
```

The existing build is explicitly configured `Coverage` with GCC 13.3. The coverage capture used only the new normal-run profiles, paired with their `.gcno` files, `lcov --branch-coverage --ignore-errors mismatch`, and exact includes for the six production `.cpp` files above. Shuffled-run profiles were separate and never merged into these totals. The normal execution log and supplied earlier logs both emit work-queue diagnostics from functional fixtures; GTest results, assertions, and exit status determine the recorded execution result. This audit did not rerun a whole-repository test campaign, rebuild in a completely dependency-free environment, perform flight simulation, or certify autopilot safety.
