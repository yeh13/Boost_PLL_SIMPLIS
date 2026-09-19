# Current Polarity and Startup Contract Audit

## Scope and result

This is a source-level and discrete dry-run audit only. No firmware source,
control parameter, PWM setting, or power-plant model was changed.

Two separate contracts were found:

1. The active 668-point path is internally consistent with the legacy signed
   frame `phase_cmd=0 -> negative`, `phase_cmd=1 -> positive`. Therefore the
   exact table is approximately `-sin(theta_interp + REF_PHASE_OFFSET)`, not
   `+sin(...)`. The former 180-degree result was primarily an analysis-reference
   sign mismatch, not a discontinuity or table-index defect.
2. The startup transition is safe against digital H/L overlap under the active
   synchronized-PWM register contract. It changes from all outputs overridden
   low to PG1 50% complementary and PG2 `DC=0` complementary operation. The C
   source alone cannot certify that this switching state is a safe *physical
   power-stage neutral*; that requires the schematic, driver truth table and
   defined positive current direction.

Consequently the software/gate-logic portion of Stage 1 is no longer blocked by
the alleged 180-degree table defect or by the numerical `0 -> 6250` compare
change. Stage 1 remains **BLOCKED for physical-polarity/neutral certification**
until the board-level polarity contract is supplied or traced.

## A. Active 668 mapping

Both `DIAG_PLLPhaseToOldFrameCommand()` and `DIAG_PLLPhaseInterpMap()` implement
the same mapping. `DIAG_PLLPhaseInterpMap()` first computes
`full_idx=floor(phase_q32*668/2^32)` and then calls the old-frame mapper with the
668 coordinate source.

| Full index | Approx. theta | Table index | phase_cmd | VrefTable magnitude | iref_signed |
|---:|---:|---:|---:|---:|---:|
| 0..333 | 0 to <180 deg | full_idx | 0 | non-negative | negative |
| 334..667 | 180 to <360 deg | full_idx-334 | 1 | non-negative | positive |

The half-table starts at zero, rises to `VREF_PEAK=3787`, and returns near zero.
Thus the active signed sequence is negative in the first theta half-cycle and
positive in the second.

## A2. Current-reference and error-sign contract

The active interpolated current-reference path applies the following logic:

| phase_cmd | iref_signed | iref_cmd | Expected AN0 relative to center | Error passed to controller |
|---:|---:|---:|---:|---:|
| 0 | `-iref_amp` | `ADC_CENTER-iref_amp` | below 2048 | `adc_feedback-iref_cmd` |
| 1 | `+iref_amp` | `ADC_CENTER+iref_amp` | above 2048 | `iref_cmd-adc_feedback` |

The second sign reversal is deliberate: a positive controller error always
means "increase commanded magnitude" in the currently selected physical
direction. For example, in phase 0 an actual ADC value closer to center than
the negative reference makes `adc_feedback-iref_cmd > 0`, requesting more
negative-direction magnitude. This is coherent; it is not an accidental double
sign flip.

## A3. Buck and Boost polarity

In Buck mode `BUCK_DUTY()` is bipolar around 6250 counts:

| phase_cmd | Buck command | Meaning inside firmware frame |
|---:|---|---|
| 0 | `6250-modulation` | negative-direction command |
| 1 | `6250+modulation` | positive-direction command |

In Boost mode PG1 is a static polarity leg:

| phase_cmd | PG1 OVRDAT | PG1H | PG1L | Firmware direction |
|---:|---:|---:|---:|---|
| 0 | `0x1` | low | high | negative |
| 1 | `0x2` | high | low | positive |

PG2 remains the controlled Boost PWM leg. The mappings agree with the reference
and error conventions. Whether firmware "positive" is positive grid injection
cannot be inferred from symbol names: it additionally depends on AN0 sensor
polarity, transformer/inductor orientation and which bridge terminal is called
positive.

## A4. Polarity truth table

| Grid/theta half in analysis | 668 index | phase_cmd | Signed reference | Buck direction | Boost selector | Expected AN0 | Physical output-current label |
|---|---:|---:|---:|---|---|---|---|
| theta 0..<180 deg | 0..333 | 0 | negative | duty below 6250 | PG1L high | below center | firmware-negative; board sign unresolved |
| theta 180..<360 deg | 334..667 | 1 | positive | duty above 6250 | PG1H high | above center | firmware-positive; board sign unresolved |

Therefore `+sin(theta)` is not the firmware's definition of the first current
half-cycle. The code follows a legacy frame whose first half is negative.

## A5. Reinterpreting the 180-degree mismatch

The exact replay result `Grid -> table = +179.73043 deg` must be compared with
both mathematical references:

| Comparison | Phase relation | RMS error (pu) | Peak error (pu) | Interpretation |
|---|---:|---:|---:|---|
| exact table vs `+sin(theta_interp+3.55 deg)` | about 179.73 deg | 1.4142239 | 2.0000000 | wrong sign convention |
| exact table vs `-sin(theta_interp+3.55 deg)` | residual about -0.26923 deg | 0.0037745 | 0.0094637 | correct firmware sign convention plus table quantization/indexing |

The table THD remains 0.057842%, independent of selecting positive or negative
analysis polarity. The exact sequence crosses zero continuously at the 334/668
boundaries; the adjacent samples are small and there is no 180-degree command
jump.

Root-cause classification:

- **Not supported as a mapping bug:** all downstream signs agree with the map.
- **Supported as a legacy frame convention:** phase 0 is explicitly negative
  throughout reference, error, Buck and Boost logic.
- **Supported as an analysis-reference error:** the previous comparison imposed
  `+sin` without applying this firmware sign convention.
- **Still unresolved physically:** the source does not establish whether
  firmware-negative is the required grid-positive injection direction.

## B. Exact startup ordering

The active Stage-4 ordering on the AN0 40 kHz callback is:

1. Prior states repeatedly call `PowerPWM_AllOff()`: `PG1DC=PG2DC=0`, both
   generators' H/L overrides enabled, `OVRDAT=0`.
2. `DIAG_PLLThetaSafetyGateUpdate()` runs near the beginning of the same AN0
   callback.
3. In `STATE_WAIT_NEXT_ZC_TO_START_PWM`, the required new ZC causes control
   history clear, soft-start reset to zero, then asserts `spwm_enable`,
   `current_loop_enable_cached` and `grid_pg12_allowed`, and changes state to
   `STATE_GRID_CURRENT_RUN`.
4. The callback continues; `run_allowed` is therefore true on this same tick.
5. At the ZC-aligned table sample, zero Vref selects Buck and `BUCK_DUTY(0,0)`
   computes `PG1DC=6250`; `PG2DC=0`.
6. Buck handling calls `CURRENT_LOOP_BuckLegOverrideDisable()` **before** the
   final `PG1DC` write.
7. The final writer stores `PG1DC=6250`, `PG2DC=0`, then releases PG2 H/L
   override.
8. Hardware uses `UPDMOD=SOC` for duty updates and `OSYNC` synchronized override
   updates. Provided the ISR completes before the next local PWM update boundary,
   the final register set is applied together at that boundary.

The software order is therefore not the strongest possible preload-before-
release idiom. Its safety relies on the configured synchronized-update contract
and ISR completion margin. A future timing-exact hardware model must preserve
that fact; treating register writes as immediate pin changes would be incorrect.

## B2. Safe-off to 6250 gate replay

The 100-tick summary is in
`polarity_startup_audit_outputs/startup_release_100ticks.csv`; the first released
carrier edges are in `first_released_carrier_edges.csv`.

At release:

| Signal | Before boundary | First released carrier |
|---|---|---|
| PG1DC | 0 | 6250 |
| PG2DC | 0 | 0 |
| PG1 H/L override | both forced low | released |
| PG2 H/L override | both forced low | released |
| PG1 gates | H=0, L=0 | 50% complementary with 100-count dead time |
| PG2 gates | H=0, L=0 | H off; L active after its dead-time interval |
| soft-start | 0 | 0 on transition tick, then +16 Q15/tick |

For the modeled first PG1 carrier: both gates are low from count 0..99, H is
active after DTH, H turns off at 6250, both remain low for 100 counts, then L is
active. PG2 `DC=0` keeps H off and enables L after the boundary dead time. The
configured `PG1DTL=PG1DTH=PG2DTL=PG2DTH=100` corresponds to the previously
audited 200 ns.

Automatic results under the synchronized-update model:

- H/L overlap: **0 counts, PASS**.
- Minimum turn-on dead time: **100 counts / 200 ns, PASS**.
- Narrow abnormal pulse caused solely by `0 -> 6250`: **none, PASS**.
- Release before readiness: **none in the modeled legal state path, PASS**.
- PG2 Buck startup state: H off / L on after dead time; **logic-defined**, but
  physical topology safety is not provable from this source alone.
- Release exactly at a ZC event: avoids a large table command, but interpolation
  and ISR/PWM-boundary alignment must remain part of the timing model.

## B3. Meaning of 6250

`0 -> 6250` is a command-domain transition from disabled compare to the Buck
bipolar 50% point. While override is active, compare zero has no gate meaning;
both pins are forced low. After release, 6250 deliberately creates a
complementary 50% waveform. It is therefore neither an ordinary Buck/Boost mode
transition nor a 6250-count gate pulse.

Gate safety is judged by override synchronization, complementarity and dead
time, not by the raw subtraction of compare values. Physical zero-voltage or
zero-current neutrality is a separate topology contract and is not established
by the label "neutral".

## C. Mode-transition clarification

The earlier natural hysteresis replay remains valid: the largest traced active
state-transition compare change was approximately 446 counts. The preliminary
6250 result came from comparing the all-off initialization register with the
first Buck command and must not be reported as a Buck/Boost transition.

Forced transitions at arbitrary phases remain useful invalid-stress cases, but
they are not evidence of active hysteresis behavior. They do not replace the
natural transition result.

## Final disposition

| Item | Result |
|---|---|
| 668 indexing continuity | PASS |
| Internal signed-current contract | PASS (`phase 0 = negative`) |
| Previous 180-degree table blocker | RESOLVED as reference-frame mismatch |
| Error-sign consistency | PASS |
| Buck/Boost internal polarity consistency | PASS |
| Startup H/L overlap and dead time | PASS under active synchronized-update contract |
| `0 -> 6250` as a mode-transition defect | REJECTED; it is startup release |
| Board-level positive-current direction | BLOCKED: schematic/sensor convention required |
| 6250 as physical power-stage neutral | BLOCKED: topology/driver contract required |

Stage 1 may proceed with further **logic-only dry-run work** using
`-sin(theta_interp+REF_PHASE_OFFSET)` as the firmware signed reference. It must
not be promoted to a power-stage-safe result until the two board-level contracts
above are closed.
