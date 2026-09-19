# Active inline Stage-3D PLL feasibility verification

## Scope and status

Firmware was not modified. The source of truth is the compiled branch entered
from `_ADCAN1Interrupt()`, with
`DIAG_LOAD_STAGE_SELECT=DIAG_LOAD_STAGE_CURRENT_LOOP`. The callback model
`pll_lock_test.va` is retained only as a historical reference.

Baseline input: 60 Hz sine, 1000 centered ADC counts peak, zero noise, cold
start, 1.0 s replay. The new model is `pll_stage3d_active_test.va`.

## 1. Exact active inline flow

1. Every AN1 interrupt reads `ADCBUF1` into `adcbuf1_snapshot` and
   `valchannel_AN1` before any diagnostic branch.
2. Raw-rate counters/timing diagnostics update at 40 kHz and
   `adc1_raw_latest=valchannel_AN1`.
3. `an1_pll_decim_cnt` increments. Counts 1--15 clear the interrupt flag and
   return immediately.
4. Count 16 resets the decimator and enters the 2.5 kHz Stage-3D workload.
5. Centering occurs here:
   `v_in_count=valchannel_AN1-GRID_ADC_CENTER`.
6. Grid hysteresis/ZC sequence and recent-ZC period logic update at 2.5 kHz.
7. Q30/Q12 SOGI alpha and beta update with signed Q30 rounding.
8. Raw SOGI magnitude is calculated. Magnitude-valid hysteresis/debounce
   updates `an1_signal_valid`.
9. For cold start, SOGI warms for 50 decimated samples. The code then waits
   for a positive input crossing, sets `pll_idx=360-4=356`, clears PI state,
   sets nominal step and asserts `phase_init_complete`. A 250-sample wait
   timeout falls back to the SOGI scan.
10. Before phase initialization completes, the ISR clears the flag and returns
    after SOGI/validity work; PI/NCO do not update.
11. After initialization, the Park detector uses interpolated Q15
    `sin(theta+4)` and `cos(theta+4)`.
12. Stage-3D selects fixed normalization:
    `raw_err=(vq_counts*25326)>>15`, clamped to +/-1000.
13. The fast `/64` LPF updates with a carried signed remainder.
14. No slow LPF is updated in this branch.
15. PI uses the fast error. The integrator increment is explicitly divided by
    16 with signed truncation toward zero, then bounded and anti-windup gated.
16. Q20 P/I step terms form the target; target clamps to 55--65 Hz.
17. Only the acquisition slew limit is used; no locked-mode slew branch exists
    here.
18. The table/fraction NCO updates and wraps, then publishes its Q32 phase
    mailbox.
19. Debug/statistics update, the interrupt flag clears, and the ISR returns.
    `PLL_Task_Run()` and `ADC1_channel_AN1_CallBack` are not reached.

## 2. Resolved macros and compiled branches

| Item | Resolved active value/branch |
|---|---|
| `DIAG_LOAD_STAGE_SELECT` | 4, `DIAG_LOAD_STAGE_CURRENT_LOOP`; inline branch compiled |
| `DIAG_LOAD_STAGE_SELECT >= FULL_PLL` | true |
| `DIAG_STAGE3_SUBSTAGE_SELECT` | 4, fixed-normalization Stage-3D |
| `DIAG_STAGE3D_SIMPLE_PHASE_INIT_ENABLE` | 1 |
| `DIAG_STAGE3D_START_MODE` | positive-ZC start (1) |
| SOGI warmup / ZC timeout | 50 / 250 decimated samples |
| `DIAG_STAGE3D_NORM_GAIN_Q15` | 25326 |
| integrator scale-down | enabled, shift 4 (divide 16) |
| integrator disabled/frozen-NCO tests | disabled |
| `DIAG_STAGE4_PHASE_CHAIN_ENABLE` | 1; Q32 mailbox published |
| `PLL_CONTROL_ENABLE` | 1 from `parameter.h`; local fallback does not override |
| raw/PLL rate | 40000 / 2500 Hz |
| decimation | 16 |
| ADC center | 1986 counts |
| SOGI Q | coefficients Q30, state Q12 |
| SOGI coefficients | A1 -1919669515, A2 867878708, B0 102931558, QB0 7760857, QB1 15521713, QB2 7760857 |
| phase detector compensation | 4 deg |
| output phase compensation | 0 deg; inline output/mailbox is raw NCO phase |
| error clamp / fast LPF | +/-1000; `/64` with residual |
| slow LPF | not executed |
| nominal/min/max NCO step | 18119393 / 16609443 / 19629342 |
| nominal step conversion | integer `PLL_STEP_PER_HZ_Q=301989` |
| P/I Q20 gains | 1512043944 / 3023835 |
| integrator raw limit | +/-837766 |
| acquisition slew | 12079 step-Q/update, approximately 100 Hz/s |
| locked slew | defined as 7247 but not selected by inline Stage-3D |
| input-valid ON/OFF | raw SOGI magnitude 300/200 counts in Q12 |
| valid/lost debounce | 75 / 8 samples at 2.5 kHz = 30.0 / 3.2 ms |
| ZC validity | separate 38--46 sample period and 50-sample recent timeout; not part of `an1_signal_valid` itself |
| legacy lock profile | WIDE macros exist but their phase/lock machines are not executed in this branch |
| legacy `phase_ok`, `pll_locked` | no write sites in inline branch; remain initialized zero |

## 3. Timing and state ownership

| Operation | 40 kHz | 2.5 kHz | Conditional | State written |
|---|:---:|:---:|---|---|
| Read `ADCBUF1` | Yes | No | every AN1 IRQ | raw snapshot/latest |
| Raw timing/counters | Yes | No | selected load-stage build | debug counters |
| Decimator increment | Yes | No | every AN1 IRQ | decim counter |
| Center subtraction | No | Yes | decim hit | `v_in_count` |
| ZC event/period/recent-valid | No | Yes | FULL_PLL | ZC sequence/age/valid |
| SOGI alpha/beta | No | Yes | stage >= SOGI | all SOGI histories |
| Magnitude input-valid | No | Yes | FULL_PLL | valid/lost counters and flag |
| Phase-init warmup/wait | No | Yes | until complete | init counters, theta, PI reset |
| Park/fixed normalization | No | Yes | after init | vq/raw error |
| Fast LPF | No | Yes | after init | error plus residual |
| Slow LPF | No | No | absent | none |
| PI/integrator/anti-windup | No | Yes | after init | integrator/target |
| Slew and NCO | No | Yes | after init | step/theta |
| Q32 mailbox publish | No | Yes | Stage 4 enabled | phase mailbox/sequence |
| `phase_ok`/`pll_locked` | No | No | absent | remain zero |
| Interrupt clear/return | Yes | Yes | skip or completed branch | IRQ flag |

The code physically resides inside a 40 kHz ISR, but SOGI, validation, PI and
theta states execute only on every 16th entry.

## 4. Exact input-valid behavior

- Source: un-clamped SOGI magnitude
  `max(abs(alpha),abs(beta)) + min(abs(alpha),abs(beta))/2` in Q12.
- No raw 100-sample max/min amplitude window is used.
- ON threshold: 300 equivalent counts; must remain above it for 75 consecutive
  2.5 kHz updates.
- OFF threshold: 200 equivalent counts; must remain below it for 8 consecutive
  2.5 kHz updates.
- This is hysteresis plus debounce.
- Recent ZC is tracked independently. It is required by downstream Stage-4
  readiness, but not by the assignment of `an1_signal_valid`.
- When invalid, the inline branch does **not** reset or freeze SOGI, PI, NCO or
  phase initialization. Those states continue according to their normal
  init/run conditions. Only ZC event qualification/readiness is blocked.
- Therefore callback conclusions saying invalid input resets the entire PLL do
  not apply to this active path.

## 5. Old model versus active inline Stage-3D

| Item | Old `pll_lock_test.va` | Active inline Stage-3D | Impact |
|---|---|---|---|
| Entry | callback abstraction | `_ADCAN1Interrupt` inline branch | old model is not active firmware path |
| Raw/PLL timing | 40 kHz then /16 | same rates | headline rates agree |
| Input-valid | raw 100-sample detector; optional bypass | SOGI magnitude 300/200, 75/8 | completely different dropout/recovery |
| Invalid behavior | reset SOGI/PLL when non-bypass | no PLL/SOGI reset | acquisition behavior changes |
| Phase init | disabled by old model default or 100-sample scan | 50-sample warmup then positive ZC | cold-start phase changes |
| SOGI | floating approximation | Q30/Q12 signed rounding | quantization differs |
| Normalization | measured magnitude division | fixed Q15 gain 25326 | amplitude dependence changes |
| Fast filter | floating `/64` | integer `/64` plus carried remainder | removes integer dead zone |
| Slow filter | present | absent | legacy phase criteria unavailable |
| PI integrator | full increment | error increment divided by 16 | substantially weaker I action |
| NCO | real angle | integer table/fraction step | timing/phase quantized |
| Lock criteria | legacy phase/lock machines | not executed | `locked` cannot assert |
| Phase compensation | 4 deg parameter | active 4 deg | value agrees; optimum does not transfer |
| Output phase | real `sin(theta)` | interpolated table/NCO mailbox, 0 output comp | phase must be remeasured |
| Early return | none | returns before registered callback | downstream callback writes never occur |

## 6. 60 Hz / 1000-count baseline

The numerical replay used the exact 40 kHz sampling and `/16` state schedule,
firmware SOGI rounding/state order, Stage-3D validity, positive-ZC init, fixed
normalization, LPF residual, scaled integrator, Q20 gains, clamps, slew and
integer NCO geometry. The new Verilog-A model exposes the same state topology;
its portable real SOGI arithmetic should be checked in SIMPLIS against this
integer replay before sign-off.

| Metric | Result |
|---|---:|
| input valid asserted | 32.0 ms |
| phase initialized | 33.6 ms |
| input valid at 1 s | Yes |
| phase initialized at 1 s | Yes |
| final PLL frequency | 59.9989 Hz |
| observed frequency range after init | 59.9846--60.2262 Hz |
| final fast error | -1 count |
| slow error | not implemented / 0 debug output |
| `phase_ok` | 0, structurally never written |
| `pll_locked` | 0, structurally never written |
| lock drop | not applicable; lock never asserts |

The NCO and error loop converge and remain continuous, but the requested
legacy definition of PASS item 9/10 cannot pass because the inline branch has
no corresponding state machines. For the full controller, the relevant
readiness signal is the separate Stage-4 phase-chain gate, not `pll_locked`.

## 7. Waveform and phase results

Last 20 positive-going crossings in the 1 s integer replay:

| Signal pair | Average phase | Interpretation |
|---|---:|---|
| Grid -> SOGI alpha | +0.173 deg | alpha lags grid |
| SOGI alpha -> beta | +90.027 deg | beta lags alpha; quadrature is normal |
| Grid -> PLL output | **-4.498 deg** | PLL leads grid |

Alpha and beta are stable bipolar sine waves. Theta wraps continuously after
initialization, and the PLL output is a complete positive/negative sine. No
periodic reset was observed. Phase sign is positive for lag and negative for
lead, with linear positive-ZC interpolation and wrap to `[-180,+180)`.

The grid-to-PLL value is an offline integer replay result, not a new SIMPLIS
measurement. Direct SIMPLIS measurement from the new `.va` file remains the
final cross-check for this baseline.

## 8. Pass/fail and next action

| Requirement | Result |
|---|---|
| 60 Hz input and bipolar alpha/beta | PASS |
| approximately 90-degree alpha/beta | PASS |
| continuous theta and full PLL sine | PASS |
| frequency/error convergence | PASS for feasibility |
| `phase_ok` asserts | FAIL by construction |
| `pll_locked` asserts/stays locked | FAIL by construction |
| active-path equivalence established | PARTIAL until SIMPLIS `.va` cross-check |

The old 15.42-degree compensation must not be reused. With active 4 degrees,
the new baseline is about -4.50 degrees, so a new compensation sweep will be
required only after the new Verilog-A baseline agrees with the integer replay.

Full-controller simulation Phase 2 should **not begin yet**. First resolve the
meaning of lock for this build: either formally accept and verify the Stage-4
phase-chain readiness contract, or select an intended firmware configuration
that actually executes the legacy `phase_ok/pll_locked` machines. This is an
architecture decision and is not changed in this verification round.
