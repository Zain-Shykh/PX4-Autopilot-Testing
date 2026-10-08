# Member 2 coverage evidence after main integration

The current 19-test suite passes in normal order and in 20 shuffled repetitions (seeds 2027–2046). The 34 runtime operand vectors and all 17 D1-D3 pairs pass `scripts/validate_mcdc.py`. This establishes the documented return-decision pairs, not complete MC/DC of every governing decision in the selected component.

Fresh isolated, branch-enabled measurement: **71/116 lines (61.2%), 94/192 raw branches (49.0%), and 7/10 functions** in `MulticopterLandDetector.cpp`. The ground-effect return decision covers **10/10 raw branches**. The merged D3-B test uses Member 2's direct cached-flag setter with the local runtime assertions. Removing the incidental ground-contact call means two raw edges at line 211 are no longer covered by that test; they remain explicit obligations in the hover-thrust governing guard. The former priority-run total of 96/192 is historical. The earlier claims of 100% line/branch coverage within all five decision methods and exclusively infrastructural gaps are withdrawn.

Evidence:

- [Normal execution and all 78 group cases](logs/merge_main/normal_execution.log)
- [Current 19-case XML with runtime vectors](logs/merge_main/functional-LandDetector.xml)
- [20 shuffled repetitions](logs/merge_main/functional-LandDetector_shuffle.log)
- [MC/DC validation and source-derived short-circuit masks](logs/merge_main/mcdc_validation.json)
- [Generated current HTML](coverage/merge_main/html/index.html) and [LCOV](coverage/merge_main/coverage.info)

Unexecuted business-method lines still include 182, 189, 199, 227, 229, 230, 234, 235, and 262: vertical derivative fallback, unavailable position, commanded-descent processing, landed-state gating, and climb-rate-mode thrust computation. These are not justified exclusions. The remaining compound guards are explicitly listed in [Part 2](part2_mcdc_analysis.md). `_update_params()` includes threshold adjustment logic and cannot be dismissed as an inaccessible parameter daemon; functional GTest supports parameters and local uORB. Constructor initialization is exercised by the fixture.

This priority correction does not complete the full gap investigation or supply a comparable baseline coverage capture. Those remain required before final submission. Raw GCC edges include compiler-generated control flow and should not be relabeled as source-decision or MC/DC percentages.

Build with `cmake -S . -B build/px4_sitl_test -G Ninja -DCONFIG=px4_sitl_test -DCMAKE_BUILD_TYPE=Coverage`, then build `functional-LandDetector`. The clock replacement is linked only into that test executable. [Merged-run verification notes](../assignment_audit/MAIN_MERGE.md) record the execution and isolated coverage procedure. Use explicit branch collection; `make tests` alone does not establish the build type on a fresh checkout.
