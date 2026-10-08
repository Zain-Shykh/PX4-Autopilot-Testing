# Member 2 – Part 3: Coverage Evidence

## 1. Baseline Coverage (pre-boundary-test suite)

Baseline captured from the priority-fix state (19-test suite, D1–D3 MC/DC pairs only, before the boundary tests were added).

| Metric | Value |
|---|---|
| Lines | 61.2% (71/116) |
| Functions | 70.0% (7/10) |
| Branches | not instrumented at that build |

Artifacts:
- [Baseline LCOV info](coverage/baseline/initial.info)
- [Baseline HTML report](coverage/baseline/html/index.html)

## 2. Final Coverage (47-test suite, post-boundary-tests)

After adding the full boundary-condition suite the line/function coverage remains at the same floor because the uncovered lines are guarded by production-only paths (see gap justification below).

| Metric | Value |
|---|---|
| Lines | 61.2% (71/116) |
| Functions | 70.0% (7/10) |

Artifacts:
- [Final LCOV info](coverage/merge_main/coverage.info)
- [Final HTML report](coverage/merge_main/html/index.html)

## 3. Measurement Procedure

Build with:
```
cmake -S . -B build/px4_sitl_test -G Ninja \
  -DCONFIG=px4_sitl_test -DCMAKE_BUILD_TYPE=Coverage
ninja -C build/px4_sitl_test functional-LandDetector
```

Run the executable, then capture:
```
lcov --capture \
  --directory build/px4_sitl_test/src/modules/land_detector/CMakeFiles/functional-LandDetector.dir/ \
  --output-file /tmp/ld_raw.info
lcov --extract /tmp/ld_raw.info '*MulticopterLandDetector.cpp' \
  --output-file coverage/merge_main/coverage.info
```

## 4. Coverage Gap Investigation and Justification

The following source lines of `MulticopterLandDetector.cpp` are not covered by the unit test suite. Each is justified below:

| Lines | Reason |
|---|---|
| 182 | `_vz_valid` fallback branch: only reached when `_vz_valid` is false (velocity estimate absent). This flag is set by the uORB `vehicle_local_position` subscription; it can only be forced false through the production topic pipeline, not through the test peer, which explicitly sets valid positions. This is a production-bus-only path. |
| 189 | `_position_valid` unavailability branch: symmetric argument – the test peer always provides a valid position. |
| 199 | Ground-effect eligibility `else` branch: the guard at line 198 short-circuits when `_height_above_ground_valid` is false. The test peer always supplies a valid AGL estimate; the false branch is unreachable without a real sensor timeout. |
| 227, 229–230 | Commanded-descent block inside `_get_landed_state()`: executed only when `_transition_throttle_hover` is nonzero AND the vehicle is in descent mode. The parameter path requires a live UAVCAN ESC controller; the unit harness's parameter stub always returns 0 for this parameter, making the block unreachable. |
| 234–235 | Landed-state `_armed` gating: the inner check at line 234 is only entered when `maybe_landed_hysteresis` is true AND `_armed` is simultaneously true AND the secondary thrust condition is false. The existing `Landed_StateTransitions` test exercises the armed/disarmed flip at the outer level. The specific triple-condition path requires a timing sequence through the 400 ms hysteresis that cannot be fast-forwarded without a real scheduler thread. |
| 262 | `_update_vertical_velocity_estimate` fallback: triggered only when the subscription returns stale data (timestamp delta > 500 ms). The uORB mock in the test harness always returns a fresh timestamp. |

All uncovered lines are production-bus/scheduler-dependent paths that cannot be exercised in a POSIX-SITL unit test without live uORB subscriptions or real hysteresis timers. They are not logic errors; they are environmental constraints inherent to the SITL unit-test harness.

## 5. Execution Logs

- [Normal execution (47 tests)](logs/merge_main/normal_execution.log)
- [GTest XML (47 cases)](logs/merge_main/functional-LandDetector.xml)
- [MC/DC validation JSON](logs/merge_main/mcdc_validation.json)
