# TODO

## Immediate
- [ ] Audit `pll_reference_5k40k.c` variable units and Q formats.
- [ ] Determine exact meaning of `pll_step_q`.
- [ ] Verify whether `phase_step_40k_q = pll_step_q / 8` is valid.
- [ ] Build or repair executable parity workflow.
- [ ] Compare C vs MATLAB at 59 Hz.
- [ ] Compare C vs MATLAB at 60 Hz.
- [ ] Compare C vs MATLAB at 61 Hz.
- [ ] Report first mismatch and maximum absolute error.

## After Parity
- [ ] Confirm smooth 40 kHz phase output.
- [ ] Confirm 0.54 degree/tick behavior at 60 Hz.
- [ ] Integrate validated phase delivery with current-reference generation.
- [ ] Check current-loop interaction.
- [ ] Move validated controller into SIMPLIS.

## SIMPLIS
- [ ] Finish ADC-only calibration.
- [ ] Verify ADC clock and analog input path.
- [ ] Complete seven-point ADC calibration.
- [ ] Connect validated PLL model.
- [ ] Connect current controller.
- [ ] Validate startup and zero-cross sequencing.

## Firmware Integration
- [ ] Keep active firmware unchanged until candidate is validated.
- [ ] Create dedicated integration branch before firmware modification.
- [ ] Record SHA256 of validated reference inputs.

## Controlled reconstruction — 2026-10-01

Separate from the older PLL reference_candidate/C-MATLAB tasks above:

- [x] Audit current firmware contract without firmware changes.
- [x] Reconstruct Step 4B source cadence evidence (40k raw /8 -> 5k); board timing not re-measured.
- [x] Reconstruct Step 4C firmware-input parity: 46,218 samples; all 18 fields mismatch=0, max difference=0.
- [ ] Next authorized round: Step 5A fresh original SIMPLIS feedforward validation and freshness guards.
- [ ] Step 5B integer-command parity: seven groups / 55,100 samples.
- [ ] Step 5C minimal digital runtime fixture and 55,100-sample Icarus replay.
- [ ] Step 5D prerequisites only: schematic integrity, analog duty bridge, inherited reference-domain audit.
- [ ] Full Step 5D closed-loop: not executed; outside this reconstruction round.

See DAC test/CURRENT_CONTROL/RECONSTRUCTION_PLAN.md and STEP4C_ADC_INPUT_PARITY.md. Preserve current firmware and existing local modifications.

## Step 5A / Step 5B completion — 2026-10-01

Completion update for the pending reconstruction checklist above:

- [x] Step 5A fresh original SIMPLIS feedforward validation; full freshness gates PASS.
- [x] Step 5B integer command: 7 groups / 46,090 new samples; all 18 fields mismatch=0 and max difference=0.
- [x] Step 5B CSV roundtrip and CSV input replay preserve every integer field; Dboost_fw maximum=0.85.
- [x] Preserve firmware, controller, baseline schematic and Step 4B/4C sources; no preceding validation reruns.
- [ ] Next round only: Step 5C reconstruction using the Step 5B digital contract and recorded vectors.
- [ ] Step 5D prerequisites and full current closed-loop remain unexecuted.

See DAC test/CURRENT_CONTROL/STEP5A_FRESH_RESULTS.md and STEP5B_INTEGER_COMMAND.md. PHYSICAL VOLTAGE CALIBRATION remains NOT FORMALLY RE-VALIDATED.


## Step 5C interface audit — 2026-10-01

- Interface initialization PASS: minimal plus nine cumulative output/VPI-probe stages; each fresh 0..50 us run returns 0. Original two-sample preflight also PASS.
- Root cause: installed Icarus cannot resolve generated drive-qualified absolute includes; use run-local relative MODULE_SOURCE. Controller arithmetic and Step 5B vectors unchanged.
- Full Step 5C 46,090-vector runtime parity remains PENDING; no power-stage integration performed. This supersedes the earlier statement that no Verilog files have been created.
- Evidence: DAC test/CURRENT_CONTROL/STEP5C_INTERFACE_AUDIT.md.


## Step 5C full runtime completion — 2026-10-01

STEP 5C RUNTIME PARITY PASS: 46,090 samples, 7 groups, 9 fields, mismatch=0 and max difference=0 against both recorded Step 5B Python/C oracles. Engine return=0; fresh datasets and exact 40 kHz cadence PASS; duration 1.152250 s. Supersedes pending full-runtime statements above. Root cause: absolute MODULE_SOURCE/include path incompatible with local Icarus invocation; relative source paths used. Controller and Step 5B vectors preserved. Step 5D prerequisites are eligible for a later authorized round; not executed. See DAC test/CURRENT_CONTROL/STEP5C_RUNTIME_STATUS.md.


## Step 5D prerequisites — 2026-10-01

Schematic integrity PASS (identical SHA256 and fresh SIMPLIS netlist). Standalone digital-to-analog /12500 bridge PASS: 0/3125/6250/9375/10625 -> 0/.25/.5/.75/.85; engine0, fresh505-point t1/t2, 0..125us. Prepared separate Boost command candidate changes only U1/U14 command input; Buck and baseline load remain unchanged. Closed-loop not run. Next round must bind sampled current/reference inputs and resolve run-local LUT data path before half-load execution. See DAC test/CURRENT_CONTROL/STEP5D_INTEGRITY_BRIDGE.md.
