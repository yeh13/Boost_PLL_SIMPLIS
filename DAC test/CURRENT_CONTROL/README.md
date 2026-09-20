# Phase 2A Step 3: observation only

## Baseline and scope

Golden: `F:/Bidirection High Gain Inverter.sxsch` (621068 bytes).
SHA256: `F2505B747F1DA4471F9EEAABB0EFE34D22B9DC233CA1652B5A6A0333800A6365`.
Earlier wrong-golden audit/run conclusions are superseded and do not apply.

Candidate: `Bidirection High Gain Inverter_Current_Control.sxsch`.
R31 and Z are 196 ohms. Power wiring, original devices, PWM carriers,
feedforward, mode selection, gate mapping and existing voltage control remain.
No PLL or firmware current controller is connected. Buck compensation remains
unchanged. No new correction connects to U25. Transient duration is 120 ms.

## Debug interface

`CC_PHASE2A_OBSERVE` uses unilateral dependent sources only:

| Net | Definition | Display unit |
|---|---|---|
| CC_Iref | original signed V6 reference / Z | 1 V = 1 A |
| CC_Iout | existing V$IPROBE1 current, unchanged sign | 1 V = 1 A |
| CC_current_error | CC_Iref - CC_Iout | 1 V = 1 A |
| CC_error_counts_unquantized | CC_current_error * CURRENT_COUNTS_PER_AMP | 1 V = 1 count |
| CC_Vsense_delta | CC_Iout * 0.46 | sensor voltage relative to midpoint |

Global parameter CURRENT_COUNTS_PER_AMP = 570.957575757576 represents
0.46 * 4096 / 3.3, not 925. It is an adjustable candidate calibration,
without ADC quantization, offset, clipping or connection to a controller.
IPROBE1 retains its original output path, including the sensing branch;
it does not measure exclusively R31 current. No polarity inversion is made.
All new outputs terminate in observation nets without control feedback.

## Measured results

SIMPLIS 8.30 completed the 0-120 ms transient. Measurement window:
66.6666667-116.6666667 ms (three complete 60 Hz cycles).
RMS uses time-weighted piecewise-linear integration, not an unweighted
average of nonuniform event samples. Peak means maximum absolute value.

| Quantity | RMS | Absolute peak |
|---|---:|---:|
| Iref [A] | 1.1224489764 | 1.5873825700 |
| Iout [A] | 1.1246744712 | 1.5955993514 |
| current_error [A] | 0.0079183747 | 0.0290521900 |
| current_error [counts] | 4.521056019 | 16.587567981 |

Theoretical reference: 220/196 = 1.12244898 Arms;
sqrt(2)*220/196 = 1.58738257 A peak.

Main-waveform polarity agrees: at a positive reference peak,
Iref = +1.58738257 A, Iout = +1.59535925 A; at a negative peak,
Iref = -1.58738257 A, Iout = -1.59059677 A.
No error-sign violations occur when Iout < Iref (1 nA comparison tolerance).
Maximum identity residuals: 1.01e-11 A for subtraction, 2.90e-9 counts for
scaling, consistent with text-export rounding.

**Strict all-sample polarity is not a PASS:** among 330221 window samples,
718 have opposite signs near zero crossings; 266 have Iref > 0 but Iout <= 0.
Mismatches are confined to |Iref| <= 25.732 mA and |Iout| <= 26.247 mA.
None occur outside a 30 mA reference band. These are event-sample counts,
not duration percentages. This indicates a small zero-crossing mismatch,
not globally reversed measurement polarity. No sign was flipped.
Step 4 remains unstarted pending review. This is not a current-loop PASS.

## Reproduction and waveform viewer

- `phase2a_step3_run.sxscr`: open candidate and run SIMPLIS.
- `phase2a_step3_export.sxscr`: export completed group simplis_tran1.
- `phase2a_step3_analyze.py`: analyze export into phase2a_step3_metrics.json.
- `SIMPLIS_Data/`: generated netlist, processed deck and simulation results.
- `phase2a_step3_waveforms.txt`: full export, approximately 144 MB.

Viewer: Iref, Iout, current_error, error_counts_unquantized;
optionally Vsense_delta. Put counts on a separate axis.
This generated deck uses nodes 275, 273, 277, 278, 276 respectively;
node numbers can change after schematic edits. Show exports vectors in
reverse order as consecutive time/value blocks; the analysis script handles
this specific export layout.

Future Step 4 must explicitly distinguish 12500 (primary Buck/PG1 period)
from 10625 (Boost/PG2 upper domain), not use 10625 as universal full scale.
No controller-domain normalization or implementation is made here.
