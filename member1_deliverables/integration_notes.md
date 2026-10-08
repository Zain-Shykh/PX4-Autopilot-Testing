# Workbook and team integration notes

The refreshed Member 1/2 handoff workbook contains **36 Member 1 cases and all 19 current Member 2 cases**, for **55 rows**. The original 15 Member 2 IDs are preserved; four newer cases are assigned TC_M2_16–TC_M2_19. Member 2 source references and PASS statuses now come from the corrected source and fresh XML. The separate Member 3 workbook contains 23 cases; adding those to a single 78-row final group workbook remains an integration task.

The second sheet imports all **34 current MC/DC rows** for Member 2's **17 D1-D3 return-decision pairs**. The workbook builder first runs `member2_deliverables/scripts/validate_mcdc.py` against current XML, so it refuses to publish a matrix that disagrees with passing runtime properties. Review notes explicitly limit the evidence to D1-D3. Full governing-guard MC/DC is not yet established; see Member 2 Part 2.

The workbook has exactly two sheets. Excel forbids `/` in worksheet names, so the second tab is `MC-DC Evidence`. There are no additional scope/defect/coverage sheets.

`px4_assignment2_tests.patch` covers Member 1 plus current Member 2 source/CMake/access changes. Member 3 remains in its separate `member3_tests.patch`; no claim is made that this handoff patch alone reconstructs all three members. The Member 1-only patch remains available.

The previous statements that Member 3 and four Member 2 pairs/tests were absent are superseded. Remaining integration work is the full governing-guard analysis, outstanding test/coverage issues listed in the audit, one consolidated report/workbook/patch, a supported group judgment, group details, and LMS filenames. See [current merge status](../assignment_audit/MAIN_MERGE.md).
