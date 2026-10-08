# Assignment 02 — Member 1 technical report

**Course:** SE3002, Structural Testing and Coverage Analysis of PX4 Autopilot  
**Session date:** 8 October 2026  
**Group names / roll numbers / section:** to be supplied by the group  
**Required baseline commit:** `d6f12ad1c4f70ad3230afd7d86e971421e02fef4`

Member 1's work is complete: 29 new estimator functional tests and 7 new hysteresis unit tests pass. The deliverables include source/CMake changes, actual execution evidence, baseline/student/final HTML and LCOV coverage, an integrated two-sheet XLSX, reproducible patches, and an AI-assistance record. This is the Member 1 report for team consolidation, not the final consolidated three-member report.

Read the following sections in order:

1. [Repository, environment, and structural scope](part1_scope_record.md)
2. [Test derivation and source-based obligations](part2_test_derivation.md)
3. [Execution, measured coverage, and every remaining gap](part3_coverage_evidence.md)
4. [Confirmed defect and scoped assessment](part4_findings.md)
5. [Workbook and team integration notes](integration_notes.md)
6. [AI assistance and assumptions](AI_ASSISTANCE.md)

The complete estimator file reaches **199/225 lines and 347/556 raw GCC branch edges**; its core filter/message methods reach **184/184 lines**. Hysteresis reaches **22/22 lines and 17/20 branch edges**, with all three missing edges proven Boolean-infeasible. Raw branch totals retain compiler exception edges. These figures are intentionally narrower than a claim of complete estimator branch coverage or autopilot safety.

The concrete finding is that `ATT_BIAS_MAX` never updates the internal zero-valued bias limit; a dedicated passing characterization test reproduces the resulting suppression of learned bias. Production behavior is unchanged. The refreshed workbook includes all 19 Member 2 cases and 34 runtime-checked D1-D3 matrix rows; additional governing guards remain to be analyzed. All evidence and commands are indexed in [README.md](README.md) and [REPRODUCE.md](REPRODUCE.md).
