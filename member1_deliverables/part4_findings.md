# Part 4 — Findings and scoped assessment

## Confirmed defect: configured gyro-bias bound never reaches the filter

**Evidence:** `TC_M1_A25`, `AttitudeEstimatorQAssignment.ConfiguredBiasLimitIsNotApplied`, in the final GTest XML/log. In the production source, `_bias_max{}` is initialized to zero at line 155; the `ATT_BIAS_MAX` parameter wrapper is declared at line 171; `update_parameters()` at lines 399–424 never copies it to `_bias_max`; `update()` clamps each learned bias against that zero at line 548. The baseline commit contains the same missing assignment. Parameter metadata describes a gyro-bias limit with default **0.05 rad/s**.

Reproduction sets the actual `ATT_BIAS_MAX` parameter to `0.05`, refreshes production parameters, initializes a level attitude, uses zero gyro rate, a +Y magnetic heading, magnetic weight `1`, bias weight `0.1`, and `dt=0.01`. The parameter store confirms `0.05`, but the internal limit remains zero and the learned Z bias is zero.

Given the declared configurable limit, the expected first update would retain the negative bias correction, approximately `-pi/2 * 0.1 * 0.01 = -0.0015708 rad/s`, which is within that bound. Actual execution clips it to zero. This is not a failed test manufactured for the assignment: the passing **characterization** test explicitly records the observed defect. A test asserting intended corrected behavior would fail on this baseline. The separate saturation test `A11` injects a nonzero internal limit only to evaluate downstream clamp behavior.

No production fix is included, preserving the assignment baseline. A follow-up fix should wire the parameter into `_bias_max`, add a regression asserting the intended learned bias, and review live parameter-change behavior. The present evidence supports the parameter-wiring defect, not a measured flight-level consequence.

## Assessment

The suite supplies reproducible evidence for the selected filter, message validation, and timing logic under synthetic inputs. It checks output values and state changes in addition to counting execution. Hysteresis already had complete line coverage upstream; its new tests add precise deadlines and interruptions that line coverage alone does not distinguish. Estimator tests add actual execution of the complete filter/message-processing line scope and reveal that a passing, broadly covered suite can still characterize an implementation defect.

Remaining risks include scheduler timing, sensor noise, invalid quaternion components beyond the current validation assumptions, inconsistent messages, floating-point extremes, timestamp rollover, and system-level estimator interactions. Source-impossible Boolean edges and normal-operation defensive guards must be distinguished from untested allocation/concurrency failures. The uncovered-obligation inventory preserves that distinction and the full raw denominator.

The next useful steps are a targeted bias-parameter regression after an approved fix, a dependency-failure seam for callback/copy errors, and simulation tests of concurrent messages and estimator startup. These unit/functional results support a scoped assessment of the exercised implementation; they do not certify flight readiness. Member 2's MC/DC and Member 3's control/battery work require their own verified evidence before any consolidated group quality conclusion.
