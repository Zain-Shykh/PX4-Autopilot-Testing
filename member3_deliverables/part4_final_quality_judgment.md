## 5. Part 4 — Findings, Defensive Behavior & Final Quality Judgment (CLO3)

> **Priority correction status (8 October 2026):** Battery parameters/messages are isolated, all 23 cases pass, and the battery suite passes 20 shuffled repetitions. The fabricated battery MC/DC equation has been withdrawn. Current evidence is in [priority_fix HTML](coverage/priority_fix/html/index.html) and [the fix record](../assignment_audit/PRIORITY_FIX.md). The earlier coverage totals, gap explanations, and broad quality claims below are historical draft material; the audit identifies corrections still required. This is not a completed group submission.

### 5.1 Key Findings & Defect Investigation

1. **uORB Subscription Update Semantics**:
   - *Behavior*: In FlightModeManager::start_flight_task(), _vehicle_status_sub.get() is invoked to read navigation states without an explicit .update() call inside that helper.
   - *Investigation*: If the main FlightModeManager::updateSubscriptions() is not called prior to evaluating flight task requests, the manager operates on uninitialized or stale status structures.
   - *Resolution*: This is an architectural coupling requirement in PX4. We verified that in operational flight loops, Run() always executes updateSubscriptions() at 50 Hz before task switching.

2. **Real-Time Command Freshness Constraint (200 ms Expiration)**:
   - *Behavior*: 	ryApplyCommandIfAny() strictly rejects mode switch commands whose timestamp is > 200 ms in the past.
   - *Verification*: TC_M3_FMM_06 confirmed that stale MAVLink commands (e.g., delayed over a high-latency telemetry link) are safely dropped, preventing delayed unexpected mode changes.

3. **RLS Load Drop Compensation Stability**:
   - *Behavior*: The Recursive Least Squares estimator in Battery dynamically estimates internal battery resistance.
   - *Verification*: TC_M3_BAT_09 verified that rapid current step changes (0A -> 30A) do not cause numerical divergence in the covariance matrix P, correctly calculating load-drop-corrected Open Circuit Voltage.

### 5.2 Final Quality Judgment (300–400 Words)

> **Evidence-Based Quality Assessment of Tested PX4 Flight Control & Safety Modules**
>
> Structural analysis and functional test execution of the PX4 flight control (FlightModeManager) and energy safety (Battery, AnalogBattery) modules demonstrate robust, defensively engineered architectural design. By achieving **81.3% overall line coverage** (including **93.5% line and 100% function coverage** on the core Battery library) across 23 deterministic functional test cases, our verification confirms that safety-critical state transitions, mode switching fallbacks, command freshness validations, and hierarchical battery warning thresholds operate with high fidelity under deterministic inputs. The battery warning cases exercise representative state-of-charge comparisons. They do not establish a compound voltage/SoC MC/DC decision. The group uses the land detector for MC/DC, with corrected D1-D3 pairs and further governing-guard obligations still pending.
>
> Furthermore, targeted testing of internal mathematical filters—such as the Recursive Least Squares (RLS) estimator for dynamic cell resistance and load-drop-corrected Open Circuit Voltage—demonstrated numerical stability and rapid convergence under extreme step-current transients without numerical divergence. Similarly, FlightModeManager displayed consistent defensive behavior by strictly enforcing the 200 ms real-time freshness boundary on external MAVLink vehicle commands and maintaining safe default task fallbacks during navigation state transitions.
>
> However, structural coverage evidence strictly bounds the scope of our quality claim. While the algorithmic and state-machine business logic within the tested components exhibits high reliability, the remaining coverage gaps (such as dynamic FlightTask execution, hardware-level ADC register sampling, and NSH CLI dispatchers) represent boundaries where POSIX unit and functional test harness isolation cannot fully emulate target microcontroller hardware. Furthermore, high structural coverage in isolated functional tests does not guarantee immunity against asynchronous race conditions across high-frequency uORB topics, extreme RTOS scheduling jitter, or sensor estimator divergent states in Gazebo SITL physics simulations.
>
> In conclusion, the structural evidence provides strong confidence that the core decision logic, state debouncing, and safety-critical threshold evaluations of FlightModeManager and Battery are sound, robust, and correctly implemented. Nonetheless, this assurance remains strictly confined to the tested POSIX functional scope and must not be generalized as a claim that the entire PX4 Autopilot firmware is defect-free or fully verified across all embedded flight profiles.

---

