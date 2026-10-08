# Priority corrections — 8 October 2026

> **Historical correction snapshot.** The subsequent [main-branch integration](MAIN_MERGE.md) keeps Member 2's direct D3-B setter and refreshes execution, workbook, patch, and coverage evidence. Use that record for the current merged state.

The incorrect D1-D3 MC/DC evidence and the reproduced battery test-order failure have been repaired. **All 78 existing student cases pass.** The land-detector and battery suites each also pass **20 shuffled repetitions**, including the previously failing seed 2027. No production algorithm was changed.

This completes the immediate evidence corrections and battery isolation. **It does not complete every obligation in the original audit's first priority:** additional governing decisions still require MC/DC tests and analysis. The full assignment is not ready for final submission.

## Corrections made

- **D3-B now changes only B.** Refreshing horizontal movement also cleared `_in_descend`; the test now restores it before checking the second vector. Both runtime vectors assert A remains true.
- **D2-E now keeps G true in both rows.** Only E changes, and the actual production result changes with it. The CSV, report, and shared workbook agree.
- **Every documented vector is checked at runtime.** The land-detector tests assert and record all 34 controlled operand vectors and actual production outcomes in GTest XML. D1-A, D2-A, and D3 cached flags now explicitly match their documented inputs.
- **Land-detector time and thresholds are controlled.** A test-only linker clock replacement removes dependence on host uptime. Fresh/stale timestamps and the eight-second hysteresis are set deterministically; the production clock is unchanged.
- **Battery tests reset shared state.** Parameters are reset between cases, and neutral vehicle-status/flight-phase messages replace retained messages. The disconnected case explicitly sets unknown capacity (`BAT1_CAPACITY=0`) before expecting a NaN remaining-time result.
- **The nonexistent battery MC/DC equation is withdrawn.** Member 3's analysis now describes the actual warning ladder and points to the group's land-detector matrix. It does not claim battery MC/DC or DO-178C compliance.
- **Handoff artifacts are refreshed.** The Member 1/2 workbook has 55 cases and 34 matrix rows; Member 3's separate workbook retains its 23 cases with current source references. Source copies, patches, evidence links, and assistance records have been updated. Older Member 3 report sections are explicitly marked as draft material awaiting the remaining audit corrections.

## Verification

| Check | Result | Evidence |
|---|---|---|
| Build affected targets | PASS | [Initial build](priority_fix/initial_build.log), [final incremental build](priority_fix/final_build.log) |
| All five student suites, normal order | 78/78 PASS | [Full execution log](priority_fix/verified_execution.log) |
| Land detector, seeds 2027–2046 | 19 × 20 = 380 PASS | [Shuffle log](../member2_deliverables/logs/priority_fix/functional-LandDetector_shuffle.log) |
| Battery, seeds 2027–2046 | 11 × 20 = 220 PASS | [Shuffle log](../member3_deliverables/logs/priority_fix/functional-Battery_shuffle.log) |
| D1-D3 runtime matrix validation | 34 vectors, 17 pairs PASS | [Validation JSON](priority_fix/verified_mcdc_validation.json) |
| Original incorrect matrix | Rejected as expected | [Negative check](priority_fix/validator_negative_check.log) |
| Member 1 and Member 1/2 patches | Apply and reconstruct exact current files | [Patch log](../member1_deliverables/logs/patch_validation.log) |
| Member 3 patch and source copies | Apply and reconstruct exact current files | [Patch log](../member3_deliverables/logs/priority_fix/patch_validation.json) |
| Workbook contents and repeated-run counts | Match the CSV/XML/log evidence | [Artifact check](priority_fix/artifact_validation.json) |

The [validator](../member2_deliverables/scripts/validate_mcdc.py) checks passing XML, vector/outcome agreement, one changed condition per independence pair, changed decision results, and both values of each evaluated condition. Short-circuit evaluation masks are **derived from the source expressions**; they are not compiler-generated MC/DC instrumentation. D2's repeated E/!E occurrences are treated as one coupled logical input. The claim is limited to the three documented return decisions.

The functional harness still emits work-queue diagnostics, and the analog-battery case emits an invalid-parameter diagnostic. Passing GTest results do not resolve those remaining harness/assertion concerns or establish whole-system correctness.

## Current coverage and counts

Coverage was captured from the fresh normal run only, with isolated counters and exact includes for six production `.cpp` files. Shuffled-run counters were kept separate. The [LCOV file](priority_fix/verified_coverage.info) contains raw GCC branch edges; [summary JSON](priority_fix/coverage_summary.json) records exact denominators.

- Land detector: **71/116 lines**, **96/192 raw branch edges**, **7/10 functions**. The ground-effect return reaches all 10 recorded edges, up from 9; this does not establish complete component coverage.
- Battery: **218/231 lines**, **156/248 raw branch edges**, **19/19 functions**.
- Member 3's three-file total: **423/518 lines (81.7%)**, **307/564 raw branch edges (54.4%)**, **37/44 functions (84.1%)**. These replace the earlier capture for this corrected run.
- Other four production files retain the counts in the original audit.

Current HTML: [Member 2](../member2_deliverables/coverage/priority_fix/html/index.html), [Member 3](../member3_deliverables/coverage/priority_fix/html/index.html). Each is generated from its matching `coverage.info` in the same directory. The old captures remain historical.

| Member | Cases | Breakdown |
|---|---:|---|
| Member 1 | 36 | 29 estimator + 7 hysteresis |
| Member 2 | 19 | Land detector |
| Member 3 | 23 | 12 flight-mode-manager + 11 battery/analog-battery |
| Total | **78** | No extra cases counted for matrix rows or repeated runs |

## Repeat the checks

From the repository root, using the configured coverage build and installed PX4 dependencies:

```bash
cmake --build build/px4_sitl_test --target \
  unit-HysteresisAssignment functional-AttitudeEstimatorQ \
  functional-LandDetector functional-Battery functional-FlightModeManager -j 4

priority_run=$(mktemp -d /tmp/sqe-priority-XXXXXX)
GCOV_PREFIX="$priority_run/normal/profiles" \
GTEST_OUTPUT="xml:$priority_run/normal/xml/" \
ctest --test-dir build/px4_sitl_test \
  -R '^(unit-HysteresisAssignment|functional-AttitudeEstimatorQ|functional-LandDetector|functional-Battery|functional-FlightModeManager)$' \
  --timeout 30 -V

python3 member2_deliverables/scripts/validate_mcdc.py \
  --xml "$priority_run/normal/xml/functional-LandDetector.xml"

for suite in functional-LandDetector functional-Battery; do
  GCOV_PREFIX="$priority_run/shuffle/profiles" \
    "build/px4_sitl_test/$suite" \
    --gtest_shuffle --gtest_random_seed=2027 --gtest_repeat=20
done
```

The saved coverage capture used GCC/gcov 13.3 with the configured `Coverage` build. Fresh `.gcda` files for the six selected production objects were paired with their matching build `.gcno` files in a separate capture directory. LCOV used `--capture --branch-coverage --ignore-errors mismatch` and exact source-file includes. HTML used `genhtml --branch-coverage`. The [capture log](priority_fix/verified_coverage_capture.log) preserves the collector output. These results are a student-suite capture, not a newly measured upstream baseline.

## Remaining work

1. Complete applicable land-detector governing-guard MC/DC obligations listed in [Member 2 Part 2](../member2_deliverables/part2_mcdc_analysis.md).
2. Repair the battery capacity-fusion setup, improve weak output assertions, and add justified missing boundary/error cases from the [original audit](ASSESSMENT.md).
3. Measure comparable baseline/final coverage and replace the remaining incorrect Member 3 gap explanations and unsupported quality claims.
4. Consolidate the final group workbook, report, patch, AI-assistance record, group details, and filenames. The current combined patch covers Member 1 and Member 2; Member 3 is still separate.

The original audit is retained as a before-fix snapshot. Use this record and the `priority_fix` evidence directories for the corrected state.
