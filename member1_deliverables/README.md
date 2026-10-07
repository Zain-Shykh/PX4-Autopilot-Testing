# Member 1 deliverables — Assignment 02

Completed on 8 October 2026: attitude-estimator functional tests, hysteresis unit tests, repository/setup analysis, coverage investigation, workbook integration, and test patches. **36 new tests pass** (29 estimator + 7 hysteresis). Member 2's existing 15 tests were also rerun successfully.

Start with [Report.md](Report.md), then open [Testing_Workbook.xlsx](Testing_Workbook.xlsx) and the [final HTML coverage report](coverage/final/html/index.html).

| Artifact | Purpose |
| --- | --- |
| [part1_scope_record.md](part1_scope_record.md) | Baseline, environment, dependencies, and scope |
| [part2_test_derivation.md](part2_test_derivation.md) | Source-derived test basis and boundary selection |
| [part3_coverage_evidence.md](part3_coverage_evidence.md) | Measured baseline/student/final coverage and every gap category |
| [part4_findings.md](part4_findings.md) | Confirmed bias-limit defect and scoped conclusion |
| [Testing_Workbook.xlsx](Testing_Workbook.xlsx) | 51 inventory rows and 26 supplied Member 2 MC/DC rows |
| [sheet1_test_inventory.csv](sheet1_test_inventory.csv) | Editable inventory interchange file |
| [integration_notes.md](integration_notes.md) | Member 2 MC/DC issues and remaining group handoff |
| [AI_ASSISTANCE.md](AI_ASSISTANCE.md) | Material assistance, assumptions, and student review responsibilities |
| [work_plan_member1.md](work_plan_member1.md) | Required individual plan and completion checklist |
| [REPRODUCE.md](REPRODUCE.md) | Exact setup, build, execution, coverage, and packaging commands |
| [member1_tests.patch](member1_tests.patch) | Only Member 1 test-related changes against the required commit |
| [px4_assignment2_tests.patch](px4_assignment2_tests.patch) | Combined Member 1 + existing Member 2 test-related changes |
| `source_files/` | Copies of the five Member 1 source/CMake deliverables, retaining repository paths |
| `coverage/` | Three HTML/LCOV views, XML results, summary JSON, and complete uncovered-obligation CSV |
| `logs/` | Build, execution, shuffled-run, environment, and patch-reconstruction evidence |
| `scripts/` | Reproduction, coverage, analysis, XLSX generation, and patch packaging tools |

The two workbook tabs are `Test Inventory` and `MC-DC Evidence`. Excel forbids `/` in worksheet names, so the latter is the valid equivalent of the assignment's “MC/DC Evidence” label. The workbook contains exactly two sheets.

These are Member 1's completed contributions, not a claim that the entire three-member submission is complete. Member 3's work is absent, and Member 2's full MC/DC claim needs correction. Names, roll numbers, and section were not supplied; fill these in and apply the LMS naming convention to the final team submission.
