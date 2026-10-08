## 6. Part 5 — AI Assistance Record

In accordance with academic integrity guidelines, this section documents all AI tool interactions utilized during the preparation of this assessment:

1. **Tool Utilized**: Antigravity AI (Google DeepMind Advanced Agentic Coding Engine).
2. **Tasks & Workflows Delegated**:
   - Rapid generation and styling of the binary Testing_Workbook.xlsx using Python openpyxl library.
   - Initial structural scaffolding of GoogleTest fixtures (FlightModeManagerTest.cpp and BatteryTest.cpp).
   - Compilation and formatting of structural coverage metrics from lcov tracefiles.
3. **Student Verification & Validation**:
   - All C++ test assertions, uORB topic mappings, and parameter updates were manually audited and debugged against PX4 source code.
   - Discovered and corrected uORB subscription caching issues and parameter initialization defaults in test fixtures.
   - The earlier battery MC/DC analysis was withdrawn after source review. Current land-detector D1-D3 evidence is maintained by Member 2; full governing-guard analysis remains pending.

Codex assisted with the priority correction: resetting battery parameters and retained message state, making the unknown-capacity premise explicit, reproducing the former seed-2027 failure and checking 20 shuffled repetitions, and withdrawing the battery MC/DC equation that did not exist in the selected source. Remaining assertion and coverage-analysis limitations are recorded in the audit.
