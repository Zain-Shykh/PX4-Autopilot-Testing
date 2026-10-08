# Part 3: Structural Coverage Evidence & Gap Analysis (Scope 3)

## 1. Measured Coverage Results
- **Statement / Line Coverage**: **89.8%** (465 of 518 lines)
- **Function Coverage**: **88.6%** (39 of 44 functions)
- **Baseline Line Coverage**: 0.0% (Uninstrumented baseline capture)

### Detailed File Breakdown
| File | Lines Total | Lines Hit | Line Coverage | Functions Total | Functions Hit | Function Coverage |
| :--- | :---: | :---: | :---: | :---: | :---: | :---: |
| `src/lib/battery/battery.cpp` | 231 | 228 | **98.7%** | 19 | 19 | **100.0%** |
| `src/modules/battery_status/analog_battery.cpp` | 54 | 45 | **83.3%** | 7 | 6 | **85.7%** |
| `src/modules/flight_mode_manager/FlightModeManager.cpp` | 233 | 192 | **82.4%** | 18 | 14 | **77.8%** |
| **Subsystem Total** | **518** | **465** | **89.8%** | **44** | **39** | **88.6%** |

## 2. Uncovered Obligations Justification
See `coverage/uncovered_obligations.csv` for detailed source line references and architectural rationale.
