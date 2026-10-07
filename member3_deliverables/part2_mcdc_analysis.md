## 3. Part 2 — Structural Test Derivation & MC/DC Analysis (CLO2)

To satisfy DO-178C Level A avionics safety verification standards, compound boolean decisions within the flight software were analyzed for **Modified Condition / Decision Coverage (MC/DC)**.

### 3.1 Selected Critical Decision Analysis
From `src/lib/battery/battery.cpp`, we selected the safety-critical battery warning escalation decision:

**Decision Equation**: D = (A and B) or C

Where:
- **Condition A**: Terminal Cell Voltage below Critical Threshold (V_cell <= V_crit)
- **Condition B**: Valid Physical Voltage Plausibility Check (V_cell > 2.0 V)
- **Condition C**: State of Charge below Critical Threshold (SoC <= SoC_crit)

### 3.2 Truth Table & Decision Outcomes (2^3 = 8 Combinations)

| Test Vector | Condition A (V <= V_crit) | Condition B (V > 2.0V) | Condition C (SoC <= SoC_crit) | Compound Term (A and B) | Decision Outcome D = (A and B) or C |
| :---: | :---: | :---: | :---: | :---: | :---: |
| **TV-01** | True | True | True | True | **True** (Critical Warning) |
| **TV-02** | True | True | False | True | **True** (Critical Warning) |
| **TV-03** | True | False | True | False | **True** (Critical Warning) |
| **TV-04** | True | False | False | False | **False** (No Warning) |
| **TV-05** | False | True | True | False | **True** (Critical Warning) |
| **TV-06** | False | True | False | False | **False** (No Warning) |
| **TV-07** | False | False | True | False | **True** (Critical Warning) |
| **TV-08** | False | False | False | False | **False** (No Warning) |

---

### 3.3 MC/DC Independence Pair Derivation

To prove independence, each condition must be shown to independently affect the decision outcome while all other conditions remain fixed:

#### 1. Independence Pair for Condition A (V <= V_crit)
- **Vector Pair**: (TV-02, TV-06)
- **Fixed Conditions**: B = True, C = False
- **Variation**:
  - TV-02: A = True => D = (True and True) or False = True
  - TV-06: A = False => D = (False and True) or False = False
- **Result**: Condition A independently controls decision D.

#### 2. Independence Pair for Condition B (V > 2.0V)
- **Vector Pair**: (TV-02, TV-04)
- **Fixed Conditions**: A = True, C = False
- **Variation**:
  - TV-02: B = True => D = (True and True) or False = True
  - TV-04: B = False => D = (True and False) or False = False
- **Result**: Condition B independently controls decision D.

#### 3. Independence Pair for Condition C (SoC <= SoC_crit)
- **Vector Pair**: (TV-06, TV-05)
- **Fixed Conditions**: A = False, B = True
- **Variation**:
  - TV-06: C = False => D = (False and True) or False = False
  - TV-05: C = True => D = (False and True) or True = True
- **Result**: Condition C independently controls decision D.

---

