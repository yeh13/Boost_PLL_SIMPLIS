# Full Controller Dry-Run Stage 1

## Status

**BLOCKED at the 40 kHz current-reference baseline.**

The active interpolation implementation does not preserve the phase of the
2.5 kHz post-update mailbox sample. It reconstructs from the previous output
toward the newly received mailbox target over 16 current-loop ticks. This
creates an approximately 15/16-sample lag relative to the current mailbox
phase. Therefore the previously selected `REF_PHASE_OFFSET=-4.55 deg`, which
was derived from direct `theta_after`, cannot be inserted into the active
interpolated path without first redefining/revalidating the reference offset.

Per the baseline gate, this report does not claim that the full controller/gate
model is ready. No power plant, hardware test, current-loop retuning or firmware
modification was performed.

## Active-build selections

| Item | Active value | Update rate | Writer | Consumer |
|---|---|---:|---|---|
| AN1/PLL stage | inline Stage-3D fixed normalization | 2.5 kHz after /16 | `_ADCAN1Interrupt` | phase mailbox |
| Phase source | post-update `theta_after` Q32 mailbox | 2.5 kHz | `DIAG_PLLPhaseMailboxPublish` | 40 kHz interpolator |
| Interpolation | `DIAG_PLL_PHASE_INTERP_40K_TEST=1`, 16 steps | 40 kHz | `DIAG_PLLPhaseInterpAdvance40k` | current command mapping |
| Current loop scheduler | `CONTROL_TASK_SCHEDULER_ENABLE=1` | 40 kHz | control scheduler / AN0 path | AN0 controller |
| AN0 center | 2048 counts | 40 kHz | AN0 ADC/filter | current error |
| AN0 filter | `y += (adc-y)>>2` | 40 kHz | AN0 path | current error |
| Current-error deadband | +/-2 counts; `abs(error)<3` becomes zero | 40 kHz | AN0 path | Boost 2P2Z |
| Iref zero band | `abs(i_ref)<20` becomes zero | 40 kHz | reference mapper | current error |
| Reference gain | Q15 mapping, peak placeholder 917 counts at `VREF_PEAK=3787` | 40 kHz | reference mapper | current error |
| Current soft-start | Q15 step 16/tick to 32768, about 51.2 ms | 40 kHz | Stage-4 state/AN0 path | reference amplitude |
| DC threshold | `Vdc=574` counts | 40 kHz | constant in `pwm.c` | mode selector |
| Mode hysteresis | +/-20 counts around Vdc | 40 kHz | mode selector | Buck/Boost ownership |
| Boost 2P2Z A | `{30010, 2668}` Q15 | Boost-mode 40 kHz | AN0 controller | duty correction |
| Boost 2P2Z B | `{26000,-32000,9640}` Q15 | Boost-mode 40 kHz | AN0 controller | duty correction |
| Buck 2P2Z | coefficients exist but are not active | n/a | n/a | n/a |
| Boost correction limit | +/-2500 counts | 40 kHz | AN0 controller | Boost duty |
| Buck duty limit | 0..12500 | 40 kHz | feedforward/mapping | PG1DC |
| Boost duty limit | 0..10625 | 40 kHz | feedforward + 2P2Z | PG2DC |
| Physical bumpless transfer | disabled (`0`) | mode transition | n/a | n/a |
| SSR test mode | `GRID_TEST_MODE_NO_SSR=1` | state-machine | Stage-4 | physical SSR remains off |

## Active controller equations

The 40 kHz current reference is derived from the interpolated 668-position
phase mapping and `VrefTable`, followed by Q15 current gain and Stage-4
soft-start. The current reference offset requested for future simulation is not
present in active firmware; it must remain a separate simulation/future-output
calibration.

Filtered feedback:

`adc_filt[n] = adc_filt[n-1] + ((adc_raw[n]-adc_filt[n-1]) >> 2)`.

The error sign is half-cycle normalized:

- positive half: `error = iref_cmd-adc_filt`;
- negative half: `error = adc_filt-iref_cmd`.

Thus positive error means insufficient current magnitude on both half cycles.

Boost controller:

`u[n] = (26000e[n]-32000e[n-1]+9640e[n-2]`
`        +30010d[n-1]+2668d[n-2]) >> 15`.

`u` is limited to +/-2500. Boost feedforward is:

`Boost_PWM = (abs(vref)-Vdc)*12500/(abs(vref)+Vdc)`.

Final Boost duty is `clamp(Boost_PWM+u,0,10625)`. Stored duty states are the
correction relative to feedforward.

Buck mode currently does not execute Buck 2P2Z. It uses:

`Buck_PWM = 6250 +/- abs(vref)*6250/Vdc`, clamped to 0..12500.

On every Buck/Boost change all controller states are cleared. Physical
bumpless transfer is compiled off.

## 40 kHz interpolation phase audit

On a new mailbox sequence:

1. `interp_start_q32` is the current interpolation output.
2. `interp_target_q32` becomes the newly published `theta_after`.
3. Delta is divided by 16 with signed truncation.
4. In the same 40 kHz call, step count becomes 1 and output advances by one
   sixteenth of that delta.
5. Calls 2..15 add another one-sixteenth; call 16 snaps to target.

For steady 60 Hz operation this output is approximately the previous mailbox
phase plus 1/16..16/16 of the latest mailbox delta. Relative to the physical
current time it lags the current post-update mailbox frame by approximately
15/16 of one PLL update.

Using the measured PLL frequency 59.999943371 Hz:

- one 2.5 kHz update = 8.639991845 deg;
- 15/16 update = 8.099992355 deg;
- Grid -> post-update mailbox = +4.548578 deg;
- predicted Grid -> active 40 kHz interpolation = about **-3.551414 deg**.

This must be verified by a bit-exact Stage-4 replay trace, but it is already a
large, structural mismatch from the requested +4.55 deg baseline—not a small
numeric error.

If `REF_PHASE_OFFSET=-4.55 deg` is then applied downstream, predicted
Grid-to-i_ref becomes approximately **-8.101414 deg**, not zero.

Therefore the current-reference baseline did not pass and the requested offset
was not promoted into the full controller model.

## Preliminary virtual-feedback controller exercise

A no-plant 40 kHz equation harness was run with a small 100-count reference,
the active AN0 filter, error polarity, Buck feedforward, Boost 2P2Z,
saturation and mode-state clearing. It uses an analytic sine proxy rather than
the bit-exact 334/668 lookup mapping, so these results are diagnostic only and
are not a substitute for the blocked interpolation baseline.

| Mode | Max abs error | Mean abs error | Boost clamps | Observation |
|---|---:|---:|---:|---|
| B, ideal filtered feedback equals i_ref | 0 | 0 | 0 | controller correction remains zero |
| B, raw ADC equals i_ref before active IIR | 3 | 1.440 | 50 | filter lag reaches deadband edge |
| A, feedback zero | 100 | 63.659 | 0 | correction has positive insufficient-current sign |
| C, feedback 0.8*i_ref | 23 | 12.885 | 1 | positive correction direction |
| C, feedback 1.2*i_ref | 23 | 12.385 | 97 | negative correction; feedforward ceiling causes clamps |
| C, feedback phase +5 deg | 9 | 4.074 | 48 | error changes sign across cycle as expected |

The harness observed no duty outside configured limits. It also exposed 144
Buck/Boost transitions over the 0.6 s run and a maximum PG command step of
6250 counts. Because physical bumpless transfer is disabled and controller
states clear on mode changes, mode-transition jump behavior is not yet a PASS.

## PWM ownership and gate timing

- AN0 ISR/current-loop path is the sole normal writer of PG1DC/PG2DC.
- PG1 and PG2 are configured hardware-complementary (`PMOD=Complementary`).
- `PG1PER=PG2PER=12499`, corresponding to the 40 kHz PWM frame.
- `PG1DTL=PG1DTH=PG2DTL=PG2DTH=100` counts.
- Auxiliary PLL setup gives a 500 MHz PWM time base; one count is 2 ns.
- Configured rising/falling dead time is therefore **200 ns**.
- In Boost mode PG1 is not PWM duty: override drives the half-cycle polarity
  pair, while PG2 is the controlled complementary PWM leg.
- In Buck mode PG1 is complementary PWM and PG2 duty is zero.
- `PowerPWM_AllOff()` sets both duties to zero and forces all four H/L outputs
  low through overrides.

An event-domain complementary-pair assertion confirmed that a correctly
modeled hardware complementary pair with 100-count gaps cannot have H and L
simultaneously high. A waveform-level gate PASS is intentionally withheld
until the exact interpolation/startup path is integrated; otherwise gate timing
would be tested with the wrong reference frame.

## Startup and readiness

Nonzero current command requires more than frequency convergence:

1. `phase_init_complete`;
2. PLL step inside 55--65 Hz;
3. nonzero/even/fresh phase mailbox (age under 80 40 kHz ticks);
4. seeded, changing and fresh interpolation (age under 80 ticks);
5. therefore `grid_phase_chain_valid=1`;
6. `an1_signal_valid=1`;
7. `an1_zc_recent_valid=1`;
8. the complete readiness return held for 1000 ticks (25 ms);
9. the required subsequent ZC state transitions;
10. `STATE_GRID_CURRENT_RUN` and the 51.2 ms Q15 current soft-start.

With `GRID_TEST_MODE_NO_SSR=1`, physical SSR command remains off. Logical
state/ZC sequencing still advances for debug.

## Safety-assertion disposition

| Assertion | Status |
|---|---|
| Same complementary pair never both high | structurally PASS for event model; waveform replay pending |
| Dead time at least firmware setting | 100 counts / 200 ns configured; waveform replay pending |
| Duty inside hardware limits | PASS in preliminary equation harness |
| Disabled outputs safe | active override path audited; integrated replay pending |
| Fault immediately forces all gates low | active `PowerPWM_AllOff` path audited; injected-fault replay pending |
| No output before readiness | state contract audited; integrated replay pending |
| ZC/polarity transition no shoot-through | hardware complementarity/override audited; integrated replay pending |
| Mode transition has no illegal jump | **NOT PASS**; 6250-count preliminary jump, bumpless disabled |

## Readiness for simplified plant integration

**Not ready.** Before adding a plant, the next simulation-only work must:

1. reproduce the exact Q32 mailbox/interpolator at 40 kHz and measure its
   Grid-to-phase result directly;
2. choose/revalidate a reference-output offset against that interpolated phase,
   keeping it separate from `PLL_PHASE_COMP_DEG`;
3. replay the exact 334/668 lookup mapping and soft-start;
4. resolve or explicitly accept the mode-transition command jump with existing
   firmware behavior; and
5. run waveform-level gate assertions through startup, ZC, mode change and
   injected fault.

No current controller or power-stage parameter should be retuned to hide the
interpolation mismatch.

