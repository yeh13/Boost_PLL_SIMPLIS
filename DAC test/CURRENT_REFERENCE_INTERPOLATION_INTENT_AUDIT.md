# Current-Reference Interpolation Intent Audit

## Executive conclusion

The active interpolator is **TYPE A: causal smoothing of a past mailbox
target**. It is not forward phase reconstruction or prediction.

It successfully converts the 2.5 kHz held angle into an extremely smooth
40 kHz ramp and suppresses the principal 2.5 kHz sampling images by about
71 dB. At the same time, its causal timing produces a steady approximately
15/16-mailbox-increment phase lag relative to a same-timestamp ideal 40 kHz
phase. Smoothness is therefore a PASS; absolute current-reference phase needs
an independent reference-output calibration.

No firmware or controller parameter was modified.

## Active implementation audit

### Mailbox writer

The 2.5 kHz inline Stage-3D path advances the NCO to `theta_after`, converts it
to Q32 and calls `DIAG_PLLPhaseMailboxPublish()`. The writer uses an odd/even
seqlock around target and sequence updates.

### Mailbox reader and new-target timing

`DIAG_PLLPhaseInterpAdvance40k()` runs at 40 kHz. It reads a consistent target
and sequence. When the sequence changes:

1. `interp_start_q32 = interp_output_q32`;
2. `interp_target_q32 = active_target`;
3. shortest signed delta is obtained by casting Q32 subtraction to signed
   32-bit (`DIAG_PhaseDeltaQ32`);
4. step is `delta/16` with explicit signed truncation toward zero;
5. `interp_step_count` is reset to zero;
6. in that same call, count increments to one and output advances by step 1;
7. calls 2 through 15 add another step;
8. call 16 assigns the exact target, absorbing division remainder.

The next 2.5 kHz target nominally arrives at the next 16-tick boundary, after
the prior interval has reached its target. Scheduler/ISR ordering can decide
which routine is observed first on the exact boundary, but the active sequence
check and snap behavior keep the causal construction: it never predicts beyond
the newest mailbox target.

Wrap is handled by signed Q32 subtraction. A 359-to-0-degree forward move
becomes a small positive signed delta rather than a -359-degree reverse move.

## One steady-state 16-tick interval

Measured PLL frequency: 59.999943371 Hz.

- mailbox increment: 8.639991845°;
- interpolation step: approximately 0.539999490° per 40 kHz tick.

The table uses the latest mailbox target as 0° relative reference.

| Tick | Mailbox target | Interp start | Interp step | Interp output relative target | Progress |
|---:|---:|---:|---:|---:|---:|
| 0 | 0° | -8.639992° | +0.539999° | -8.099992° | 1/16 |
| 1 | 0° | -8.639992° | +0.539999° | -7.559993° | 2/16 |
| 2 | 0° | -8.639992° | +0.539999° | -7.019993° | 3/16 |
| 3 | 0° | -8.639992° | +0.539999° | -6.479994° | 4/16 |
| 4 | 0° | -8.639992° | +0.539999° | -5.939994° | 5/16 |
| 5 | 0° | -8.639992° | +0.539999° | -5.399995° | 6/16 |
| 6 | 0° | -8.639992° | +0.539999° | -4.859995° | 7/16 |
| 7 | 0° | -8.639992° | +0.539999° | -4.319996° | 8/16 |
| 8 | 0° | -8.639992° | +0.539999° | -3.779996° | 9/16 |
| 9 | 0° | -8.639992° | +0.539999° | -3.239997° | 10/16 |
| 10 | 0° | -8.639992° | +0.539999° | -2.699997° | 11/16 |
| 11 | 0° | -8.639992° | +0.539999° | -2.159998° | 12/16 |
| 12 | 0° | -8.639992° | +0.539999° | -1.619998° | 13/16 |
| 13 | 0° | -8.639992° | +0.539999° | -1.079999° | 14/16 |
| 14 | 0° | -8.639992° | +0.539999° | -0.539999° | 15/16 |
| 15 | 0° | -8.639992° | snap | 0° | 16/16 |

## Three same-timestamp angle references

- **ANGLE A — theta_hold:** newest 2.5 kHz mailbox target held for 16 current
  ticks.
- **ANGLE B — theta_interp_active:** exact active Q32 target/start/signed-delta,
  truncation, first-step and final-snap replay.
- **ANGLE C — theta_ideal_40k:** analysis-only same-time phase advancing by
  `360*fPLL/40000` every current tick and passing through the mailbox phase at
  each 2.5 kHz publication instant.

All recorded columns use the same physical 40 kHz timestamp. The generated CSV
contains five plotted 60 Hz cycles:

[Same-timestamp trace](<D:\Desktop\DAC test\CURRENT_REFERENCE_INTERPOLATION_PLOTS\interpolation_trace.csv>)

## Angle plots

- [Theta hold](<D:\Desktop\DAC test\CURRENT_REFERENCE_INTERPOLATION_PLOTS\theta_hold.svg>)
- [Active interpolation](<D:\Desktop\DAC test\CURRENT_REFERENCE_INTERPOLATION_PLOTS\theta_interp_active.svg>)
- [Ideal 40 kHz](<D:\Desktop\DAC test\CURRENT_REFERENCE_INTERPOLATION_PLOTS\theta_ideal_40k.svg>)
- [Wrapped overlay](<D:\Desktop\DAC test\CURRENT_REFERENCE_INTERPOLATION_PLOTS\theta_overlay.svg>)
- [Unwrapped overlay](<D:\Desktop\DAC test\CURRENT_REFERENCE_INTERPOLATION_PLOTS\theta_unwrapped_overlay.svg>)

The hold plot has explicit 16-tick stairs. The active unwrapped plot is nearly
linear. It has no residual 16-tick plateaus or periodic 8.64° jump. Q32 division
remainder is absorbed by the tick-16 snap without a material slope jump.

## Delta-theta and phase-error statistics

Steady data exclude the initial seed interval.

| Metric | HOLD | ACTIVE | IDEAL |
|---|---:|---:|---:|
| Mean delta/tick | 0.53918°* | 0.539999490° | 0.539999490° |
| Delta standard deviation | 2.08993° | 0.000000280° | <0.000000001° |
| Minimum delta | 0° | 0.539999417° | 0.539999490° |
| Maximum delta | 8.639991861° | 0.540000592° | 0.539999491° |
| RMS phase error vs ideal | 4.75386° | 8.09999° | 0° |
| Maximum absolute error vs ideal | 8.09999° | 8.09999° | 0° |

`*` HOLD's finite-window mean differs slightly because the window ends inside a
hold interval; over complete mailbox intervals its mean is the ideal increment.

ACTIVE is vastly smoother than HOLD. Its larger RMS phase error is a constant
causal lag, not angle ripple.

## Controller-domain phase

`REF_PHASE_OFFSET=0` for all results in this section.

| Grid-relative phase | 40 kHz DFT | Discrete interpolated positive ZC |
|---|---:|---:|
| theta_hold reconstruction | +0.498581° | +0.312952° |
| theta_interp_active | **-3.550330°** | **-3.551414°** |
| theta_ideal_40k | +4.548570° | +4.548578° |

HOLD DFT and ZC differ because a held discontinuous waveform does not have a
unique smooth crossing time; the result depends on reconstruction convention.
ACTIVE and IDEAL agree between DFT and ZC to about 0.0011° or better.

The active-to-latest-mailbox relationship ranges from -15/16 increment at tick
0 to zero at tick 15. Its interval average is not the key timing metric. The
same-timestamp active-to-ideal error is essentially constant:

`theta_interp_active-theta_ideal_40k = -8.099992°`.

This validates the 15/16 derivation and, more importantly, proves that the same
lag exists relative to physical 40 kHz time—not merely relative to a future
mailbox label.

## Sine-reference waveform and spectral comparison

[HOLD / ACTIVE / IDEAL sine overlay](<D:\Desktop\DAC test\CURRENT_REFERENCE_INTERPOLATION_PLOTS\sine_reference_overlay.svg>)

| Metric relative to ideal sine | HOLD | ACTIVE, offset 0 |
|---|---:|---:|
| RMS waveform error | 0.0586375 pu | 0.0998728 pu |
| Peak waveform error | 0.141254 pu | 0.141254 pu |
| 2440 Hz image amplitude | 0.0247179 pu | 0.00000680 pu |
| 2560 Hz image amplitude | 0.0235738 pu | 0.00000619 pu |

The active waveform has larger error against the chosen ideal only because it
has a fixed -8.10° phase. Its shape is much smoother. At the principal
`2500 Hz +/- 60 Hz` sampling images, interpolation improves amplitude by about
71 dB.

Conventional integer-harmonic THD from the finite record was approximately
0.000685% for HOLD and 0.00955% for ACTIVE. That number is misleading for this
question: the dominant staircase products are asynchronous sampling images
near 2.5 kHz, not integer 60 Hz harmonics. The explicitly measured image
components and waveform plots are the appropriate evidence of smoothing.
After removing the constant phase offset, ACTIVE approaches the ideal sine and
retains the approximately 71 dB image suppression.

## Independent reference-offset audit

Interpolation timing, smoothness and phase are now coherent, so a downstream
reference-offset sweep is permitted. The PLL/SOGI trace is unchanged in all
cases.

Coarse prediction/DFT rotation:

| REF offset | Grid -> i_ref_interp |
|---:|---:|
| +2.0° | -1.550330° |
| +3.0° | -0.550330° |
| +3.5° | -0.050330° |
| +4.0° | +0.449670° |
| +5.0° | +1.449670° |

The 0.1° bracket is +3.5° to +3.6°. On the 0.01° grid, the best value is:

`REF_PHASE_OFFSET = +3.55 deg`

with DFT residual approximately `-0.000330 deg` and ZC residual approximately
`-0.001414 deg`. The mathematical DFT zero is about +3.55033°.

This replaces the earlier direct-mailbox-only candidate of -4.55° for the
interpolated current-reference path. It remains a simulation recommendation,
not a firmware change. Offset rotation is strictly downstream and does not
change detector phase, PLL frequency, fast error or alpha/beta.

## Complete phase-chain table

| Signal/frame | Rate | Grid-relative phase | Role |
|---|---:|---:|---|
| theta_before | 2.5 kHz | -4.091405° | phase read before current PLL update |
| theta_detector | 2.5 kHz | -0.091402° | internal detector phase with 4° compensation |
| theta_after mailbox | 2.5 kHz | +4.548578° | published post-update phase |
| theta_hold, 40 kHz S/H view | 40 kHz | +0.498581° DFT | no-interpolation comparison |
| theta_interp_active | 40 kHz | -3.550330° DFT | active causal smoothing output |
| theta_ideal_40k | 40 kHz | +4.548570° DFT | analysis-only forward-time reference |
| i_ref_interp, offset 0 | 40 kHz | -3.550330° DFT | active reference before output calibration |
| i_ref_interp, offset +3.55° | 40 kHz | -0.000330° DFT | recommended simulation alignment |

## Open issue retained

The previously observed 6250-count mode-transition discontinuity remains open.
No bumpless-transfer, mode, duty or PWM logic was changed or evaluated for PASS
in this audit.

## Can FULL_CONTROLLER_DRY_RUN_STAGE1 resume?

**Yes, with conditions.** The phase-chain blocker is resolved at the analysis
level:

- retain the active interpolator;
- model it as causal past-target smoothing;
- use `REF_PHASE_OFFSET=+3.55°` only as a separate simulation current-reference
  calibration;
- reproduce the exact Q32/668-table path in the resumed controller dry-run;
- keep the 6250-count mode-transition discontinuity as a blocking gate issue;
- do not modify `PLL_PHASE_COMP_DEG=4°`.

