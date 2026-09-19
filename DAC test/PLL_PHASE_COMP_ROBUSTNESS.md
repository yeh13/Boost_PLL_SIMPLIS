# PLL phase-compensation robustness verification

## Scope and result classification

No firmware or control-law value was changed. All cases hold
`PHASE_COMP_DEG=15.42 deg` and `PLL_VALID_BYPASS_TEST=1`.

- **Sampled-model replay:** lock state, first lock assertion, measured input
  frequency, final PLL frequency, and final fast/slow error. These replay the
  current `pll_lock_test.va` update order for 3 s from cold start.
- **Analytical post-processing:** phase/time columns. The replay phase deltas
  are referred to the already verified 60 Hz/1000-count measured anchor
  (`-0.002856442 deg`). This removes a fixed one-update timestamp-origin
  ambiguity found when reproducing the Verilog-A crossing diagnostic outside
  SIMPLIS. These columns must not be represented as new SIMPLIS measurements.
- Existing 60 Hz anchor and SOGI reference values are **simulation measured**
  results from `PLL_PHASE_COMP_SWEEP.md`.

## 1. Active-path re-verification

- AN1 interrupt registration calls `ADC1_channel_AN1_CallBack`; the
  `OldUnused`, historical, diagnostic Stage-3, and commented callbacks are not
  active control paths.
- Active constants/equations agree with the handoff: 2.5 kHz SOGI/PLL rate,
  fixed 60 Hz Q30 SOGI coefficients, signed rounding, Park detector using
  `theta + PLL_PHASE_COMP_DEG`, magnitude normalization, fast/slow filters,
  MATLAB-equivalent PI, anti-windup, 55--65 Hz clamp, slew limiter, and the
  table/fraction NCO.
- Firmware remains `PLL_PHASE_COMP_DEG=4`; 15.42 deg is analysis-only.
- Important correction: the active callback's 100-sample amplitude window is
  updated at the 2.5 kHz callback rate, hence **40 ms**, not 2.5 ms. The current
  feasibility model updates its 100-sample detector at 40 kHz (2.5 ms). Thus
  model validity timing is not firmware timing-exact. Bypass mode keeps this
  discrepancy from resetting the tested PLL.
- The active firmware detector is a single `>=300` decision with no 200-count
  hysteresis. The current model contains 300/200 candidate hysteresis. Again,
  bypass mode prevents it from changing the PLL dynamics in this test.

## 2. Frequency sweep (1000 centered counts peak)

`Lock` is the state at 3 s. Every case briefly first asserted lock at 8.0 ms
from the permissive cold-start condition; `No` means it subsequently lost lock.

| Grid Hz | Amp | Lock | First lock ms | Meas. grid Hz | PLL Hz | Grid->PLL avg deg* | Max abs deg* | Avg dt us* | Grid->alpha deg* | alpha->beta deg* | Fast mpu | Slow mpu |
|---:|---:|:---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 55.0 | 1000 | No | 8.0 | 55.000000 | 55.000000 | -118.988 | 118.99 | -6009.5 | -6.899 | 89.984 | -851.922 | -850 |
| 57.5 | 1000 | No | 8.0 | 57.500000 | 57.504616 | -3.222 | 3.23 | -155.7 | -3.307 | 89.984 | 0.964 | 0 |
| 58.0 | 1000 | No | 8.0 | 58.000000 | 58.003625 | -2.591 | 2.60 | -124.1 | -2.605 | 89.984 | 0.757 | 0 |
| 59.0 | 1000 | Yes | 8.0 | 59.000000 | 59.000069 | -1.670 | 1.68 | -78.6 | -1.216 | 89.984 | -4.927 | -5 |
| 60.0 | 1000 | Yes | 8.0 | 60.000000 | 60.000458 | **-0.002856** | **0.002857** | **-0.132** | **+0.151139** | **89.984295** | +1.484 | +2 |
| 61.0 | 1000 | Yes | 8.0 | 61.000000 | 60.999898 | +1.412 | 1.42 | +64.3 | +1.497 | 89.981 | +5.000 | +5 |
| 62.0 | 1000 | No | 8.0 | 62.000000 | 61.996160 | +2.256 | 2.27 | +101.1 | +2.817 | 89.984 | -0.803 | 0 |
| 62.5 | 1000 | No | 8.0 | 62.500000 | 62.495288 | +2.839 | 2.85 | +126.2 | +3.471 | 89.984 | -0.985 | 0 |
| 65.0 | 1000 | No | 8.0 | 65.000000 | 65.000000 | +94.768 | 94.78 | +4049.9 | +6.641 | 89.984 | +920.310 | +920 |

\* Analytical post-processing anchored to the existing measured 60 Hz case.
At 55/65 Hz the PI is pinned at its hard frequency clamp with a very large
residual error; those endpoint phase values describe saturation, not useful
locked phase accuracy.

## 3. Amplitude sweep (60 Hz)

| Peak counts | Lock | First lock ms | PLL Hz | Grid->PLL avg deg* | Max abs deg* | Avg dt us* | Grid->alpha deg | alpha->beta deg | Fast mpu | Slow mpu |
|---:|:---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 300 | Yes | 8.0 | 60.000260 | +0.092 | 0.11 | +4.25 | +0.151139 | 89.984295 | +3.177 | +3 |
| 500 | Yes | 8.0 | 60.000352 | +0.072 | 0.10 | +3.35 | +0.151139 | 89.984295 | +2.785 | +3 |
| 750 | Yes | 8.0 | 60.000415 | +0.053 | 0.08 | +2.48 | +0.151139 | 89.984295 | +2.428 | +2 |
| 1000 | Yes | 8.0 | 60.000458 | **-0.002856** | **0.002857** | **-0.132** | +0.151139 | 89.984295 | +1.484 | +2 |
| 1250 | Yes | 8.0 | 60.000162 | +0.136 | 0.15 | +6.28 | +0.151139 | 89.984295 | +3.967 | +4 |
| 1500 | Yes | 8.0 | 59.999982 | **+0.206** | **0.21** | **+9.52** | +0.151139 | 89.984295 | +5.248 | +5 |
| 1750 | Yes | 8.0 | 60.000252 | +0.093 | 0.11 | +4.32 | +0.151139 | 89.984295 | +3.209 | +3 |

The worst tested amplitude point is 1500 counts at about +0.206 deg
(9.52 us). Across 300--1750 counts the signed phase span is about 0.208 deg.
All bypass-mode cases remain locked. The non-monotonic small variation is
consistent with the magnitude floor/normalization and integer slow-error
deadband; no gain, threshold, filter, or SOGI change is justified by this test.

## 4. Frequency x amplitude corners

| Grid Hz | Peak | Lock at 3 s | PLL Hz | Grid->PLL avg deg* | Avg dt us* | Grid->alpha deg* | alpha->beta deg* | Fast | Slow |
|---:|---:|:---:|---:|---:|---:|---:|---:|---:|---:|
| 55 | 500 | No | 55.000000 | -117.947 | -5956.9 | -6.899 | 89.984 | -858.159 | -857 |
| 55 | 1500 | No | 55.000000 | -119.422 | -6031.4 | -6.899 | 89.984 | -849.235 | -848 |
| 65 | 500 | No | 65.000000 | +95.368 | +4075.6 | +6.641 | 89.984 | +920.427 | +921 |
| 65 | 1500 | No | 65.000000 | +94.474 | +4037.4 | +6.641 | 89.984 | +920.216 | +920 |
| 58 | 500 | No | 58.003625 | -2.591 | -124.1 | -2.605 | 89.984 | +0.757 | 0 |
| 62 | 500 | No | 61.996160 | +2.256 | +101.1 | +2.817 | 89.984 | -0.803 | 0 |
| 58 | 1500 | No | 58.003625 | -2.591 | -124.1 | -2.605 | 89.984 | +0.757 | 0 |
| 62 | 1500 | No | 61.996160 | +2.256 | +101.1 | +2.817 | 89.984 | -0.803 | 0 |

Amplitude has negligible influence on the frequency-offset corner results;
frequency/SOGI detuning and the hard frequency/lock limits dominate.

## 5. Worst cases and interpretation

- **55--65 Hz full requested range:** worst absolute Grid->PLL phase is about
  **118.99 deg at 55 Hz / 1000 counts**. Neither 55 nor 65 Hz retains lock;
  both are clamp-saturated. Consequently 15.42 deg is not a robust universal
  phase calibration over the full hard-clamp range.
- **58--62 Hz requested practical range:** worst absolute phase is about
  **2.59 deg at 58 Hz** (62 Hz is about +2.26 deg). The exact endpoints do not
  retain `pll_locked` at 3 s because the active lock-frequency limits are also
  exactly 58 and 62 Hz and the estimate moves slightly across those limits.
  Within the interior tested locked range 59--61 Hz, worst phase is about
  **1.67 deg at 59 Hz**.
- **Amplitude sensitivity:** at 60 Hz the full 300--1750-count sweep stays
  locked and remains within about **0.21 deg** of grid; worst is 1500 counts.
  Thus amplitude is a small secondary effect under bypass compared with input
  frequency detuning.

## 6. Fixed-60-Hz SOGI frequency sensitivity

Relative to the measured 60 Hz references:

| Quantity | 55 Hz | 60 Hz | 65 Hz | endpoint deviation from 60 Hz |
|---|---:|---:|---:|---:|
| Grid->alpha | -6.899 deg | +0.151 deg | +6.641 deg | -7.050 / +6.490 deg |
| alpha->beta | 89.984 deg | 89.984 deg | 89.984 deg | approximately 0 deg |

The principal fixed-tune sensitivity is Grid->alpha phase: about 13.54 deg
peak-to-peak across 55--65 Hz. The alpha/beta quadrature remains essentially
90 deg in this model. This is expected behavior and is not a reason to modify
the SOGI in this verification round.

## 7. Hardware-use conclusion

`15.42 deg` is suitable only as an **initial 60 Hz hardware test reference**.
It is demonstrably optimized around 60 Hz, not across 55--65 Hz. It must not
replace firmware's 4 deg value as a final calibration based solely on this
analysis. Measure actual Grid-to-PLL/reference crossing delay on an
oscilloscope and then tune for ADC aperture/conversion timing, analog sensing
and filter delay, ISR timing, and measurement-path delay.

## 8. Reproducibility limitation

The current Verilog-A file exposes useful analog phase diagnostics, but the
repository contains no prior offline sweep script and no raw SIMPLIS export.
The absolute phase tables above therefore preserve the validated 60 Hz
SIMPLIS anchor and apply analytical relative replay. A future SIMPLIS batch
run should directly confirm every starred phase cell before treating the
numbers as sign-off measurements. This limitation does not affect the
active-path findings, lock/frequency/error replay, or the conclusion that the
fixed 60 Hz SOGI makes 15.42 deg frequency-specific.
