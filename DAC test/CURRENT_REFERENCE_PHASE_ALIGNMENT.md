# Current-Reference Phase Alignment

## Scope

This is a minimum dry-run reference model only:

`theta_after -> sine(theta_after + REF_PHASE_OFFSET) -> i_ref_test`

It contains no current controller, current plant, PWM, MOSFET or power-stage
model. `REF_PHASE_OFFSET` is a simulation/reference-output calibration and is
completely independent of `PLL_PHASE_COMP_DEG`.

The active PLL remains at `PLL_PHASE_COMP_DEG=4 deg`. No firmware or control
parameter was modified.

## Reference-frame contract

The current-reference source is the active post-update phase:

`i_ref_test[n] = sin(theta_after[n] + REF_PHASE_OFFSET)`.

In the future controller this means the freshness-checked mailbox phase and its
40 kHz interpolation, not `theta_before` and not `theta_detector`.

The simulation-only `pll_stage3d_active_test.va` was extended with an
`i_ref_test` diagnostic pin and a `REF_PHASE_OFFSET_DEG` parameter. That
parameter appears only in the reference sine expression and has no path back
to SOGI, detector, PI, frequency, NCO or readiness.

## Baseline: `REF_PHASE_OFFSET=0 deg`

Conditions: 60 Hz, 1000 centered-count peak, cold start, 1 s direct SIMPLIS.
Controller-domain analysis uses common 2.5 kHz indices over 0.5--1.0 s.

| Measurement | DFT | Discrete interpolated ZC | Result |
|---|---:|---:|---|
| Grid -> `theta_after` sine | +4.548578° | +4.548521° | reference frame |
| Grid -> `i_ref_test` | +4.548578° | +4.548521° | PASS; identical to theta_after |
| Grid -> `theta_detector` | -0.091402° | -0.091270° | unchanged detector equilibrium |

Other baseline results:

| Signal | Result |
|---|---:|
| Final PLL frequency | 59.999943371 Hz |
| Final fast error | -0.819769 count |
| `input_valid` assertion | 32.025 ms; remains asserted |
| `phase_initialized` assertion | 33.625 ms; remains asserted |

The baseline matches the expected approximately +4.55° post-update phase, so
there is no reference-frame fallback to `theta_before`.

## Reference phase-offset sweep

Because the offset is a pure downstream rotation with no feedback, all cases
use the same verified `theta_after[n]` trace. This is an exact controller-domain
sample replay of the minimum dry-run equation, not a rerun or retuning of the
PLL. DFT phase is the sweep objective.

Invariant values in every case:

- Grid -> theta detector: -0.091402° DFT;
- Grid -> theta after: +4.548578° DFT;
- PLL frequency: 59.999943371 Hz;
- final fast error: -0.819769 count;
- input valid: PASS;
- phase initialized: PASS;
- active readiness contract: unaffected by the reference offset.

### Coarse sweep

| `REF_PHASE_OFFSET` | Grid -> `i_ref_test` DFT | Equivalent time at 60 Hz | Active readiness |
|---:|---:|---:|---|
| -6.0° | -1.451422° | -67.195 us | unchanged |
| -5.0° | -0.451422° | -20.899 us | unchanged |
| -4.5° | +0.048578° | +2.249 us | unchanged |
| -4.0° | +0.548578° | +25.397 us | unchanged |
| -3.0° | +1.548578° | +71.694 us | unchanged |
| -2.0° | +2.548578° | +117.990 us | unchanged |
| 0.0° | +4.548578° | +210.582 us | unchanged |

The zero-phase bracket is `[-5.0, -4.5] deg`.

### Fine sweep at 0.1 deg

| Offset | Residual Grid -> `i_ref_test` |
|---:|---:|
| -4.4° | +0.148578° |
| -4.5° | +0.048578° |
| -4.6° | -0.051422° |
| -4.7° | -0.151422° |

The 0.1° bracket is `[-4.6, -4.5] deg`.

### Fine sweep at 0.01 deg

| Offset | Residual Grid -> `i_ref_test` | Equivalent time at 60 Hz |
|---:|---:|---:|
| -4.53° | +0.018578° | +0.860 us |
| -4.54° | +0.008578° | +0.397 us |
| **-4.55°** | **-0.001422°** | **-0.0658 us** |
| -4.56° | -0.011422° | -0.5288 us |
| -4.57° | -0.021422° | -0.9918 us |

Best value on the requested 0.01° grid:

`REF_PHASE_OFFSET = -4.55 deg`

with residual `-0.001422 deg`, equivalent to approximately `-0.066 us` at
60 Hz. The mathematical zero from this baseline is approximately
`-4.548578 deg`; the report recommends the explicitly tested resolution value
`-4.55 deg`, not writing either value into firmware at this stage.

## Active readiness contract

`grid_phase_chain_valid` is written/evaluated by
`DIAG_PLLThetaReadyForCurrent()` at the 40 kHz Stage-4 rate. It is true only
when all of these are true:

1. `phase_init_complete` is true.
2. `pll_step_q` is inside the 55--65 Hz window.
3. Mailbox sequence is nonzero.
4. Mailbox seqlock is even, so no publish is in progress.
5. Mailbox sequence freshness is less than 80 40 kHz ticks (2 ms).
6. Interpolation has been observed and seeded.
7. Interpolated phase freshness is less than 80 40 kHz ticks (2 ms).

There is no independent debounce inside `grid_phase_chain_valid`.

The complete function readiness returned to the Stage-4 consumer additionally
requires:

- `an1_signal_valid=1`; and
- `an1_zc_recent_valid=1`.

The startup state machine then requires the complete readiness return to remain
true for `GRID_VALID_CONFIRM_40K_SAMPLES=1000`, or 25 ms, in
`STATE_WAIT_GRID_VALID`. Any loss resets this confirmation and returns the
state toward PLL locking. Outputs remain off during this qualification.

The minimum Verilog-A PLL model directly exposes input validity, initialization,
frequency and phase, but does not implement the complete C seqlock,
40 kHz interpolation-freshness state machine, or `grid_phase_chain_valid` pin.
Therefore the sweep table records readiness as **unchanged by offset**, not as
a fabricated direct waveform measurement. Full Stage-4 replay must expose the
actual readiness bits before current-command enable testing.

## When current reference may be enabled

It is safe to calculate a zero-amplitude/debug reference before readiness, but
a nonzero current-reference command must not be admitted to the future current
loop until:

1. the complete `DIAG_PLLThetaReadyForCurrent()` result is true, including
   phase chain, input magnitude and recent-ZC validity;
2. it has remained true through the 25 ms Stage-4 confirmation;
3. the startup state machine has reached the intended post-confirmation/ZC
   synchronization state; and
4. the reference is introduced through the future soft-start/ramp rather than
   as an immediate step.

This is stricter than `grid_phase_chain_valid=1` alone. In particular,
`grid_phase_chain_valid` does not itself contain input-valid, recent-ZC or the
25 ms state debounce.

## Recommendation for future current-loop simulation

- Source phase from the 40 kHz interpolated form of mailbox
  `theta_after`, with the existing sequence/seqlock/freshness checks.
- Keep `PLL_PHASE_COMP_DEG=4 deg` as detector calibration.
- Keep `REF_PHASE_OFFSET` as a separate current-reference/output calibration.
- Start with `REF_PHASE_OFFSET=-4.55 deg` as the 60 Hz nominal simulation value,
  but keep it configurable and do not commit it to firmware until frequency,
  amplitude and startup-transition tests establish robustness.
- Gate nonzero reference with the complete readiness and startup contract.
- Add current controller and plant only after reference phase, readiness and
  zero-crossing enable sequencing are reproduced in the full controller model.

## Completion state

The minimum current-reference phase-alignment dry run is complete. No current
controller, power plant, PWM or MOSFET model was added. Work stops here as
requested.

