# Full Controller Dry-Run Stage 1 Continuation

## Status

**Phase A exact-table replay found a polarity-contract mismatch. Stage 1 does
not proceed to MODE A/B/C or a plant.**

The active 668-to-334 mapping was reproduced exactly at 40 kHz using the active
Q32 interpolator and simulation reference offset `+3.55 deg`. The lookup-table
shape and update rate are valid, but the firmware half-cycle sign convention
produces `-sin(theta)` relative to the requested `+sin(theta)` definition.

The mode-transition audit was completed independently. The previously reported
6250-count transition was a classification error in the preliminary harness:
it compared safe-off compare zero against Buck's 50% neutral compare at enable.
It is not the largest naturally occurring Buck/Boost handoff in the exact
active mapping.

No firmware, mode, bumpless, PWM or controller parameter was modified.

## Phase A — exact 668-table replay

### Table source and ownership

The active source is `VrefTable[334]` in `mcc_generated_files/pwm.c`. It is a
334-entry positive half-sine magnitude table with peak 3787 counts.

The 40 kHz interpolation owner maps Q32 phase as:

`full_phase_idx = (interp_phase_q32 * 668) >> 32`, constrained to 0..667.

`DIAG_PLLPhaseToOldFrameCommand(..., source=668)` then maps:

| Full index | Table index | `phase_cmd` | Active signed i_ref |
|---|---:|---:|---:|
| 0..333 | same index | 0 | negative magnitude |
| 334..667 | index-334 | 1 | positive magnitude |

At wrap, Q32 naturally rolls over and full index returns from 667 to zero.
Table lookup runs on every 40 kHz interpolation call. It does not return to a
2.5 kHz held reference.

The replay applies the simulation-only phase offset before mapping:

`phase_for_reference = theta_interp_active + 3.55 deg`.

### Ideal versus exact signed table

The requested comparison defines:

`i_ref_ideal = +sin(theta_interp+3.55 deg)`.

The active signed mapping produces approximately:

`i_ref_table = -sin(theta_interp+3.55 deg)`.

Measured over 3 seconds at 40 kHz:

| Metric | Exact result |
|---|---:|
| RMS error vs requested +sin | 1.414224 pu |
| Peak error vs requested +sin | 2.000000 pu |
| Grid-relative table fundamental phase | +179.730433 deg |
| Grid-relative requested ideal phase | -0.000341 deg |
| Fundamental magnitude | 1.000016 pu |
| Conventional harmonic THD, harmonics 2..100 | 0.057842% |
| 2440 Hz sampling image | 0.00000429 pu |
| 2560 Hz sampling image | 0.00000567 pu |

The approximately 0.269° departure from exactly 180° is consistent with the
floor-based 668-position table quantization. Zero-crossing and peak timing are
approximately half a 60 Hz cycle away from the requested +sin reference, with
the additional sub-degree table-index timing offset.

This is a polarity/frame contract failure, not excessive lookup-table
distortion. If the intended current direction is actually the firmware's
historical negative-first convention, the correct ideal comparison must be
explicitly changed to `-sin`; this audit does not assume permission to change
that system-level definition.

### Exact-table plots and trace

- [Five-cycle ideal/table overlay](<D:\Desktop\DAC test\FULL_CONTROLLER_STAGE1_CONTINUATION_PLOTS\ideal_vs_table_5cycles.svg>)
- [Difference waveform](<D:\Desktop\DAC test\FULL_CONTROLLER_STAGE1_CONTINUATION_PLOTS\table_error_5cycles.svg>)
- [Zero-crossing zoom](<D:\Desktop\DAC test\FULL_CONTROLLER_STAGE1_CONTINUATION_PLOTS\zero_crossing_zoom.svg>)
- [Peak-region zoom](<D:\Desktop\DAC test\FULL_CONTROLLER_STAGE1_CONTINUATION_PLOTS\peak_zoom.svg>)
- [Exact 40 kHz waveform CSV](<D:\Desktop\DAC test\FULL_CONTROLLER_STAGE1_CONTINUATION_PLOTS\exact_table_wave.csv>)

## Current-reference phase disposition

`REF_PHASE_OFFSET=+3.55 deg` correctly cancels the causal interpolator lag for
a positive sine generated directly from phase. It cannot by itself correct the
active table's negative-first half-cycle convention.

Do not respond by modifying `PLL_PHASE_COMP_DEG`. Before resuming Stage 1, the
owner must confirm whether commanded positive grid current is intended to use:

- `+sin(theta_interp+offset)`, as stated in the current contract; or
- the existing firmware `phase_cmd` convention, equivalent here to `-sin`.

Only after that polarity contract is fixed can a meaningful final reference
offset and residual be reported for the exact table path.

## Phase B — source of the reported 6250-count step

### Root cause

Buck feedforward is bipolar PWM centered at 6250 counts:

`Buck_PWM = 6250 +/- vref*6250/574`.

At zero voltage magnitude it is exactly 6250. The preliminary dry-run initialized
its previous compare to zero and counted the first enabled Buck command as a
mode transition. That produced:

`6250-0 = 6250 counts`.

It is an **enable/safe-off-to-Buck-neutral transition**, not a natural
Buck/Boost mode transition and not a 2P2Z-generated step.

### Natural active transition ownership

The active selector uses Vdc=574 and +/-20-count hysteresis:

- Buck enters Boost above 594 counts;
- Boost returns to Buck below 554 counts.

On a mode change, `CURRENT_LOOP_StateClear()` clears Boost/Buck error and duty
states before the new mode computes. Physical bumpless initialization is not
called because `PHYSICAL_BUMPLESS_ENABLE=0`.

In Boost:

- PG1 is a static half-cycle polarity override: 0 or 12500;
- PG2 is `Boost_PWM + Boost 2P2Z correction`, clamped to 0..10625.

In Buck:

- PG1 is complementary Buck PWM around the 6250 center;
- PG2 is zero;
- Buck 2P2Z coefficients are not active.

Exact ideal-feedback replay found the largest natural handoff in the captured
trace at Boost-to-Buck near the low hysteresis boundary:

- previous PG1DC: 0;
- new PG1DC: 446;
- PG2DC remains 0;
- command jump: **446 counts = 3.568% of a 12500-count period**;
- equivalent gate-width change: **892 ns** at the 500 MHz PWM clock.

The C cross-check reported a conservative maximum of 660 counts when including
boundary/sample bookkeeping. The retained tick-resolved trace identifies 446
counts for the actual state-change row. Neither supports classifying 6250 as a
natural mode-handoff result.

### Transition trace

The trace records 100 ticks before and after the first eight natural events,
including time, reference, feedback, error, modes, state-clear marker,
feedforward, mapped duty, PG1DC, PG2DC, override, clamp and gate enable:

[Natural transition trace](<D:\Desktop\DAC test\FULL_CONTROLLER_STAGE1_CONTINUATION_PLOTS\mode_transition_trace_100ticks.csv>)

## Forced transition phase/amplitude audit

The following values intentionally force a Buck/Boost ownership change at the
listed phase. They are not transitions the active hysteresis would naturally
request when far from `|vref|=574`.

| Phase | Small 100-count ref | Medium 300-count ref | Classification |
|---:|---:|---:|---|
| 0 deg | 3920-count max jump | 3920 | forced outside natural boundary |
| 30 deg | 7182 | 7341 | illegal/uncommanded forced handoff |
| 60 deg | 8946 | 9105 | illegal/uncommanded forced handoff |
| 90 deg | 9284 | 9443 | illegal/uncommanded forced handoff |
| 120 deg | 8737 | 8896 | illegal/uncommanded forced handoff |
| 150 deg | 6313 | 6472 | illegal/uncommanded forced handoff |
| 180 deg | 3920 | 3920 | forced outside natural boundary |

Amplitude changes only the first Boost correction after states are cleared:
approximately 79 counts at small reference and 238 counts at medium reference.
Mode selection itself is voltage-reference driven, not current-amplitude driven.

[Phase/amplitude summary CSV](<D:\Desktop\DAC test\FULL_CONTROLLER_STAGE1_CONTINUATION_PLOTS\mode_transition_phase_amplitude_summary.csv>)

These forced results demonstrate why the mode state must never change at an
arbitrary phase. They do not imply that the active hysteresis produces those
jumps in normal operation.

## Existing disabled bumpless path

An existing compile-time path is present under `PHYSICAL_BUMPLESS_ENABLE`:

1. convert the old physical Buck or Boost duty to normalized `M_counts`;
2. map that physical command into the new mode;
3. initialize the new mode's duty/controller compensation states;
4. clamp Boost compensation to +/-2500;
5. hold the mapped physical duty for two 40 kHz ticks.

It is disabled in the active build. The source does not document a definitive
reason for disabling it, so this audit does not invent one. If enabled, its
intended effect would be continuity of physical conversion ratio/duty across
the mode handoff, not a change to reference phase. It was not enabled or tested.

## Gate-command impact

- Natural 446-count command change is legal compare data and about 3.57% duty.
- At 500 MHz it changes commanded pulse width by about 0.892 us in the next
  25 us PWM period.
- Hardware complementarity and 100-count/200 ns dead time remain active; a
  compare jump does not itself create H/L overlap.
- It can still cause a converter transient and inductor di/dt once a plant is
  attached; no plant-level safety conclusion is made here.
- The 6250 enable transition represents a 50% compare change and must be audited
  separately as startup/override release, not mode handoff.

## Command-level PASS/FAIL classification

| Check | Natural handoff result |
|---|---|
| Compare outside legal range | PASS; none |
| Saturation/clamp at ideal-feedback natural handoff | PASS; none |
| Unexpected polarity inversion | PASS in natural trace |
| One-tick full-scale jump | PASS; maximum observed far below full scale |
| Complementary overlap | structurally prevented by hardware complementary mode |
| Dead-time violation | no register change; configured 200 ns remains |
| Arbitrary-phase forced handoff | FAIL by design; active selector must prevent it |
| Safe-off to Buck-neutral 6250 release | OPEN startup issue, not mode PASS |

## Can Stage 1 proceed to simplified plant?

**No, not yet.** Mode-transition source localization is complete and natural
handoff commands are legal, but Phase A exposed an unresolved 180-degree
current-reference polarity contract. Before MODE A/B/C resumes or a plant is
added:

1. explicitly approve the intended signed current direction relative to Grid;
2. compare exact table against the matching `+sin` or `-sin` ideal;
3. revalidate the independent reference offset in that signed frame; and
4. separately audit safe-off/override release into Buck's 6250 neutral compare.

The mode-transition issue remains documented without changing firmware.

