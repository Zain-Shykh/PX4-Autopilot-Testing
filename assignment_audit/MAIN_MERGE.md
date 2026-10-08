# Main integration — 8 October 2026

Local `main` was fast-forwarded from `63242fbe0c5db90a257062c1e9d8c5b005412a61` to `fe03631f85edd7fd46f3ba9b189df631ebd18f17`. Both incoming Member 2 commits are preserved with their original author, Muhammad Zain:

- `45b1b2b84070f1f3d2c2608c8035942e5f835a87` — evidence and review added.
- `fe03631f85edd7fd46f3ba9b189df631ebd18f17` — member2 work validated.

The local assignment corrections were reapplied, five overlapping files were reconciled, and the resulting changes are **unstaged and uncommitted**. No push or new branch-history commit was performed. The configured identity remains `huzaimamalik <i243014@isb.nu.edu.pk>`; it was not changed.

## Merge decisions

| Area | Retained implementation and reason |
|---|---|
| D3-B test | Member 2's direct `set_horizontal_movement(false/true)` setup, plus local assertions of both complete runtime vectors and actual production results. This changes only B without a ground-contact helper overwriting A. |
| D2-E | Member 2 and the local correction agree: G stays true in both rows, and the report uses the actual execution order. The CSV now matches `origin/main` exactly. |
| Remaining D1-D3 evidence | Keep local explicit operand setup, the 34 XML properties, matrix validator, controlled clock, and thresholds. These verify the written vectors instead of relying only on passing outcome assertions. |
| Battery fixture | Keep local parameter/message resets and explicit unknown-capacity setup. Incoming commits did not change BatteryTest. |
| Coverage report | Keep the withdrawal of 100% method-coverage claims from both versions. Use the fresh merged execution's measured values and actual remaining obligations; discard stale arithmetic, uptime assumptions, and unsupported infrastructure explanations. |
| Quality judgment | Keep the scoped local judgment, explicitly identify the release baseline, and include the incoming concern about work-queue diagnostics. The resulting body is 343 whitespace-delimited words. |
| Workbook and patch | Regenerate the Member 1/2 workbook, current source references, baseline patch, and checksums from the merged code. Member 3's source and handoff fixes remain preserved. |

## Guidelines check

The original PDF, page 1 and page 4 Part 3A, prohibits changing production behavior just to simplify coverage, permits necessary test registration, and requires separate justification of unavoidable testability edits.

This integration introduces **no additional production C++ or header modifications**. The outstanding source changes relative to fetched `main` are `BatteryTest.cpp`, `LandDetectorTest.cpp`, and the land-detector CMake setting that confines the clock replacement to the functional-test executable. The tests still call the actual production methods.

Earlier submission edits—the estimator and land-detector friend declarations, and the battery/FMM `private` to `protected` changes—are unchanged by this merge. They still require the separate testability justification discussed in the audit; this integration is not a claim that every assignment requirement is complete. The required AI-assistance records are retained.

The evaluated PX4 release remains **v1.17.0, `d6f12ad1c4f70ad3230afd7d86e971421e02fef4`**. The teammate's later repository commit is recorded as the integration base, not mislabeled as the release baseline.

## Fresh verification

- Build of all five student test targets: PASS; both changed test objects and executables rebuilt.
- Normal execution: **78/78 cases PASS** across all five suites. Counts remain Member 1 = 36, Member 2 = 19, Member 3 = 23.
- Land detector: **20 shuffled repetitions PASS**, seeds 2027–2046, 380 case executions.
- Battery: **20 shuffled repetitions PASS**, seeds 2027–2046, 220 case executions.
- MC/DC validator: **34 runtime vectors / 17 D1-D3 independence pairs PASS**. The short-circuit masks are derived from source expressions; this is not a compiler MC/DC percentage or full governing-guard analysis.
- Member 1/2 workbook: **55 cases, 34 matrix rows, exactly two sheets**. Member 3 remains a separate 23-case handoff.
- Regenerated Member 1 and Member 1/2 patches apply to reconstructed release-baseline files and reproduce the submitted files exactly.

Evidence: [execution log](merge_main/normal_execution.log), [five XML files](merge_main/xml), [test summary](merge_main/test_summary.json), [land shuffle](merge_main/functional-LandDetector_shuffle.log), [battery shuffle](merge_main/functional-Battery_shuffle.log), [matrix validation](merge_main/mcdc_validation.json), [commands and source hashes](merge_main/verification_manifest.json), [patch validation](../member1_deliverables/logs/patch_validation.log).

The new isolated coverage capture reports **71/116 lines, 94/192 raw branch edges, and 7/10 functions** for `MulticopterLandDetector.cpp`. The ten ground-effect decision edges remain covered. Compared with the prior local fix's 96/192, two incidental ground-contact edges at source line 211 are no longer executed after removing the extra helper call from D3-B. They belong to the hover-thrust governing guard and remain uncovered obligations; no source or denominator was excluded to improve the percentage. All other five production-file totals match the earlier priority capture.

Current coverage: [Member 2 HTML](../member2_deliverables/coverage/merge_main/html/index.html), [matching Member 2 LCOV](../member2_deliverables/coverage/merge_main/coverage.info), [six-file LCOV](merge_main/coverage.info), [summary](merge_main/coverage_summary.json), [capture log](merge_main/coverage_capture.log). Normal and shuffled counters were isolated; shuffled execution was not merged into reported coverage.

Work-queue diagnostics and the existing analog-battery parameter diagnostic remain visible in the logs. The broader guard MC/DC, battery-fusion/assertion improvements, baseline/gap investigation, and final group consolidation remain outstanding as recorded in the audit.

## Repeat the test checks

From the repository root, using the configured coverage build:

```bash
cmake --build build/px4_sitl_test --target \
  unit-HysteresisAssignment functional-AttitudeEstimatorQ \
  functional-LandDetector functional-Battery functional-FlightModeManager -j 4

merge_run=$(mktemp -d /tmp/sqe-merge-check-XXXXXX)
GCOV_PREFIX="$merge_run/normal/profiles" \
GTEST_OUTPUT="xml:$merge_run/normal/xml/" \
ctest --test-dir build/px4_sitl_test \
  -R '^(unit-HysteresisAssignment|functional-AttitudeEstimatorQ|functional-LandDetector|functional-Battery|functional-FlightModeManager)$' \
  --timeout 30 -V

python3 member2_deliverables/scripts/validate_mcdc.py \
  --xml "$merge_run/normal/xml/functional-LandDetector.xml"

for suite in functional-LandDetector functional-Battery; do
  GCOV_PREFIX="$merge_run/shuffle/profiles" \
    "build/px4_sitl_test/$suite" \
    --gtest_shuffle --gtest_random_seed=2027 --gtest_repeat=20
done
```

Recovery copies were retained: the safety stash `dc58aeaecfa99f1f497fa1ee5c4d0a70733ec29d` and `/tmp/sqe-main-merge-backup-eyybolbw` containing the original tracked patch and 131 local files. The stash has already been applied and reconciled; it should not be applied again to this working tree. The local virtual environment was preserved and is outside the suggested staging paths.
