# Full Controller Dry-Run Stage 1 Final

## Final judgment

**STAGE1_LOGIC_PASS**

This judgment applies only to the firmware-equivalent reference, current-loop
math, mode command and digital gate-command model. No power plant or energized
hardware was used. The physical blockers listed at the end remain open.

The replay uses the already-audited active path:

`theta_after (2.5 kHz) -> causal 40 kHz interpolation -> +3.55 deg reference offset -> exact 668/334 table map -> legacy signed-current frame`

The firmware signed-current truth is approximately
`-sin(theta_interp + 3.55 deg)`. The table, PLL, interpolation intent, phase
offset and startup root cause were not retuned or re-audited here.

## Test configuration

- Sample rate: 40 kHz.
- Input record: 6667 samples, approximately 0.1667 s / ten 60 Hz cycles.
- ADC center: 2048.
- AN0 filter: `y += (adc-y)>>2`.
- Current deadband: +/-2 counts (`-3 < error < 3` becomes zero).
- Stage-4 soft-start: +16 Q15 counts per 40 kHz tick to 32768.
- Buck/Boost thresholds: initial 574; Buck-to-Boost `vref>594`; Boost-to-Buck
  `vref<554`.
- Buck command: feedforward only.
- Boost correction: active 2P2Z, limited to +/-2500 counts.
- PWM compare limits: PG1 0..12500; controlled PG2 0..10625.
- Dead time: 100 auxiliary-clock counts = 200 ns at 500 MHz.

## Case summary

Positive- and negative-half correction failure counts are zero for every case.
The gate checks also returned zero overlap, illegal compare and illegal override
events in every valid case. Minimum enabled dead time was 100 counts / 200 ns.

| Case | Feedback definition | Max raw error | Max filtered error | Pos/neg correction failures | Boost limiter hits | Occupancy | Buck bounded | Max natural step | Gate overlap | Result |
|---|---|---:|---:|---:|---:|---:|---|---:|---:|---|
| MODE B ideal tracking | raw feedback = exact active i_ref | 0 | 40 | 0 / 0 | 0 | 0% | PASS | 446 | 0 | PASS |
| MODE A zero feedback | ADC = 2048; 0.25-scale nonzero reference | 229 | 229 | 0 / 0 | 0 | 0% | PASS | 2401 | 0 | PASS, disturbance observation |
| MODE C 0.8x | feedback = 0.8 i_ref | 183 | 187 | 0 / 0 | 0 | 0% | PASS | 2043 | 0 | PASS, disturbance observation |
| MODE C 1.2x | feedback = 1.2 i_ref | 183 | 187 | 0 / 0 | 1104 | 16.56% | PASS | 446 | 0 | PASS with saturation observation |
| Phase lag -2 deg | delayed exact i_ref | 52 | 76 | 0 / 0 | 0 | 0% | PASS | 446 | 0 | PASS |
| Phase lag -5 deg | delayed exact i_ref | 103 | 124 | 0 / 0 | 0 | 0% | PASS | 446 | 0 | PASS |
| Phase lag -10 deg | delayed exact i_ref | 180 | 203 | 0 / 0 | 0 | 0% | PASS | 460 | 0 | PASS |
| Phase lead +2 deg | advanced exact i_ref | 58 | 29 | 0 / 0 | 0 | 0% | PASS | 446 | 0 | PASS |
| Phase lead +5 deg | advanced exact i_ref | 105 | 80 | 0 / 0 | 0 | 0% | PASS | 446 | 0 | PASS |

“Raw/filtered error” in this table means feedback minus reference in the ADC
tracking domain. The controller subsequently applies the active half-cycle sign
contract. A nonzero tracking error is not itself a correction-sign failure.

## MODE B: raw versus filtered tracking

The virtual raw ADC exactly follows the integer active current reference, so its
maximum and RMS raw tracking errors are both zero. The controller does not use
that raw value directly: the active AN0 IIR introduces a causal lag while the
reference changes.

After soft-start reaches unity:

- maximum absolute filtered lag: **40 counts**;
- RMS filtered lag: **18.415 counts**;
- 60 Hz DFT phase of filtered feedback relative to exact i_ref: **-1.594 deg**.

This lag generates a bounded Boost correction (maximum 206 counts) with no
limiter hit or runaway. It is filter dynamics, not a signed-frame failure.

## Exact Boost 2P2Z replay

The active coefficients are:

```
A = [30010, 2668]
B = [26000, -32000, 9640]
```

For every Boost sample the replay follows the firmware ordering:

```
acc = 26000*e[n]
    - 32000*e[n-1]
    +  9640*e[n-2]
    + 30010*u[n-1]
    +  2668*u[n-2]

raw_u = acc >> 15
limited_u = clamp(raw_u, -2500, +2500)
duty_cmd = Boost_feedforward + limited_u
PG2DC = clamp(duty_cmd, 0, 10625)
u_state[n] = PG2DC - Boost_feedforward
```

The signed right shift is an arithmetic Q15 shift; there is no added half-LSB
rounding. `e[n-1]`, `e[n-2]`, `u[n-1]` and `u[n-2]` are captured before the
current calculation in each CSV. After calculation, error history shifts and
the realized post-duty-clamp compensation becomes the new `u[n]` state.

There is no separate integrator anti-windup branch. Bounded behavior comes from
the +/-2500 correction limiter, final duty clamp, use of realized compensation
as state, and clearing all error/output histories on a mode change.

Limiter occupancy by case is zero except MODE C 1.2x: 1104/6667 samples, or
**16.56%**. This is a **SATURATION OBSERVATION**. Its externally prescribed
feedback never responds to duty, so it proves neither closed-loop stability nor
instability. Stability remains a plant-stage question.

## MODE A and MODE C correction direction

In the positive half-cycle the active error is `iref_cmd-filtered_adc`. In the
negative half-cycle it is `filtered_adc-iref_cmd`. Therefore positive controller
error consistently means “increase magnitude in the selected direction.”

After full soft-start and away from the zero band:

- MODE A zero feedback has positive mean error in both halves (about 160 and
  158 counts), requesting more magnitude.
- MODE C 0.8x has positive mean error in both halves (about 120 and 122 counts).
- MODE C 1.2x has negative mean error in both halves (about -122 and -117
  counts), requesting less magnitude.

Short extrema in raw error caused by the AN0 filter are classified against the
filtered current actually consumed by the controller. The resulting automated
correction-direction failure count is zero in each half-cycle and case.

## Buck behavior

Buck remains a voltage/feedforward command, not a Boost-style 2P2Z current loop:

```
modulation = vref_abs * 6250 / 574
phase_cmd=1: PG1DC = 6250 + modulation
phase_cmd=0: PG1DC = 6250 - modulation
PG1DC = clamp(PG1DC, 0, 12500)
PG2DC = 0
```

All replayed Buck commands remained inside the active compare range and matched
the signed half-cycle mapping. Buck is therefore PASS for feedforward mapping
and bounded commands. It is deliberately not judged by Boost closed-loop
current-regulation criteria.

## Natural Buck/Boost transitions

Only active hysteresis transitions were allowed. No arbitrary forced transition
was inserted. In MODE B ideal tracking, ten events of each type were observed:

| Half-cycle | Direction | Active threshold | Maximum compare-register step |
|---|---|---:|---:|
| negative | Buck to Boost, rising magnitude | `vref>594` | 327 |
| negative | Boost to Buck, falling magnitude | `vref<554` | 446 |
| positive | Buck to Boost, rising magnitude | `vref>594` | 329 |
| positive | Boost to Buck, falling magnitude | `vref<554` | 446 |

Thus the nominal full signed-frame replay confirms the previous **446-count**
maximum.

The no-plant disturbance cases differ: MODE A reaches 2401 counts and MODE C
0.8x reaches 2043 counts. At mode entry the active firmware clears the 2P2Z
history and `PHYSICAL_BUMPLESS_ENABLE` is zero, while the prescribed current
feedback does not respond to the old duty. These larger cross-register changes
are bounded and legal and do not create an override or gate pulse violation.
They should not replace 446 counts as the nominal ideal-tracking transition
result, but they are retained in the CSV as controller-disturbance observations.

## Logic-level gate validation

Across all valid cases:

| Check | Result |
|---|---:|
| PG1 same-leg H/L overlap | 0 |
| PG2 same-leg H/L overlap | 0 |
| Minimum enabled dead time | 100 counts / 200 ns |
| Compare outside active range | 0 |
| Illegal H/L override combination | 0 |
| Narrow-pulse anomaly | 0 |
| Zero-cross polarity anomaly | 0 |
| Mode-transition gate anomaly | 0 |

In Buck mode both generators use legal complementary commands. In Boost mode
PG1 is statically selected with `OVRDAT=0x1` or `0x2`, never `0x3`, while PG2 is
the bounded PWM leg. Changing half-cycle polarity does not assert both gates on
the same leg.

The prior startup contract is adopted without reinterpretation: forced-off to
PG1DC=6250 is startup release into Buck 50% complementary operation, not a
Buck/Boost transition. Its existing result remains overlap zero, minimum dead
time 200 ns and no narrow pulse. This is only a logic/PWM timing result.

## PASS criteria disposition

| Criterion | Disposition |
|---|---|
| Exact legacy signed reference replay | PASS |
| MODE B no controller runaway | PASS |
| MODE A correction direction | PASS |
| 0.8x / 1.2x correction direction | PASS |
| Positive/negative half consistency | PASS |
| Boost state/output bounded by active limits | PASS |
| Buck feedforward bounded and mapped correctly | PASS |
| Natural transition has no illegal compare | PASS |
| PG1/PG2 overlap zero | PASS |
| Dead time at least 200 ns | PASS |
| Existing startup logic audit remains valid | PASS |

## PHYSICAL_BLOCKERS

These items are intentionally not cleared by `STAGE1_LOGIC_PASS`:

1. **AN0 current-sensor physical polarity** is not certified.
2. **Firmware-positive versus actual grid-positive current** is not certified.
3. **PG1DC=6250 Buck neutral as a physical zero-energy/safe state** is not
   certified.
4. **Actual MOSFET, inductor and capacitor dynamics** are absent.
5. **Actual closed-loop current stability** cannot be assessed until a
   simplified/current plant is connected in the next simulation stage.

No firmware file, controller coefficient, PLL/table/reference offset, PWM or
startup logic was modified.

## Artifacts

The Stage-1 output directory contains:

- `case_summary.csv`;
- one CSV and SVG for every MODE A/B/C and phase-error case;
- `natural_transition.csv`, containing mode-before/after, vref, threshold,
  PG1/PG2 before/after and command step for every natural event.
