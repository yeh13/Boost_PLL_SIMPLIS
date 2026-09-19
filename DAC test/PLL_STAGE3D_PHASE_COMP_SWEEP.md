# Active Stage-3D Direct SIMPLIS Verification and Phase-Compensation Sweep

## Status and scope

**Status: BASELINE CROSS-CHECK BLOCKED; compensation sweep was not run.**

The requested direct SIMPLIS baseline was run for 1.000 s with a cold start,
60 Hz input, 1000 centered-count peak, and `PHASE_COMP_DEG=4`. The model was
`D:\Desktop\DAC test\pll_stage3d_active_test.va`; the legacy
`pll_lock_test.va` was not used as active truth.

Timing/readiness/frequency results agree with the offline integer replay, but
the absolute Grid-to-alpha and Grid-to-PLL phase results do not. Per the test
gate, no coarse or fine compensation sweep was started.

No firmware source, `adc1.c`, current-loop, PWM, MOSFET, SSR, startup, or
control parameter was modified.

## Direct SIMPLIS baseline versus offline integer replay

SIMPLIS source: transient analog outputs printed every 25 us from a 1 s run.
The reported analog phases below are 60 Hz fundamental least-squares fits over
0.5--1.0 s. Assertion times are the first printed 5 V debug state. The same
waveforms were also inspected for rising-zero-crossing timing.

| Measurement | Direct SIMPLIS | Offline integer replay | Difference / result |
|---|---:|---:|---|
| `input_valid` assertion | 32.025 ms | about 32.0 ms | PASS; one 25 us print interval |
| `phase_initialized` assertion | 33.625 ms | about 33.6 ms | PASS; one 25 us print interval |
| Final PLL frequency | 59.999943 Hz | 59.998894 Hz | PASS; +0.001049 Hz |
| Final fast error | -0.820 count | -1 count | PASS; expected real/integer quantization difference |
| Grid to SOGI alpha | -4.744152 deg | +0.1728 deg | **FAIL / measurement-model mismatch** |
| SOGI alpha to beta | +89.999602 deg | +90.02666 deg | PASS; -0.02706 deg |
| Grid to PLL output | -0.041826 deg | -4.49794 deg | **FAIL / measurement-model mismatch** |

The direct analog rising-crossing inspection is consistent with sampled/held
outputs changing only at the 2.5 kHz update instants; it does not support
treating the replay's discrete-sample phase values as continuous analog pin
phase without an explicit timestamp convention.

## Baseline mismatch diagnosis

The SIMPLIS run itself completed normally. The failure is not a missing model,
legacy model selection, failure to initialize, or lack of convergence.

1. The Verilog-A model updates alpha, beta, PLL, frequency and debug outputs at
   2.5 kHz and exposes them as sample-and-hold analog voltages.
2. A held 2.5 kHz waveform has an average zero-order-hold delay of half a
   sample. At 60 Hz this is `360 * 60 * (0.4 ms / 2) = 4.32 deg`.
3. The firmware SOGI coefficients evaluated as a discrete transfer function at
   60 Hz give alpha phase about -0.15375 deg and alpha-minus-beta exactly
   90 deg before integer rounding and output-hold effects.
4. The approximately one-sample-scale disagreement between the two phase
   reports therefore points to incompatible timestamp/output conventions:
   offline replay phase was derived from discrete samples, while direct
   SIMPLIS analog fitting sees the held waveform. The Verilog-A model also uses
   real arithmetic, whereas firmware uses Q30/Q12 rounding, so it is not yet a
   timing-exact phase oracle.
5. The complementary phase shifts are especially diagnostic: Grid-to-alpha is
   about 4.92 deg more negative than replay while Grid-to-PLL is about 4.46 deg
   more positive. Changing compensation now would hide this interface mismatch
   rather than establish the firmware's nominal compensation.

Required correction before sweeping: define one common phase observable and
timestamp convention, then implement it in both environments. Recommended is a
2.5 kHz diagnostic stream containing the input sample, alpha, beta, and the
exact PLL output sample from the same firmware update frame. Compute all phase
values from those sample records using identical interpolation. A separate
continuous sample-and-hold pin phase may be reported, but must not be compared
to the discrete result as though they were the same observable.

## Active readiness contract audit

The active Stage-3D branch does not write `pll_locked` or `phase_ok`. Neither is
part of the active readiness contract.

| Ready signal | Writer | Update rate | Conditions | Consumer |
|---|---|---:|---|---|
| `an1_signal_valid` (`input_valid`) | inline `_ADCAN1Interrupt` Stage-3D path | 2.5 kHz | raw SOGI magnitude at/above 300 counts for 75 consecutive samples; clears below 200 counts for 8 samples | `DIAG_PLLThetaReadyForCurrent`, ZC validity/event logic, fault classification |
| `phase_init_complete` | inline Stage-3D one-shot phase initialization | 2.5 kHz | 50-sample SOGI warm-up, then configured positive input ZC; timeout fallback can initialize but is not by itself proof of valid AN1 | `grid_phase_chain_valid`, phase publication/current-phase chain |
| `an1_zc_recent_valid` | inline AN1 ZC period tracker | 2.5 kHz | a measured positive-ZC period inside the configured valid-period window while `an1_signal_valid=1`; clears on timeout or input invalid | final return of `DIAG_PLLThetaReadyForCurrent`, fault selection |
| `grid_pll_frequency_valid` | `DIAG_PLLThetaReadyForCurrent` | 40 kHz | `PLL_W55_STEP_Q <= pll_step_q <= PLL_W65_STEP_Q` | `grid_phase_chain_valid` |
| `grid_mailbox_valid` | `DIAG_PLLThetaReadyForCurrent` | 40 kHz | phase sequence nonzero, mailbox seqlock even, and unchanged sequence age less than 80 ticks (2.0 ms) | `grid_phase_chain_valid` |
| `grid_interpolation_valid` | `DIAG_PLLThetaReadyForCurrent` | 40 kHz | interpolation has been observed, interpolation is seeded, and unchanged output age less than 80 ticks (2.0 ms) | `grid_phase_chain_valid` |
| `grid_phase_chain_valid` | `DIAG_PLLThetaReadyForCurrent` | 40 kHz | `phase_init_complete && grid_pll_frequency_valid && grid_mailbox_valid && grid_interpolation_valid` | final PLL/current readiness gate and debug/state logic |
| function readiness return / `dbg_pll_ready_for_current` | `DIAG_PLLThetaReadyForCurrent` / safety-gate update | 40 kHz | `grid_phase_chain_valid && an1_signal_valid && an1_zc_recent_valid` | Stage-4 grid startup state machine and current-command safety gate |
| `grid_valid_confirm_40k_count` | Stage-4 `STATE_WAIT_GRID_VALID` | 40 kHz | readiness return remains true for 1000 consecutive ticks (25 ms); resets immediately if readiness becomes false | transition toward `STATE_WAIT_ZC_TO_COMMAND_SSR` while outputs remain off |

Important distinction: `grid_phase_chain_valid=1` itself does **not** directly
contain input magnitude, recent-ZC validity, or the 25 ms debounce. Magnitude and
recent ZC are added by the function return. The 25 ms hold is state-machine
qualification after readiness first becomes true.

## Active PLL feasibility PASS definition

For this build, PLL feasibility PASS requires all of the following, with no use
of legacy `pll_locked` or `phase_ok`:

1. `an1_signal_valid=1` after its actual 75-sample debounce and remains valid.
2. `phase_init_complete=1` from the intended warm-up/positive-ZC path, with no
   repeated initialization or reset.
3. `an1_zc_recent_valid=1` from valid measured input periods.
4. `pll_step_q` remains inside the active 55--65 Hz readiness window and the
   measured frequency converges to the test input within the agreed tolerance.
5. Phase mailbox sequence is nonzero, seqlock is even, and freshness is under
   80 40 kHz ticks.
6. 40 kHz phase interpolation is seeded, observed changing, and fresh under the
   same 80-tick limit.
7. Therefore `grid_phase_chain_valid=1`, and the complete
   `DIAG_PLLThetaReadyForCurrent()` return is 1.
8. Theta is continuous modulo wrap, phase error is bounded under a common
   measurement definition, and there are no repeated initialization/reset
   events.
9. For Stage-4 advancement, the complete readiness return additionally remains
   continuously true for 1000 40 kHz samples (25 ms).

## Compensation sweep

### Coarse sweep

Not run. The required 4 deg direct-SIMPLIS baseline did not match offline replay
for the two absolute phase observables.

### Zero-crossing bracket and fine sweep

Not established. No 0.5 deg, 0.1 deg, or 0.01 deg sweep was performed.

### Best compensation, residual, time error and sensitivity

Not validly determinable from the present cross-environment observables. In
particular, the old 15.42 deg value was not used and was not written to
firmware. Selecting a new value from the current held-waveform phase would bake
a measurement-interface delay into `PLL_PHASE_COMP_DEG`.

## Stage-3D Phase-1 completion decision

**Not complete yet.** Functional timing, initialization, frequency convergence,
fast-error convergence, and alpha/beta quadrature pass the direct baseline.
Absolute phase cross-check does not. The next work item is narrowly scoped:
make the direct SIMPLIS and offline replay phase diagnostics share the same
2.5 kHz sample timestamp/output convention, rerun the 4 deg baseline, and only
then execute the requested coarse and fine compensation sweep.

