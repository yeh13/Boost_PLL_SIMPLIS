# Active Stage-3D Phase Reference Audit

## Decision

The active phase-measurement contract is split into two explicitly named
domains. They must never be mixed in one `Grid->PLL` result:

1. **CONTROLLER-DOMAIN PHASE**: values belonging to one 2.5 kHz Stage-3D
   update frame and evaluated against sample index `n`.
2. **ANALOG S/H OBSERVED PHASE**: continuous SIMPLIS pins after the controller
   values have been reconstructed as sample-and-hold analog waveforms.

PLL compensation tuning must use controller-domain phase. Analog S/H phase is
an interface/timing observation and must not be fed back into
`PLL_PHASE_COMP_DEG`.

The present audit does **not** authorize a compensation sweep. It found that
the previous offline `Grid->PLL=-4.4979 deg` and the active post-update phase
observable are from different theta frames.

## Exact active Stage-3D update ordering

The active path is the early-return branch in `_ADCAN1Interrupt`. Raw AN1 runs
at 40 kHz and every sixteenth entry executes this 2.5 kHz order:

1. Read the current `ADCBUF1`/`valchannel_AN1` sample and subtract
   `GRID_ADC_CENTER` to form `v_in_count`.
2. Update input ZC period/event state from the current centered sample.
3. Run the Q30/Q12 SOGI difference equations using current `v_in_count` and
   previous input/alpha/beta states.
4. Assign `sogi_va=va_new`, `sogi_vb=vb_new`, then shift all SOGI histories.
   Thus `alpha[n]` and `beta[n]` are **current-sample outputs**.
5. Calculate raw SOGI magnitude and update `an1_signal_valid` debounce.
6. Until phase initialization, perform SOGI warm-up and positive input-ZC
   detection. Pre-initialization samples return before the PLL core.
7. Read the current, not-yet-incremented NCO state as **theta_before[n]**.
   The phase detector uses `theta_before[n] + PLL_PHASE_COMP_DEG` together with
   current `alpha[n]` and `beta[n]`.
8. Normalize the phase error, update the carried-remainder 1/64 LPF, PI
   integrator, target step, clamp and acquisition slew.
9. Update `pll_step_q`.
10. Advance `theta_acc_q/pll_idx` using the new step, producing
    **theta_after[n]**.
11. Publish `theta_after[n]` through `DIAG_PLLPhaseMailboxPublish()`.
12. The feasibility model's `pll_sin[n]` is generated from this post-update
    theta. The active inline firmware does not generate a separate pre-update
    sine in this branch.

Consequently, phase detector theta and published/output theta differ by one
current NCO increment. At 60 Hz and 2.5 kHz this is approximately 8.64 deg.

## Sample timestamp definition

Let update frame `n` be the nth decimated AN1 event. Its controller timestamp
is the physical instant at which that ADC sample is consumed:

`t_n = n * Ts`, where `Ts = 1/2500 = 400 us`.

All controller-domain trace columns carry this same timestamp:

| Column | Definition at frame `n` |
|---|---|
| `n` | Stage-3D update index |
| `time_tick` | `n * 400 us` |
| `grid[n]` | centered AN1 sample consumed by the frame |
| `alpha[n]` | new SOGI alpha after processing `grid[n]` |
| `beta[n]` | new SOGI beta after processing `grid[n]` |
| `theta_before[n]` | NCO phase read by the detector before this frame's PI/NCO update |
| `theta_after[n]` | NCO phase after this frame's new step is applied; mailbox value |
| `pll_sin[n]` | `sin(theta_after[n])`, the selected active output/published-phase observable |

The feasibility model schedules its first `/16` update at 0.4 ms. Its analog
pins change at the update event via `transition(..., 0, 100 ns, 100 ns)` and are
then held until the next event. That held analog waveform does not move the
controller timestamp to `t_(n-1)` or `t_(n+1)`; it is a separate reconstruction
of the controller value assigned at `t_n`.

## Controller-domain phase algorithm

For steady-state sample records, use the same indices and the same analysis
window for grid, alpha, beta and PLL sine. At measured grid frequency `f`, with
`Fs=2500 Hz`:

`X_x = sum(x[n] * exp(-j*2*pi*f*n/Fs))`

and `phase_x = arg(X_x)`. Results are wrapped to `[-180, +180)`:

- `Grid->alpha = phase_alpha - phase_grid`
- `alpha->beta = phase_alpha - phase_beta`
- `Grid->PLL = phase_pll_after - phase_grid`

The sign of `alpha->beta` intentionally reports alpha leading beta as positive.

The direct SIMPLIS 1 s baseline was reconstructed into 2499 common 2.5 kHz
frames. Values below use 0.5--1.0 s at 60 Hz.

## Discrete interpolated-zero-crossing algorithm

This is a cross-check on the same controller samples, not a crossing of the
analog S/H pins. For a rising crossing where `x[n] < 0` and `x[n+1] >= 0`:

`fraction = -x[n] / (x[n+1] - x[n])`

`t_zc = (n + fraction) * Ts`

Matched crossing-time differences are converted at the measured grid
frequency. The same phase directions listed above are retained.

## Unified baseline results

| Observable | Controller DFT | Controller interpolated ZC | Analog S/H observed DFT |
|---|---:|---:|---:|
| Grid to alpha | -0.153750 deg | -0.153805 deg | -4.744152 deg |
| Alpha to beta | +90.000000 deg | +90.000046 deg | +89.999602 deg |
| Grid to PLL, post-update/published frame | +4.548578 deg | +4.548521 deg | -0.041826 deg |

DFT versus interpolated ZC disagreement is at most about 0.000057 deg. This is
the required evidence that the controller-domain measurement definition is
self-consistent and is not affected by sample-and-hold edge placement.

## Analog-to-controller offsets

Define offset as:

`analog S/H observed phase - controller-domain DFT phase`.

| Observable | Offset | Equivalent samples | Equivalent time at 2.5 kHz |
|---|---:|---:|---:|
| Grid to alpha | -4.590402 deg | -0.53130 sample | -212.52 us |
| Alpha to beta | -0.000398 deg | -0.000046 sample | -0.018 us |
| Grid to PLL | -4.590404 deg | -0.53130 sample | -212.52 us |

At 60 Hz / 2.5 kHz:

- one sample = 8.64 deg = 400 us;
- half a sample = 4.32 deg = 200 us.

The observed approximately 0.531-sample offset is common to Grid-to-alpha and
Grid-to-PLL, while alpha-to-beta cancels it. It consists primarily of the
sample-and-hold reconstruction delay. The remaining approximately 0.031 sample
is associated with the exported 25 us observation grid and transition/update
edge visibility; it is not a controller phase state. A raw adaptive-time
SIMPLIS dataset could refine the analog-only number, but is unnecessary for
controller compensation tuning.

## Why the earlier offline Grid-to-PLL result does not match

The active post-update controller result is `+4.5486 deg`, whereas the earlier
offline result was `-4.4979 deg`. Their separation is approximately 9.0465 deg,
close to one 60 Hz/2.5 kHz NCO update (8.64 deg), with the remainder attributable
to different integer/real and timestamp conventions.

This is consistent with comparing a detector/pre-update theta frame against a
post-update/published theta frame. It is not evidence that the direct SIMPLIS
loop failed: initialization, final frequency, fast error, and alpha/beta
quadrature already matched.

The earlier replay result must therefore be relabeled with its exact theta
read point or regenerated with both `theta_before[n]` and `theta_after[n]` in
the same trace. A single ambiguous `pll_sin[n]` column is no longer acceptable.

## Which phase definition controls compensation tuning

Use **CONTROLLER-DOMAIN PHASE**, measured from common-index discrete records.
For the active Stage-4 consumer, the authoritative PLL output reference is the
post-update phase published to the mailbox. The detector's pre-update theta is
a separate internal observable useful for loop diagnostics.

Do not use analog S/H observed phase to tune `PLL_PHASE_COMP_DEG`; its offset
belongs to waveform reconstruction and observation timing. Likewise, do not
compare pre-update replay PLL sine against post-update mailbox PLL sine.

## Status of the active 4 deg compensation and sweep gate

`PLL_PHASE_COMP_DEG=4 deg` was retained and no firmware was changed.

The 4 deg case cannot yet be declared nominally phase-correct because the
desired tuning target must state which controller signal is intended to align:

- detector/pre-update theta, or
- Stage-4 post-update/published theta.

Firmware architecture makes post-update/published theta authoritative for the
Stage-4 consumer, but the existing offline target values were produced from a
different frame. The next simulation-only action is to export both theta frames
from the integer replay, confirm the one-step relationship, and define the
desired post-update Grid-to-PLL target. Until that is done, the coarse/fine
phase-compensation sweep remains blocked. No firmware modification is required
for this measurement audit.

