# Part 2: Decision Coverage & Logic Derivations (Scope 3)

## 1. Governing Decisions in Scope 3

### Decision 1: Battery Connection Status Gating
- **Expression**: `connected = (voltage_v > 2.1V) && (cell_count > 0)`
- **Conditions**:
  - $A$: Pack voltage exceeds recognition threshold ($V > 2.1	ext{V}$)
  - $B$: Pack cell count is strictly positive ($N_{	ext{cells}} > 0$)
- **Test Cases**: `TC-M3-BAT-02` ($A=	ext{False}, B=	ext{True} \implies 	ext{Disconnected}$) vs `TC-M3-BAT-03` ($A=	ext{True}, B=	ext{True} \implies 	ext{Connected}$).

### Decision 2: Static Resistance vs RLS Estimator Branching
- **Expression**: `use_configured_resistance = (_params.r_internal >= 0.0f)`
- **Conditions**:
  - $A$: Configured internal resistance parameter is non-negative
- **Test Cases**: `TC-M3-BAT-12` ($A=	ext{True} \implies 	ext{Static drop } V_{	ext{ocv}} = V + I \cdot R_{	ext{int}}$) vs `TC-M3-BAT-07` ($A=	ext{False} \implies 	ext{Dynamic 2-state RLS filter}$).

### Decision 3: Transition Mode Gating
- **Expression**: `is_transition_active = (in_transition_mode && flag_control_altitude_enabled)`
- **Conditions**:
  - $A$: Vehicle reports transition mode active (`in_transition_mode == true`)
  - $B$: Altitude control is enabled (`flag_control_altitude_enabled == true`)
- **Test Cases**: `TC-M3-FMM-13` ($A=	ext{True}, B=	ext{True} \implies 	ext{FlightTaskIndex::Transition}$).

### Decision 4: Strict Command Freshness Boundary
- **Expression**: `is_command_fresh = (hrt_absolute_time() < cmd.timestamp + 200_ms)`
- **Conditions**:
  - $A$: Elapsed time since command publication is strictly less than 200 ms ($t_{	ext{now}} - t_{	ext{cmd}} < 200\,	ext{ms}$)
- **Test Cases**: `TC-M3-FMM-10` evaluates boundaries at $\Delta t = 190\,	ext{ms}$ (Applied), $\Delta t = 200\,	ext{ms}$ (Rejected), $\Delta t = 210\,	ext{ms}$ (Rejected).
