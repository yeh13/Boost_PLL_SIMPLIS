# Full Controller Stage 2 — Simplified Current Plant

## Final judgment

**STAGE2_SIMPLIFIED_PLANT_PASS** for the selected, explicitly provisional
averaged plant.

The firmware-equivalent sampled controller is bounded, the Boost regions track,
the active limiter releases, both signed half-cycles behave similarly, and no
natural transition or zero crossing loses control in this model. This is not a
hardware or switching-stage PASS.

## Model authority and limitations

The project contains no formal or measured `Gid_buck(s)` / `Gid_boost(s)`.
The only traceable duty-to-current model is
`simulate_buck_boost_current_transition.m`, which labels itself a tunable
second-order PLACEHOLDER. Stage 2 therefore reuses its nominal parameters:

| Region | Natural frequency | Damping | Compensation-to-current gain |
|---|---:|---:|---:|
| Buck | 2.6 kHz | 0.72 | 0.115 counts/count |
| Boost | 1.8 kHz | 0.62 | 0.145 counts/count |

The plant is integrated at 200 kHz (five 5 us substeps per control interval).
The controller executes only once every 25 us. Each tick preserves
sample -> integer ADC/filter/controller calculation -> held duty -> plant
response. `CURRENT_SENSOR_POLARITY=+1` is used solely to match the firmware
signed convention; board polarity remains unverified.

The averaged operating-point term maps the active signed reference to the
nominal plant current, while the Boost 2P2Z output drives the documented local
small-signal gain. This makes the result a conditional feasibility test, not an
identified hardware prediction.

## Controller reproduced without retuning

- active exact 668/334 signed reference and +3.55 deg reference offset;
- 40 kHz causal reference replay and Stage-4 Q15 soft-start;
- centered ADC current model and `y += (adc-y)>>2` filter;
- active signed error equations;
- Boost coefficients `A={30010,2668}`, `B={26000,-32000,9640}`;
- arithmetic Q15 shift, +/-2500 controller limiter and 0..10625 PG2 clamp;
- Buck feedforward only;
- active 574 / 594 / 554 mode-selection and hysteresis;
- mode-entry controller state clear, PWM mapping and polarity commands.

No firmware or coefficient was modified.

## Steady closed-loop metrics

Metrics use the settled final 0.15 s of each 0.5 s run. THD includes harmonics
2 through 40; no strict THD pass threshold was imposed.

| Reference case | RMS error (counts) | Peak error | i_ref->current phase | Amplitude ratio | THD | Controller limiter | Duty clamp occupancy |
|---|---:|---:|---:|---:|---:|---:|---:|
| small, 0.25x | 3.831 | 19.264 | +0.073 deg | 1.00144 | 2.295% | 0% | 0.300% |
| 0.5x nominal | 6.837 | 27.335 | -0.260 deg | 1.00908 | 1.651% | 0% | 0.300% |
| 1.0x nominal | 14.675 | 40.027 | -0.294 deg | 1.01398 | 1.619% | 0% | 0.435% |

The active current reference is approximately 180 degrees from the mathematical
positive grid sine because of the already-approved legacy signed frame. Combining
the Stage-1 Grid->i_ref value (about +179.730 deg) with the nominal plant lag
gives Grid->current about **+179.436 deg**. This is a frame statement, not board
polarity certification.

All currents and commands remain bounded. The brief duty clamps occur around
mode/polarity boundaries and do not persist through the cycle.

## Boost-only isolated assessment

The nominal run was filtered to natural Boost windows only, excluding Buck
samples from the stability metric:

- analyzed Boost samples: 1356 decimated report samples;
- RMS tracking error: 13.535 counts;
- peak tracking error: 29.706 counts;
- +/-2500 limiter hits: zero.

The Boost state does not grow cycle to cycle, its correction sign matches the
selected half-cycle, and its output remains away from the controller limiter in
the nominal case. Thus the active 2P2Z is usable with this local placeholder
operating-point model.

## Buck region

Buck remains feedforward. Its command is bounded in 0..12500 and uses the active
signed mapping. Plant current remains bounded while entering, dwelling in and
leaving Buck. Nonzero Buck tracking error is recorded but is not judged by the
Boost 2P2Z regulation standard.

Because the placeholder operating-point term supplies the nominal Buck current,
this simulation does not prove that the real Buck feedforward achieves a chosen
current amplitude under load variation. That requires an identified Buck plant.

## Positive/negative symmetry

For the nominal settled run:

| Half-cycle | RMS error | Peak error |
|---|---:|---:|
| phase_cmd=0 / negative | 14.088 counts | 40.027 counts |
| phase_cmd=1 / positive | 15.240 counts | 39.785 counts |

Peak behavior is essentially symmetric. RMS differs by about 1.15 counts; no
sign reversal, unbounded state or one-sided saturation occurs. The residual
asymmetry is consistent with integer mapping, filter timing and different
Buck/Boost sample placement.

## Amplitude steps

Both steps are commanded at a grid zero crossing after initial steady operation.
Using one-cycle peak envelopes:

| Step | First post-step peak | Target peak | Overshoot | 90% rise bound | 5% settling bound | Limiter hits |
|---|---:|---:|---:|---:|---:|---:|
| 0.25x -> 0.5x | 466.31 | 458.5 | 1.70% | <=4.2 ms | <=16.7 ms | 0 |
| 0.5x -> 1.0x | 936.96 | 917 | 2.18% | <=4.2 ms | <=16.7 ms | 0 |

The bounds reflect the 60 Hz envelope resolution; they are not switching-scale
rise-time measurements.

## Natural mode transitions

Only active hysteresis transitions were used. Across 0.5 s, each case contains
120 natural transition events. Maximum single-40-kHz-tick physical-current
changes were:

| Case | Maximum current step (ADC-equivalent counts) |
|---|---:|
| 0.25x | 3.909 |
| 0.5x | 4.147 |
| 1.0x | 8.413 |
| 0.25x -> 0.5x | 4.151 |
| 0.5x -> 1.0x | 8.439 |
| limiter disturbance | 23.596 |

No compare goes outside its active range, and current remains continuous and
bounded. These values are properties of the placeholder bandwidth and gain;
they must be remeasured with an identified plant.

## Zero crossing

The nominal maximum one-tick physical-current change in the audited zero-cross
window is **10.892 counts**. `phase_cmd` changes according to the exact 668 map;
current crosses continuously, controller correction remains bounded, and no
growing notch or spike appears. The disturbance case reaches 14.249 counts but
also remains bounded.

## Limiter and recovery

The dedicated test applies a deliberately adverse plant-gain/polarity
disturbance from 0.10 to 0.25 s with a 1.5x reference, then restores the nominal
plant and reduces the reference to 0.25x.

- +/-2500 controller-limiter hits: **1719 / 20000 = 8.595%**;
- final-duty clamp hits: 3014 / 20000 = 15.07%;
- after restoration: no further limiter occupancy;
- stuck-at-limit condition: none;
- settled post-recovery metrics equal the small-reference case.

The limiter was already released during the immediately preceding natural
Boost-to-Buck/zero-cross interval, so measured post-command recovery is 0 ms at
the 25 us controller resolution. The active implementation has no separate
anti-windup branch; recovery here is aided by using realized compensation state
and by the active mode-entry state clear. This result must not be generalized to
all saturation timing or a real plant.

## Stage-2 criteria

| Criterion | Result |
|---|---|
| Closed-loop current bounded | PASS |
| Boost-region stable tracking | PASS for selected placeholder |
| Correction direction | PASS |
| Limiter release / no stuck state | PASS |
| Positive/negative gross symmetry | PASS |
| Natural transition bounded | PASS |
| Zero crossing bounded | PASS |
| No long illegal duty saturation | PASS |

## PHYSICAL BLOCKERS

Even with `STAGE2_SIMPLIFIED_PLANT_PASS`, all of the following remain open:

1. actual AN0 sensor polarity and counts-per-amp calibration;
2. firmware-positive versus hardware/grid-positive current direction;
3. whether Buck compare 6250 is a physical zero-energy/safe state;
4. identified Buck and Boost `Gid(s)` at real operating points;
5. MOSFET switching, diode/body-diode and gate-driver behavior;
6. inductor, capacitor, load and parasitic dynamics;
7. real ADC noise, switching ripple and timing jitter;
8. physical SSR delay and conduction state;
9. hardware closed-loop stability and safe power-up behavior.

No power-on recommendation is made.

## Outputs

`FULL_CONTROLLER_STAGE2_SIMPLIFIED_PLANT_OUTPUTS` contains the full case CSVs,
summary and natural-transition tables, per-case tracking/control plots, full
cycle tracking, error/controller, duty/mode, transition zoom, zero-cross zoom,
limiter recovery, FFT/THD plot and harmonic CSV.
