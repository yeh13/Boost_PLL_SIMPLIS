# Full controller dry-run simulation architecture plan

## Scope

This document is a read-only architecture audit and staged simulation plan.
It does not authorize power-up, firmware edits, control-parameter changes, or
connection to a switching power stage. `adc1.c` was not modified.

The source tree is a diagnostic/integration build, not a clean single-path
production controller. The future model must first freeze and record the exact
preprocessor configuration; function names alone are insufficient to identify
the executable path.

## 1. Phase 1 completion gate

PLL feasibility verification is **not yet complete against the currently
compiled source path**. Two audit findings invalidate treating the earlier
callback-only model as timing-exact firmware verification:

1. `DIAG_LOAD_STAGE_SELECT=DIAG_LOAD_STAGE_CURRENT_LOOP` (value 4). Therefore
   `_ADCAN1Interrupt()` takes the inline Stage-3D/Stage-4 branch, decimates the
   40 kHz AN1 stream by 16, executes its inline SOGI/PLL, publishes the Q32
   phase mailbox, and returns. It does not reach `PLL_Task_Run()` and therefore
   does not invoke the registered `ADC1_channel_AN1_CallBack` in this build.
2. `parameter.h` defines `PLL_CONTROL_ENABLE=1` before `adc1.c` reaches its
   `#ifndef PLL_CONTROL_ENABLE` fallback. The active value is therefore 1, not
   0.

The inline Stage-3D PLL also differs materially from the callback model:

- input-valid uses SOGI raw magnitude with 300-count ON, 200-count OFF, 75
  valid confirmations and 8 lost confirmations at 2.5 kHz;
- phase initialization uses 50 SOGI samples then a positive grid crossing;
- Stage-3D fixed normalization gain is Q15 `25326` rather than runtime
  division by measured magnitude;
- the integrator increment is scaled down by 1/16;
- its LPF carries signed division remainder;
- the Stage-3D state and phase mailbox are the states consumed downstream.

Before Phase 2 implementation, create a timing-exact model of this inline path
and complete:

- non-bypass valid/invalid acquisition, dropout, reset and recovery;
- 2.5 kHz fixed-point alpha/beta, phase, frequency and lock behavior;
- 55--65 Hz and required amplitude/corner robustness;
- mailbox publication and 16-step 40 kHz phase interpolation;
- explicit comparison against the callback-based feasibility reports.

Phase 1 ends with a report and no firmware change. Phase 2 implementation must
not start until this discrepancy is resolved or a different intended build
configuration is explicitly selected.

## 2. Active timing architecture

| Rate/event | Active work and owner |
|---|---|
| PWM1 cycle, nominal 40 kHz | `PWM_Generator1_CallBack`; invokes `CONTROL_Scheduler40kHz`. With Stage-4 current-loop ownership active it returns before legacy PWM duty generation. |
| AN0 ADC event, nominal 40 kHz | `_ADCAN0Interrupt`; reads `ADCBUF0`, filters current, performs immediate ADC saturation/overcurrent protection, advances phase interpolation, runs Stage-4 safety gate and calls the real current loop. |
| AN1 ADC event, raw 40 kHz | `_ADCAN1Interrupt`; reads `ADCBUF1`, counts raw events and performs `/16` scheduling. |
| AN1 decimated event, 2.5 kHz | Inline Stage-3D SOGI/PLL, magnitude validity, ZC/event tracking, NCO update and Q32 mailbox publication. |
| Timer1, 40 ms | Diagnostic/statistics snapshot only; not a control-rate owner. |
| PWM carrier hardware | PG1/PG2 complementary waveform generation and dead-time insertion from the most recent duty/override registers. |

The requested timing principle is confirmed: current control and PWM command
updates are 40 kHz; SOGI/PLL remains 2.5 kHz. The controller simulation must
not run PLL math at 40 kHz. The 40 kHz consumer interpolates/extrapolates the
2.5 kHz phase mailbox.

Both AN0 and AN1 are configured for PWM1 Trigger1 (`ADTRIG0L=0x0404`). PWM1
Trigger1 is enabled from PG1 EOC. Exact ordering when both ADC completion flags
occur close together must be represented as an explicit scheduler ordering in
the dry-run model and tested for one-sample races.

## 3. Active control flow

```text
PWM1 EOC / trigger (40 kHz)
  +-> AN1 conversion -> _ADCAN1Interrupt
  |     +-> retain raw ADC
  |     +-> /16
  |           +-> fixed-Q30 SOGI
  |           +-> magnitude validity and grid ZC sequence
  |           +-> Stage-3D PLL/NCO
  |           +-> publish Q32 phase mailbox
  |
  +-> AN0 conversion -> _ADCAN0Interrupt
  |     +-> retain/filter current ADC
  |     +-> saturation and overcurrent checks
  |     +-> advance 40 kHz PLL phase interpolation
  |     +-> Stage-4 readiness/startup/fault gate
  |     +-> ADC1_channel_AN0_CallBack
  |           +-> select interpolated voltage command and polarity
  |           +-> generate current reference and apply soft-start
  |           +-> current error and Buck/Boost 2P2Z
  |           +-> mode mapping, clamps and PG overrides
  |           +-> write PG1DC/PG2DC
  |
  +-> PWM_Generator1_CallBack
        +-> CONTROL_Scheduler40kHz diagnostics
        +-> returns because AN0 owns Stage-4 current-loop outputs

PWM hardware
  +-> PG1H/PG1L complementary Buck leg
  +-> PG2H/PG2L complementary Boost leg
  +-> hardware dead time
  +-> PG7 SSR command/debug output
```

## 4. State ownership

| State/output | Writer/owner in active build | Reader/consumer |
|---|---|---|
| AN1 raw sample | `_ADCAN1Interrupt` | inline SOGI/PLL |
| SOGI histories, PLL PI/NCO, phase init | inline decimated `_ADCAN1Interrupt` | phase mailbox and readiness logic |
| `an1_signal_valid`, ZC age/event sequence | inline decimated AN1 path | Stage-4 safety gate |
| Q32 PLL mailbox | decimated AN1 path | 40 kHz interpolation path |
| interpolated phase/table index/polarity | AN0 40 kHz path | current-reference mapping |
| AN0 raw/filter | `_ADCAN0Interrupt` / AN0 callback | protection and current controller |
| current-controller histories | `ADC1_channel_AN0_CallBack` | next 40 kHz current update |
| startup state, run enable, soft-start | `DIAG_PLLThetaSafetyGateUpdate`, called from AN0 path | current-loop gate/reference scaling |
| fault latch | AN0 immediate protection and Stage-4 safety gate | every output gate |
| PG1DC/PG2DC and PG1 mode override | AN0 current loop | PWM hardware |
| PG7/SSR command | Stage-4 safety gate via `PG7_OutputSsrFlag` | PWM7/SSR pin |

Single-writer assertions are mandatory in the model. In particular, legacy
PWM1 duty calculation and legacy 2.5 kHz grid supervisor are compiled out of
ownership by the active Stage-4 selections.

## 5. ADC inputs

### AN1: grid/PCC voltage

- Raw 12-bit sample at 40 kHz, centered at `GRID_ADC_CENTER=1986`.
- Only every 16th sample enters the active SOGI/PLL.
- Active validity is derived from SOGI magnitude, not the callback's
  100-sample raw min/max window.
- Grid polarity/ZC sequence uses centered input and `20`-count hysteresis at
  the decimated point in the active inline branch.

### AN0: current feedback

- Raw 12-bit sample centered at 2048.
- Q0 integer IIR: `filt += (raw-filt)>>2`.
- Raw protection thresholds: ADC saturation below 45 or above 4050 for five
  samples; overcurrent outside 2048 +/-1500 for three samples.
- Current-loop reference and feedback remain ADC-count-domain quantities.

### DC voltage

The current source uses `const Vdc=574` rather than a live ADC channel in this
source path. Stage 1 should expose it as a configurable virtual input but hold
574 for firmware equivalence. A future live-Vdc ADC path must not be invented
without a source-level owner.

## 6. PLL and 40 kHz reference path

The simulation must reproduce the inline Stage-3D implementation, then its
downstream phase conversion:

1. fixed Q30 SOGI and Q12 histories;
2. magnitude-valid hysteresis/debounce;
3. positive-ZC initialization and timeout fallback;
4. Park detector at `theta + PLL_PHASE_COMP_DEG`;
5. fixed Q15 normalization and exact LPF remainder;
6. scaled PI/integrator, anti-windup, clamp and acquisition slew;
7. table/fraction NCO and wrap;
8. Q32 mailbox publication;
9. 16-step 40 kHz interpolation;
10. mapping to the 668-point full cycle, 334-point magnitude table and
    half-cycle `phase_cmd`.

Required invariants: monotonic phase modulo wrap, no mailbox tear, no more
than one intended full-index step per update unless documented, and no
polarity change away from the intended zero-cross region.

## 7. Current-loop path

At each AN0 update:

1. verify cached run gate and fault latch;
2. read interpolated `vref_cmd` and `phase_cmd`;
3. compute magnitude-based current reference from `VrefTable` using
   `IREF_FROM_VREF_GAIN_Q`;
4. multiply it by `grid_current_softstart_q15`;
5. apply the +/- half-cycle sign around ADC center 2048;
6. filter AN0 and calculate polarity-aware current error;
7. apply +/-3-count deadband;
8. select Buck/Boost with `CURRENT_LOOP_SelectModeWithHysteresis`;
9. clear controller histories on mode change; physical bumpless handoff is
   compiled off (`PHYSICAL_BUMPLESS_ENABLE=0`);
10. run the selected Q15 2P2Z equation using the exact coefficients in
    `adc1.c`;
11. add the compensator output to feed-forward `Buck_PWM` or `Boost_PWM`;
12. clamp and write duty/override state.

The model must preserve signed shift behavior, update order of error/duty
histories, saturation points, and the reset-on-mode-change discontinuity.

## 8. Duty, mode, PWM and gate mapping

- `BUCK_DUTY`: 50%-centered bipolar duty derived from `vref_cmd/Vdc`, clamped
  to 0..12500.
- `BOOST_DUTY`: `(vref-Vdc)/(vref+Vdc)` scaled by 12500, then clamped.
- Buck mode: PG1 overrides disabled; PG1 complementary PWM carries Buck duty;
  PG2 duty is zero.
- Boost mode: PG1 becomes a static polarity leg through `OVRDAT=0x2` or
  `0x1`; PG2 complementary PWM carries Boost duty.
- PG1/PG2 period registers are 12499; software duty limits are 12500/10625.
  The simulator must explicitly define register endpoint semantics and flag
  commands outside 0..PER before deciding whether 12500 is a legal 100% code.
- PG1 and PG2 use complementary mode (`PGxIOCONH=0x000C`) with
  `PGxDTL=PGxDTH=100`. Convert this count to time only after the auxiliary PWM
  clock is established; do not guess nanoseconds.
- Output updates request synchronization using `PG1STAT.UPDREQ` and
  `PG2STAT.UPDREQ` where written. Model register-update latency at the PWM
  boundary rather than changing gates instantaneously in the middle of a
  carrier.

The dry-run gate model must output logical `PG1H`, `PG1L`, `PG2H`, `PG2L` and
`PG7/SSR`, plus raw duty and override bits. It must distinguish a complementary
PWM pair from a static polarity override.

## 9. Startup and zero-cross sequence

Active Stage-4 state order:

```text
STATE_PLL_LOCKING
  -> STATE_WAIT_GRID_VALID
  -> STATE_WAIT_ZC_TO_COMMAND_SSR
  -> STATE_WAIT_NEXT_ZC_TO_START_PWM
  -> STATE_GRID_CURRENT_RUN
  -> STATE_FAULT on any hard failure
```

- Readiness requires the active AN1/PLL phase chain and valid recent ZC.
- Grid validity is confirmed for 1000 AN0/40 kHz samples (25 ms).
- At the next accepted ZC the SSR command is issued. In the current
  `GRID_TEST_MODE_NO_SSR=1` build, the physical SSR command remains off.
- PWM becomes allowed only after the configured following ZC event.
- Current soft-start begins at zero and increments Q15 scale by 16 per 40 kHz
  update, reaching unity in about 51.2 ms.
- Loss of readiness or ZC timeout transitions to fault and forces all outputs
  off.

The simulation must retain event ordering within each 40 kHz tick: sample,
protection, phase interpolation, safety-state update, current reference,
controller, duty register update, then PWM-boundary gate application.

## 10. Protection and shutdown path

Immediate/latched cases to reproduce:

- AN0 ADC saturation debounce;
- AN0 overcurrent debounce;
- PWM hardware `FLTEVT` detection;
- AN1 invalid, stale/missing ZC and phase-chain invalidity;
- mailbox/interpolation validity failures;
- run-gate loss while output is active;
- explicit `GRID_SetFault` behavior.

`GRID_SetFault` latches the source, clears current-controller history and
soft-start, disables SPWM/SSR/current enable, calls `PowerPWM_AllOff`, forces
PG1/PG2 override data low and enters `STATE_FAULT`. The dry-run must verify the
maximum shutdown latency in both control ticks and PWM carrier edges.

## 11. Recommended simulation stages

### Phase 1: PLL closure

Build the exact inline 2.5 kHz model and non-bypass validity tests described in
section 1. Stop and report.

### Dry-run Stage 1A: scheduler and state machine

No controller and no plant. Feed virtual AN1/AN0, reproduce mailbox,
interpolation, startup, ZC and faults. Keep PG duties at zero. Prove ownership
and ordering.

### Dry-run Stage 1B: controller command/gate model

Add reference generation, current filter, 2P2Z, mode mapping, duty registers,
PWM carrier, override semantics and dead time. Current feedback remains a
scripted virtual ADC. Produce the exact logical gate sequence; SSR stays a
logical output and no power circuit is connected.

### Dry-run Stage 2: simplified current plant

Insert a discrete averaged plant:

```text
duty/mode/polarity -> limited first-order or RL current plant
                   -> ADC quantization/offset/noise
                   -> AN0 current feedback
```

Test zero, small sine, medium sine and rated test reference. Sweep plant gain,
time constant, ADC offset/noise and one-sample delay. Observe tracking,
transient, error, saturation, zero crossing and Buck/Boost transitions.

### Dry-run Stage 3: SIMPLIS switching plant

Only after Stages 1 and 2 pass, replace the averaged plant with MOSFETs,
inductors, capacitors and load. Preserve the already verified controller and
gate-timing model as the reference oracle.

## 12. Required debug/trace signals

### Timing and ownership

`tick_40k`, `tick_pll_2p5k`, AN0/AN1 raw samples, ISR ordering, decimation
counter, mailbox sequence, mailbox age, interpolation step, PG output owner.

### PLL/grid

centered AN1, alpha, beta, magnitude, validity counters/flag, ZC sequence/age,
phase-init state, vq/raw/filtered error, integrator, target/actual step, theta,
frequency, phase-chain ready.

### Current control

raw/filtered AN0, signed reference, soft-start scale, error/deadband output,
selected mode, 2P2Z histories/terms/output, base duties, final duties, clamp
flags and mode-transition reset event.

### Startup/protection

state, ready conditions, SSR/SPWM/current enables, ZC consumed sequence,
timeouts, fault source/latch, all-off event and shutdown latency.

### Gates

PG1/PG2 period, duty shadow/active values, override enables/data, update
request, PG1H/L, PG2H/L, dead-time intervals, PG7/SSR and polarity command.

## 13. Automated pass/fail checks

1. No complementary pair has H and L high simultaneously.
2. Both turn-on transitions include at least the configured dead-time count.
3. Duty commands stay within defined software and register limits.
4. All gates and SSR are off before the startup gate allows output.
5. Normal output cannot start without valid PLL phase chain and recent ZC.
6. SSR command precedes PWM permission by the configured ZC sequence.
7. Polarity changes only at the intended half-cycle transition and never
   creates an overlap pulse.
8. Any fault forces all logical gates off within the specified maximum
   control/PWM latency and prevents automatic restart from `STATE_FAULT`.
9. State transitions do not create sub-cycle or one-carrier abnormal pulses.
10. Buck/Boost mode transitions respect the configured duty-step bound; the
    current build's history clear and disabled bumpless feature must be tested
    explicitly, not assumed safe.
11. Current-reference step and index-skip counters remain within limits.
12. Mailbox data is coherent and never stale beyond the allowed 40 kHz ticks.
13. PLL executes exactly once per 16 raw AN1 samples; current loop executes
    once per AN0 sample.
14. No secondary writer changes PG1DC/PG2DC while AN0 owns outputs.

## 14. Files/functions the future model must reproduce

| Source | Required content |
|---|---|
| `mcc_generated_files/adc1.c` | `_ADCAN1Interrupt`, inline Stage-3D PLL, mailbox/interpolation functions, `DIAG_PLLThetaSafetyGateUpdate`, `_ADCAN0Interrupt`, `ADC1_channel_AN0_CallBack`, current-loop helpers, protection and all-off functions. |
| `mcc_generated_files/parameter.h` | build switches, state enums, output ownership declarations and PLL/current integration selections. |
| `mcc_generated_files/pwm.c` | `PWM_Initialize`, PWM1 callback ownership return, base Buck/Boost equations and PWM register semantics. |
| `mcc_generated_files/pll_pwm_duty_tables.h` | 668-point phase/mode/Buck/Boost mappings used by Stage-4 diagnostics and reference mapping. |
| `mcc_generated_files/pin_manager.c/.h` | logical-to-physical pin mapping for final gate/SSR trace labels. |
| `mcc_generated_files/mcc.c`, `system.c`, `clock.c` | initialization order and PWM/ADC clock assumptions. |
| `main.c` | Required for final call-graph proof, but absent from the current workspace even though the build map references `main.o`; restore or locate it before implementation sign-off. |

The existing `.elf/.map` confirms the named initialization and callback
symbols were linked, but stale build artifacts must not substitute for the
missing source or for a fresh preprocessed configuration manifest.

## 15. Deliverables before any power-stage work

1. Phase-1 timing-exact inline PLL report.
2. Frozen macro/configuration manifest and preprocessed active-call-path map.
3. Deterministic dry-run scheduler with event-order specification.
4. Stage-1 gate trace plus automated safety-check report.
5. Stage-2 averaged-plant tracking and transition report.
6. Only then, a separate approval/plan for SIMPLIS switching integration.

No firmware modification or power-up step is part of this plan.
