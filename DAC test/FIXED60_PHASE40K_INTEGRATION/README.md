# Fixed60Hz + 5k -> 40k integration
**NEW CANDIDATE - NOT YET VERIFIED**

The integration package is prepared, not executed in MATLAB Online yet.
The preceding standalone Fixed60Hz candidate PASS is USER-REPORTED (see RESULTS/README.md).
It is not a result of this new distributed-correction integration.

## Run
Upload the entire MATLAB folder by itself, preserving its files. In MATLAB Online:
```matlab
run_fixed60_phase40k
```
Entry: MATLAB/run_fixed60_phase40k.m. Base MATLAB only; no MEX, external compiler or toolbox.
The entry sets its own path using mfilename('fullpath') and fullfile. All dependencies are local.
Input: 60.000 Hz, 30 seconds, ADC center 2048, amplitude 1860 counts, uint16 rounding,
40 kHz ADC-equivalent calls, /8 -> 5 kHz PLL. No other AC frequencies are simulated.
Runtime writes MATLAB/RESULTS/<timestamp>, starting with RUNNING status.
At least 20 seconds must follow FIRST HOLD entry. Both signal metrics use the SAME
20-second window, aligned to a grid cycle at least one second after that entry.
If that window cannot fit into 30 seconds, the script stops without qualification.
No existing results are replaced by a new run.

## Provenance / preserved logic
- Full original DAC-test inventory/classification: RESULTS/INVENTORY_CLASSIFICATION.md.
  All three old .v sources were read. inverter_controller.v is 2.5 kHz SOGI ONLY;
  its 64-bit SOGI equations/Q30 rounding are structural references, not a source of new parameters.
- pll_lock_test.va and pll_stage3d_active_test.va also specify 2.5 kHz and /16.
- dac_test_source.v is only a DAC code generator; vsx_root.v is its generated bench.
- Old CURRENT_REFERENCE_INTERPOLATION_INTENT_AUDIT.md describes causal /16 interpolation,
  including final target assignment and fixed lag; it is NOT the extrapolator used here.
- No existing correct 5k->40k no-reset implementation was found in DAC test.
- MATLAB/stage17_model_60hz.m, firmware_config.m and source sine/Vref CSVs are unchanged copies
  from the earlier Stage 1.7 package whose Fixed60Hz behavior the user reports as candidate PASS.
  Its coefficient selection/interpolation, PI, integrator, thresholds and HOLD state rules remain unchanged.
- New distributed-correction algorithm was explicitly authorized by the user after the source audit.
  It is a NEW candidate, not a relabeled prior validated extrapolator.
- New HDL file retains the PLL numeric body of the existing integration-folder
  fixed60_pll_5k_candidate.v (itself derived from the earlier DCAC 5 kHz candidate),
  and appends delivery in one clock process. The PLL-only file is not overwritten.

## Signal flow
60 Hz AC -> ADC equivalent -> unchanged SOGI/PLL TRACK @5k ->
locked + phase_ok + stable entry -> HOLD_60HZ -> actual step 9059696 ->
5 kHz same-time phase anchor -> circular error -> exact 8-tick quotient/remainder
correction -> continuous 40 kHz phase -> original Q15 sine-table interpolation.
A parallel phase path holds each 5 kHz anchor for eight 40 kHz samples and uses
the SAME Q15 lookup. There is no current-reference interpolation or output phase compensation.
Fixed frequency does not disable the detector, integrator, slow error, anchors or corrections.

## Timestamp contract (MATLAB and HDL)
1. First PLL callback/update is raw ADC sample 8, timestamp (8-1)/40000.
2. The anchor is theta BEFORE this update's NCO increment (Stage 1.7 diagnostic column 4).
   This is the detector's NCO phase at that ADC timestamp and preserves the phase-error
   convention used by the standalone Stage 1.7 test.
   Post-update PLL theta represents the next 5 kHz model instant; it remains separately observable.
   No 4.32-degree, 0.54-degree, 25-us or other output compensation is added.
3. The delivery accumulator already contains its predicted phase for the current timestamp.
   Compute circular(anchor - delivery_theta) from those SAME-time values.
4. Output the CURRENT delivery theta unchanged. Never assign it from the anchor.
5. Apply base plus correction to form the NEXT tick's internal prediction.
   Substep 0's correction is for the transition 0->1, not a jump at substep 0.
   Substeps 0..7 therefore schedule eight transitions; transition 7->next 0 ends the interval.
6. HDL uses one blocking-ordered state-update process. It does not instantiate another clocked
   mailbox owner, use an NBA result from the prior edge, or shift delivery to the falling edge.
   theta_40k is the current-time output; the internal accumulator stores the next-time prediction.

Startup: PLL input-invalid resets retain their original behavior. Delivery remains frozen until valid,
without any anchor reseed. Explicit reset initializes delivery phase to zero.
HOLD entry/exit never resets either delivery theta or PLL theta.

## Fixed-point correction
PLL angle unit is degree Q21 (360*2^21=754974720 codes/turn).
Delivery uses degree Q24 (eighth-Q21 units), M=6039797760 codes/turn.
Thus the integer code base_step_40k=pll_step_q represents exactly pll_step_q/8 in Q21 units.
No base-step remainder is lost.

error = mod(anchor_ticks - theta_ticks + M/2,M) - M/2
q = trunc(error/8)
r = error - 8*q
Each outgoing tick adds q, plus sign(r) whenever an accumulator of abs(r) reaches 8.
After eight ticks sum(correction)=error EXACTLY, for either sign, including nonmultiples of eight.
The half-turn tie uses -180 degrees. 359->1 gives +2 degrees; 1->359 gives -2 degrees.

ENDPOINT residual compares DIFFERENT timestamp-adjusted coordinates correctly:
target_end = wrap(anchor_start + 8*base_step_40k)
residual_end = circular(target_end - corrected_theta_at_end)
Using the original stationary anchor as target_end would falsely report a full interval's advance as error.
The last PLL anchor has only one observed output sample before the 30-second endpoint.
Its seven future samples and full-interval residual are excluded from reported observations.

No clipping/gain/threshold is added to correction. A large error can therefore produce a large
temporary frequency change. The report measures and bounds each increment against the actual
distributed correction; the 60-Hz steady-state offset check must also pass.
This is not a claim of safe behavior under arbitrary phase faults.

## Outputs / tests
- debug_40k.csv: time, anchor, theta, base, circular error, q, r, outgoing correction,
  remaining correction AFTER that outgoing step, substep, outgoing increment, integration residual,
  endpoint target, HOLD, phase_ok, locked, held_ref and extrapolated_ref.
- pll_5k.csv: original PLL internals, actual step, flags, HOLD.
- anchor_intervals.csv: every completed interval's error, summed correction and endpoint residual.
- reference_metrics.csv: fundamental frequency, phase RMS/peak, positive/negative ZC RMS error,
  THD, below-20kHz nonfundamental content, maximum boundary discontinuity.
- delivery_metrics.csv: maximum instantaneous increment deviation from base; endpoint residual
  max absolute/RMS/signed mean; added offset; nominal quantization drift; smoothness measures.
- pll_flags.csv: first-HOLD-to-end uptime and falling-edge counts.
- integration_checks.csv / delivery_vector_checks.csv / event_coverage.csv.
- eleven PNG and FIG figures: grid/reference, frequency, phase error, slow error, flags,
  held/extrapolated phase, held/extrapolated sine, positive/negative ZC zoom, boundary zoom,
  FFT/spectrum and peak/wrap zoom.
- integration_data.mat and STATUS.txt.

Phase error is circular NCO-to-grid error without subtracting its steady offset.
Frequency is estimated by a linear fit of same-polarity ZC times.
ZC interpolation is MEASUREMENT ONLY and never generates the output reference.
THD includes harmonics 2..40. Periodic Hann and +/-2 FFT bins around each harmonic are used.
Nonfundamental content excludes DC/fundamental and integrates all bins below 20 kHz,
reported as RMS relative to fundamental; it includes sampling images not counted by H2..H40 THD.
A 20-second window has 1200 exact grid cycles; the tiny nominal frequency quantization is retained.
Held boundary discontinuity is its boundary phase jump (its inter-boundary increment is zero);
extrapolated discontinuity is the jump unexplained by the previously scheduled increment.
The additional instantaneous-deviation metric includes the intended distributed correction.

Tests require continuous flags, nominal actual step, no return to TRACK, slow error within 45,
no PLL HOLD-entry integration discontinuity, no delivery reset at valid anchors, exact correction sums,
exact endpoint residuals, proper wrap, bounded per-tick increment and smaller adjacent reference changes.
A same-time analytic PLL ramp checks added phase offset within one delivery LSB and detects one-tick latency.
Unchanged nominal quantization drift is allowed; the old step-plateau drift is not.
Synthetic signed-error vectors exercise +/-2-degree wrap and +/-1..19 Q24-unit residuals at the SAME
nominal 60-Hz step. They are transport unit tests, not additional AC-frequency cases.

## Verilog candidate for later SIMPLIS integration
File/module: VERILOG/fixed60_pll_phase40k_candidate.v / fixed60_pll_phase40k_candidate.
External inputs only: unsigned 12-bit ac_input, 40 kHz clk, active-high reset.
This is a simulation controller candidate, not a gate-driver/power-stage implementation.
Compiler/language and HDL-vs-MATLAB parity remain to be verified before simulator integration.
The standalone fixed60_pll_5k_candidate.v remains available unchanged for comparison.
Do NOT instantiate both PLL owners to build this integrated module.

| Port | Width | Signed | Q-format / units |
|---|---:|---|---|
| ac_input | 12 | no | raw ADC codes 0..4095, center 2048 |
| clk, reset | 1 | no | 40 kHz clock; reset |
| pll_phase_5k | 30 | no | same-time pre-update degree Q21 |
| pll_phase_post_5k | 30 | no | post-update degree Q21 |
| pll_step_q | 32 | yes | degree Q21 per 5 kHz update |
| slow_error | 32 | yes | normalized detector counts |
| phase_ok, locked, hold_60hz, input_valid, pll_update | 1 each | no | flags |
| theta_40k | 33 | no | CURRENT-time degree Q24, modulo M |
| base_step_40k | 64 | yes | degree Q24 per 40 kHz tick |
| phase_error | 64 | yes | anchor circular error in Q24 codes |
| correction_quotient, correction_remainder | 64 each | yes | q/r in Q24 codes |
| correction_applied | 64 | yes | correction for NEXT transition, Q24 codes |
| correction_remaining | 64 | yes | remaining error after scheduling NEXT transition |
| substep | 3 | no | 0..7 |
| held_ref, extrapolated_ref | 16 each | yes | source sine table Q15, nominal -32767..32767 |
| anchor_residual | 64 | yes | predicted endpoint residual in Q24 codes |
| interval_complete | 1 | no | residual-valid pulse at substep 7 |

The internal predictor computes endpoint residual at substep 7 for the next boundary;
that last transition is scheduled, not already physically elapsed at substep 7.
MATLAB statistics only count intervals whose endpoint lies inside the observed run.

## Verification status / preservation
Local BigInt arithmetic spot-checks executed: 13 signed-error quotient/remainder vectors plus both 359/1 degree shortest-path directions. This only checks arithmetic concepts; no MATLAB integration or HDL simulation was executed.
Verilog structural begin/end and single-clock-owner checks were performed, not HDL compilation.
No MATLAB integration simulation, HDL simulation, HDL parity or SIMPLIS power-stage run was performed here.
Only after MATLAB execution can its STATUS say candidate checks satisfied; that is not a Verilog/SIMPLIS PASS.
Old DAC files, original PLL-only candidate, Fixed60Hz MATLAB logic, tables and formal firmware are preserved.
Pre-work source hashes are in RESULTS; verification checks compare them after delivery.
ADTR1PS remains 7 (old 1:8) in formal firmware. This package models intended 5 kHz numeric timing;
it does not validate the current hardware trigger cadence.
