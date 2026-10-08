# Part 2 — Structural Test Derivation & MC/DC Analysis
## Designated Critical Component: `src/modules/land_detector/MulticopterLandDetector.cpp`

---

## 1. Safety-Critical Component Justification

`MulticopterLandDetector` is selected as the team's designated safety-critical component for Modified Condition/Decision Coverage (MC/DC) analysis. 

The land detector directly governs flight state transitions, disarming signals, and failsafe modes. A defect in its compound decision logic can lead directly to in-flight motor shutoff (false positive land state) or uncontrolled motor spooling on ground impact (false negative land state).

---

## 2. Decision D1: Ground Contact State (`_get_ground_contact_state()`)

### 2.1 Compound Decision Expression
Located at `MulticopterLandDetector.cpp:249`:
```cpp
return !_armed ||
       (_close_to_ground_or_skipped_check && ground_contact
        && !_horizontal_movement && !_vertical_movement);
```

### 2.2 Boolean Decision Formula
$$D_1 = A \lor (B \land C \land D \land E)$$

### 2.3 Atomic Condition Definitions
- **Condition A**: `!_armed` (Vehicle is unarmed)
- **Condition B**: `_close_to_ground_or_skipped_check` (Close to ground or check skipped)
- **Condition C**: `ground_contact` (Low throttle setpoint and descent commanded)
- **Condition D**: `!_horizontal_movement` (Horizontal velocity below threshold $V_{xy} < V_{xy,\text{max}}$)
- **Condition E**: `!_vertical_movement` (Vertical velocity below threshold $V_z < V_{z,\text{max}}$)

### 2.4 MC/DC Independence Pairs Proof for Decision D1

| Condition | Test Pair | Vector $T_1 = [A,B,C,D,E]$ | Outcome $D_1$ | Vector $T_2 = [A,B,C,D,E]$ | Outcome $D_1$ | Independence Proof |
| --- | --- | --- | --- | --- | --- | --- |
| **A** | (TP_D1_A1, TP_D1_A2) | $[\mathbf{T}, F, F, F, F]$ | **True** | $[\mathbf{F}, F, F, F, F]$ | **False** | Varying $A$ independently flips $D_1$. |
| **B** | (TP_D1_B1, TP_D1_B2) | $[F, \mathbf{T}, T, T, T]$ | **True** | $[F, \mathbf{F}, T, T, T]$ | **False** | Varying $B$ while $A=F, C=T, D=T, E=T$ flips $D_1$. |
| **C** | (TP_D1_C1, TP_D1_C2) | $[F, T, \mathbf{T}, T, T]$ | **True** | $[F, T, \mathbf{F}, T, T]$ | **False** | Varying $C$ while $A=F, B=T, D=T, E=T$ flips $D_1$. |
| **D** | (TP_D1_D1, TP_D1_D2) | $[F, T, T, \mathbf{T}, T]$ | **True** | $[F, T, T, \mathbf{F}, T]$ | **False** | Varying $D$ while $A=F, B=T, C=T, E=T$ flips $D_1$. |
| **E** | (TP_D1_E1, TP_D1_E2) | $[F, T, T, T, \mathbf{T}]$ | **True** | $[F, T, T, T, \mathbf{F}]$ | **False** | Varying $E$ while $A=F, B=T, C=T, D=T$ flips $D_1$. |

---

## 3. Decision D2: Maybe-Landed State (`_get_maybe_landed_state()`)

### 3.1 Compound Decision Expression
Located at `MulticopterLandDetector.cpp:287`:
```cpp
return !_armed ||
       (minimum_thrust_now && !_freefall_hysteresis.get_state() && !_rotational_movement
        && ((vertical_estimate && _ground_contact_hysteresis.get_state())
            || (!vertical_estimate && _minimum_thrust_8s_hysteresis.get_state())));
```

### 3.2 Boolean Decision Formula
$$D_2 = A \lor \Big(B \land C \land D \land \big((E \land F) \lor (\neg E \land G)\big)\Big)$$

### 3.3 Atomic Condition Definitions
- **Condition A**: `!_armed` (Vehicle is unarmed)
- **Condition B**: `minimum_thrust_now` (Thrust setpoint below minimum threshold)
- **Condition C**: `!_freefall_hysteresis.get_state()` (Not in freefall)
- **Condition D**: `!_rotational_movement` (Angular rate below threshold $\omega_{xy} < \omega_{\text{max}}$)
- **Condition E**: `vertical_estimate` (Local position timestamp & vertical velocity valid)
- **Condition F**: `_ground_contact_hysteresis.get_state()` (Ground contact state active)
- **Condition G**: `_minimum_thrust_8s_hysteresis.get_state()` (Low thrust maintained for $>8\text{ s}$ when vertical estimate unobservable)

### 3.4 MC/DC Independence Pairs Proof for Decision D2

| Condition | Test Pair | Vector $T_1 = [A,B,C,D,E,F,G]$ | Outcome $D_2$ | Vector $T_2 = [A,B,C,D,E,F,G]$ | Outcome $D_2$ | Independence Proof |
| --- | --- | --- | --- | --- | --- | --- |
| **A** | (TP_D2_A1, TP_D2_A2) | $[\mathbf{T}, F, F, F, F, F, F]$ | **True** | $[\mathbf{F}, F, F, F, F, F, F]$ | **False** | Varying $A$ independently flips $D_2$. |
| **B** | (TP_D2_B1, TP_D2_B2) | $[F, \mathbf{T}, T, T, T, T, F]$ | **True** | $[F, \mathbf{F}, T, T, T, T, F]$ | **False** | Varying $B$ flips $D_2$. |
| **C** | (TP_D2_C1, TP_D2_C2) | $[F, T, \mathbf{T}, T, T, T, F]$ | **True** | $[F, T, \mathbf{F}, T, T, T, F]$ | **False** | Varying $C$ flips $D_2$. |
| **D** | (TP_D2_D1, TP_D2_D2) | $[F, T, T, \mathbf{T}, T, T, F]$ | **True** | $[F, T, T, \mathbf{F}, T, T, F]$ | **False** | Varying $D$ flips $D_2$. |
| **E** | (TP_D2_E1, TP_D2_E2) | $[F, T, T, T, \mathbf{F}, F, T]$ | **True** | $[F, T, T, T, \mathbf{T}, F, T]$ | **False** | Varying $E$ while $F=F, G=T$ flips $D_2$. |
| **F** | (TP_D2_F1, TP_D2_F2) | $[F, T, T, T, T, \mathbf{T}, F]$ | **True** | $[F, T, T, T, T, \mathbf{F}, F]$ | **False** | Varying $F$ when $E=T, G=F$ flips $D_2$. |
| **G** | (TP_D2_G1, TP_D2_G2) | $[F, T, T, T, F, F, \mathbf{T}]$ | **True** | $[F, T, T, T, F, F, \mathbf{F}]$ | **False** | Varying $G$ when $E=F, F=F$ flips $D_2$. |

---

## 4. Decision D3: Ground Effect State (`_get_ground_effect_state()`)

### 4.1 Compound Decision Expression
Located at `MulticopterLandDetector.cpp:300`:
```cpp
return (_in_descend && !_horizontal_movement) ||
       (_below_gnd_effect_hgt && _takeoff_state == takeoff_status_s::TAKEOFF_STATE_FLIGHT) ||
       _takeoff_state == takeoff_status_s::TAKEOFF_STATE_RAMPUP;
```

### 4.2 Boolean Decision Formula
$$D_3 = (A \land B) \lor (C \land D) \lor E$$

### 4.3 Atomic Condition Definitions
- **Condition A**: `_in_descend` (Commanded descent)
- **Condition B**: `!_horizontal_movement` (Horizontal velocity below threshold)
- **Condition C**: `_below_gnd_effect_hgt` (Distance to bottom $< \text{LNDMC\_ALT\_GND\_EFFECT}$)
- **Condition D**: `_takeoff_state == TAKEOFF_STATE_FLIGHT` (Takeoff state is FLIGHT)
- **Condition E**: `_takeoff_state == TAKEOFF_STATE_RAMPUP` (Takeoff state is RAMPUP)

### 4.4 MC/DC Independence Pairs Proof for Decision D3

| Condition | Test Pair | Vector $T_1 = [A,B,C,D,E]$ | Outcome $D_3$ | Vector $T_2 = [A,B,C,D,E]$ | Outcome $D_3$ | Independence Proof |
| --- | --- | --- | --- | --- | --- | --- |
| **A** | (TP_D3_A1, TP_D3_A2) | $[\mathbf{T}, T, F, F, F]$ | **True** | $[\mathbf{F}, T, F, F, F]$ | **False** | Varying $A$ while $B=T, C=F, E=F$ flips $D_3$. |
| **B** | (TP_D3_B1, TP_D3_B2) | $[T, \mathbf{T}, F, F, F]$ | **True** | $[T, \mathbf{F}, F, F, F]$ | **False** | Varying $B$ while $A=T, C=F, E=F$ flips $D_3$. |
| **C** | (TP_D3_C1, TP_D3_C2) | $[F, F, \mathbf{T}, T, F]$ | **True** | $[F, F, \mathbf{F}, T, F]$ | **False** | Varying $C$ while $A=F, D=T, E=F$ flips $D_3$. |
| **D** | (TP_D3_D1, TP_D3_D2) | $[F, F, T, \mathbf{T}, F]$ | **True** | $[F, F, T, \mathbf{F}, F]$ | **False** | Varying $D$ while $A=F, C=T, E=F$ flips $D_3$. |
| **E** | (TP_D3_E1, TP_D3_E2) | $[F, F, F, F, \mathbf{T}]$ | **True** | $[F, F, F, F, \mathbf{F}]$ | **False** | Varying $E$ while $A=F, C=F$ flips $D_3$. |
