# AI-assistance record

Codex materially assisted with reading the assignment and work distribution; inspecting production code and upstream conventions; designing and writing all new Member 1 test code and its friend/CMake seam; building and debugging the fixture; running tests and coverage tools; interpreting gaps; investigating the ignored gyro-bias parameter; generating the inventory/XLSX, reports, reproduction tools, and patches; and identifying inconsistencies in the supplied Member 2 matrix. These files are AI-assisted original contributions for the assignment, not a claim of unaided student authorship.

Important assumptions and adjustments:

- The specified full commit hash is authoritative locally because the release tag is absent. Existing Member 2 work was preserved.
- Actual parameter/uORB dependencies justify functional GTest for the estimator despite the distribution's proposed unit label. Hysteresis remains a unit suite.
- A test-only clock and disabled autosave make exact freshness/deadline cases reproducible without a scheduler; this does not simulate real scheduling.
- Existing upstream hysteresis tests establish a focused baseline but are not new contributions. Zero-count estimator data remains in that baseline.
- The initial workbook imported an unverified Member 2 matrix. After the priority correction, its builder validates all 34 D1-D3 rows against runtime XML; complete governing-guard MC/DC and Member 3 inventory integration remain pending.
- `MC-DC Evidence` replaces the invalid Excel tab name `MC/DC Evidence`.
- Names, roll numbers, and section are left for the group to supply.
- The bias-limit test characterizes a verified production defect; passing it does not endorse that behavior as correct.

Students remain responsible for checking source-to-test reasoning, understanding every assertion and denominator, reviewing the reported defect and limitations, reproducing results on their own system, and defending or modifying tests during the viva. No external messages, publication, Git commits, pushes, or production-behavior fixes were performed.
