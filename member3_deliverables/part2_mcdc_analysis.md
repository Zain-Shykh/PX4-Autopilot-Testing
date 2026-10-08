## 3. Structural test derivation and MC/DC scope

The group's selected MC/DC component is `MulticopterLandDetector`. Its corrected D1-D3 return-decision pairs, runtime-vector assertions, and remaining governing-guard obligations are documented in [Member 2's analysis](../member2_deliverables/part2_mcdc_analysis.md). This does not claim complete MC/DC for the full component yet.

The earlier battery equation `(V_cell <= V_crit AND V_cell > 2.0) OR (SoC <= SoC_crit)` was not present in the selected production source and has been withdrawn. Its abstract truth table was not PX4 execution evidence, and no DO-178C compliance claim is made.

In the actual `src/lib/battery/battery.cpp`, `determineWarning()` (307–321) uses successive strict `<` comparisons against state-of-charge emergency, critical, and low thresholds. `updateBatteryStatus()` (144–145) gates warning calculation with `_connected && _battery_initialized`. The current warning-ladder test checks representative outcomes of the former; it does not establish MC/DC of the latter.

The Member 3 workbook contains its 23-test inventory and an empty reserved MC/DC sheet. It is a partial handoff; the final group workbook should contain the validated land-detector matrix. The absence of battery MC/DC is not a separate missing requirement, because the assignment requires one justified critical component.

