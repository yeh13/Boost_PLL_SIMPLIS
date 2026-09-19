# Active Stage-3D Phase-Frame Audit

## Scope and result

This is a simulation-only phase-frame audit. The active firmware and all
control parameters remain unchanged. No compensation sweep was run.

The 1 s, 60 Hz, 1000 centered-count peak, cold-start direct SIMPLIS case was
rerun with additional diagnostic outputs from
`pll_stage3d_active_test.va`. Every actual 2.5 kHz update frame records:

`grid[n], alpha[n], beta[n], theta_before[n], theta_detector[n],`
`theta_after[n], sin(theta_before[n]), sin(theta_detector[n]),`
`sin(theta_after[n])`.

The audit directly confirms that `theta_after-theta_before` is one NCO update.

## Exact active update ordering

At each 40 kHz AN1 interrupt the current ADC value is captured. Fifteen of
every sixteen entries return after raw-rate work. On the sixteenth entry, the
following Stage-3D operations occur in this exact order:

1. **Input sample**
   - Read `ADCBUF1` into `valchannel_AN1`.
   - Compute `v_in_count = valchannel_AN1 - GRID_ADC_CENTER`.
   - Update grid polarity/ZC-event state and positive-ZC period state from this
     current centered sample.

2. **SOGI update**
   - Form `v_in_q` from current `v_in_count`.
   - Evaluate `acc_va` and `acc_vb` from the current input and previous input,
     alpha and beta states.
   - Apply signed Q30 rounding to obtain `va_new` and `vb_new`.
   - Assign current outputs `sogi_va=va_new`, `sogi_vb=vb_new`.
   - Only then shift `vin`, `va` and `vb` histories.
   - Therefore `alpha[n]` and `beta[n]` are current-sample outputs associated
     with `grid[n]`, not delayed labels for frame `n-1`.

3. **Magnitude, input-valid and initialization state**
   - Calculate raw alpha/beta magnitude.
   - Update 75-sample valid and 8-sample lost debounce.
   - Before initialization, update the SOGI warm-up and input-ZC state.
   - A pre-initialization frame returns before the phase detector unless the
     initialization event completes in that frame.
   - On initialization, set NCO phase, clear PI/LPF state and set
     `phase_init_complete`.

4. **Capture `theta_before[n]`**
   - This is the current `pll_idx + theta_acc_q` state before this frame's PI
     correction and before this frame's NCO increment.

5. **Construct detector phase**
   - `theta_detector[n] = theta_before[n] + PLL_PHASE_COMP_DEG`, modulo 360°.
   - With the active build, `PLL_PHASE_COMP_DEG=4°`.
   - Generate detector sine and cosine from this phase.

6. **Phase detector evaluation**
   - Use current `alpha[n]`, current `beta[n]`, and
     `theta_detector[n]` in the Park detector.
   - Apply fixed Q15 Stage-3D normalization and error limiting.

7. **LPF and PI/frequency correction**
   - Update the signed 1/64 error LPF with its carried remainder.
   - Form the integrator increment with explicit signed truncation by 16.
   - Apply integrator limit and anti-windup.
   - Calculate proportional and integral step terms.
   - Form `pll_step_target_q` and clamp it to the 55--65 Hz actuator range.

8. **PLL step update**
   - Apply acquisition slew limiting to
     `pll_step_target_q-pll_step_q`.
   - Store the resulting current-frame `pll_step_q`.

9. **Theta accumulator update**
   - Add the new `pll_step_q` contribution to `theta_acc_q`.
   - Extract whole-degree steps, retain the fractional remainder, advance
     `pll_idx`, and wrap modulo 360°.

10. **Produce `theta_after[n]`**
    - `theta_after[n]` is the resulting NCO phase after step 9.

11. **Mailbox/output write**
    - Publish `theta_after[n]` through `DIAG_PLLPhaseMailboxPublish()`.
    - The active feasibility output is
      `sin_theta_after[n] = sin(theta_after[n])`.
    - No separate pre-update sine is written by the active inline firmware
      branch; it was added only as a simulation diagnostic for this audit.

No state update capable of causing a one-frame difference is placed between
the definitions above without being listed.

## Phase-frame definitions

| Signal | Definition |
|---|---|
| `theta_before[n]` | NCO state read by the detector before the current frame's PI/NCO advance |
| `theta_detector[n]` | `wrap(theta_before[n] + 4°)`; the phase used to generate detector sine/cosine |
| `theta_after[n]` | NCO state after applying the current frame's corrected step; the phase published to the mailbox |
| `sin_theta_before[n]` | Audit-only sine of pre-update phase |
| `sin_theta_detector[n]` | Audit-only sine of detector phase |
| `sin_theta_after[n]` | Active feasibility/mailbox output-frame sine |

All controller-domain columns share the physical sample timestamp
`t_n=n/2500`. The analysis does not use the analog S/H edge as the sample
timestamp.

## Controller-domain calculation contract

The DFT uses the same steady-state indices for every signal:

`X_x = sum(x[n] * exp(-j*2*pi*f*n/Fs))`, with `Fs=2500 Hz`.

Discrete rising ZC uses the same records:

`fraction = -x[n] / (x[n+1]-x[n])`

`t_zc = (n+fraction)/Fs`.

All differences are wrapped to `[-180,+180)` and use these signs:

- Grid to signal: `phase_signal-phase_grid`.
- Alpha to beta: `phase_alpha-phase_beta`.

## DFT versus discrete-ZC results

Analysis window: 0.5--1.0 s of the direct SIMPLIS baseline.

| Controller-domain phase | DFT | Discrete interpolated ZC | Difference |
|---|---:|---:|---:|
| Grid -> `sin(theta_before)` | -4.091405° | -4.091299° | -0.000106° |
| Grid -> `sin(theta_detector)` | -0.091402° | -0.091270° | -0.000133° |
| Grid -> `sin(theta_after)` | +4.548578° | +4.548521° | +0.000057° |
| Grid -> alpha | -0.153750° | -0.153805° | +0.000055° |
| Alpha -> beta | +90.000000° | +90.000046° | -0.000046° |

The two independent discrete-domain methods agree for every required frame.

## One-update phase increment

Measured from the phase traces:

| Method | `phase(theta_after)-phase(theta_before)` |
|---|---:|
| DFT | +8.639983° |
| Discrete ZC | +8.639820° |

The final direct-SIMPLIS PLL frequency is 59.999943371 Hz. Its expected
one-update increment is:

`360 * 59.999943371 / 2500 = 8.639991845°`.

DFT differs from the frequency-derived value by about -0.000009°. The
one-NCO-update relationship is confirmed.

## Interpretation of the active 4° compensation

At steady state:

### A. `theta_before` relative to Grid

`Grid -> theta_before = -4.091405°` by DFT.

### B. `theta_detector` relative to Grid

`Grid -> theta_detector = -0.091402°` by DFT.

This equals the pre-update result plus the configured 4° within numerical
resolution.

### C. `theta_after` relative to Grid

`Grid -> theta_after = +4.548578°` by DFT.

This equals `theta_before` plus the actual one-update increment.

### D. Detector equilibrium frame

The phase detector equilibrium exists in the **detector frame**, using current
`alpha[n]/beta[n]` and `theta_detector[n]`. It does not use the post-update
mailbox phase.

Grid-to-alpha is -0.153750°, while Grid-to-detector is -0.091402°; their
residual separation is about +0.062348°. The final fast error is also slightly
nonzero (-0.820 count). Thus the detector is near its quantized/real-model
zero-error equilibrium in the detector frame. The +4.5486° mailbox phase must
not be mistaken for detector error; it includes the subsequent 8.64° NCO
advance.

## Phase-compensation sweep contract

Two different questions require two different metrics:

1. To verify the internal PLL detector and tune the meaning of
   `PLL_PHASE_COMP_DEG`, use the **detector-frame metric**. The primary internal
   residual is `phase(theta_detector)-phase(alpha)` (with raw detector error as
   a supporting signal), because those are the quantities actually entering
   the phase detector in the same frame.
2. To verify end-use grid synchronization, use the **post-update mailbox
   phase** as a system acceptance metric, because Stage-4 consumes the
   published/interpolated phase, not `theta_before` or detector-compensated
   theta.

For the requested future compensation sweep, the recommended optimization
objective is the **detector-frame phase/error**, not pre-update phase and not
post-update mailbox phase. This preserves the semantic role of
`PLL_PHASE_COMP_DEG` and avoids absorbing the deterministic one-update pipeline
advance into an internal detector correction.

The sweep must nevertheless record post-update Grid-to-PLL phase as a separate
acceptance result. If the final current-reference synchronization target is not
met, compensate the consumer/reference phase explicitly rather than silently
retuning the detector to cancel pipeline timing.

## Recommended frame for future current-reference generation

Future current-reference generation should use **`theta_after` as published by
the mailbox, followed by the existing 40 kHz freshness-checked interpolation**.
Reasons:

- it is the actual active Stage-4 ownership boundary;
- it contains the newest corrected NCO step;
- its sequence/seqlock/freshness contract is already audited;
- using `theta_before` would deliberately consume an older frame;
- using `theta_detector` would incorrectly expose an internal detector
  compensation as the commanded current phase.

If hardware/control objectives require the current reference to have a defined
phase relative to Grid, apply and document a separate current-reference/output
phase offset to the interpolated `theta_after`. Do not overload
`PLL_PHASE_COMP_DEG` with sample-pipeline, PWM, plant or current-sensor delay.

## Completion state

The phase-frame relationship is fully confirmed. The active 4° parameter was
not changed, and no sweep was run. This audit stops here as requested.

