# Workbook and team integration notes

Member 1's inventory includes all **36 new Member 1 tests** and the **15 existing Member 2 tests**. Every PASS comes from fresh GTest XML. No upstream tests are counted as student contributions; no rows or statuses are fabricated for absent Member 3 tests. Integration-only IDs `TC_M2_01` through `TC_M2_15` identify the existing Member 2 tests in source order without renaming their code.

The second sheet imports all **26 rows** from `member2_deliverables/sheet2_mcdc_evidence.csv` unchanged in their original eight columns, with a review-status column appended. This preserves provenance; it does not certify the supplied truth vectors or line references. The original Member 2 files are unchanged.

The written Member 2 derivation describes **17 independence pairs**, but its CSV contains **13 pairs**. The missing pairs are **D2 E, D2 F, D2 G, and D3 D**. Corresponding dedicated tests are also absent from `LandDetectorTest.cpp`. Some claimed input vectors require further verification against actual fixture state and short-circuit evaluation; a passing assertion does not prove all stated atomic values were evaluated. Other compound guards within the selected critical behavior also need a completeness review. Consequently, the imported matrix must not be presented as complete MC/DC evidence.

Member 2's prose claims 100% business branch coverage but the supplied extraction command does not explicitly enable branch coverage. Its coverage and final judgment should be reconciled against fresh branch-enabled measurements and the completed matrix before inclusion in a team report. Member 1's scoped coverage does not measure `MulticopterLandDetector.cpp` and cannot repair that claim. The actual Member 2 execution result here is **15 tests PASS**, which is a narrower claim.

The workbook has exactly two sheets. Because `/` is an invalid Excel worksheet-name character, the MC/DC tab uses `MC-DC Evidence`. All required condition/pair/source columns are retained, and there are no extra coverage, defect, or scope sheets.

The combined patch includes Member 1 and existing Member 2 test/CMake/access changes, excluding pre-existing EKF CSV changes and unrelated assignment documents. Member 3's tests are not present and are not represented. The Member 1-only patch supports independent handoff.

Before the final three-member submission: complete/revalidate Member 2's MC/DC evidence, add Member 3's executed inventory and coverage, consolidate the report and final 300–400-word group judgment, fill group names/roll numbers/section, and apply the required LMS filename convention. These are team integration tasks, not unfinished Member 1 module tests.
