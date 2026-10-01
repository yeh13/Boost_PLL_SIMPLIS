# Controlled reconstruction plan — 2026-10-01

Base: `debug-analog-wrapper-wip`, HEAD `eb5256ac960791e937f6787f28bae4d98598cdbd`, including its existing local changes. Lost Step 4B–5D artifacts are not recoverable locally. Historical PASS statements are targets, not fresh evidence.

## This round

1. Record source hashes and inspect the current ADC/PWM contract without changing firmware, LUT, coefficients, PLL, Buck controller, or schematics.
2. Step 4B: document configured PWM/AN0/AN1 raw 40 kHz and AN1 /8 SOGI/PLL 5 kHz. Distinguish source audit from measured hardware timing or ISR execution-time proof.
3. Step 4C: create a separate Python model and compile an independent oracle from the actual C callback, state-clear and feedforward source. Exercise raw ADC filtering, both reference paths, polarity, zero band, deadband, modes, saturation and state carry/reset. Store fresh inputs, both traces, per-field mismatch/max differences, coverage and SHA256 values in a new run directory. Fail on missing coverage or mismatches.
4. Publish `STEP4B_CADENCE_RESTORE.md` and `STEP4C_ADC_INPUT_PARITY.md`. Append status/TODO notes while preserving all pre-existing bytes. Stop before Step 5A.

## Later rounds (not executed now)

- Step 5A: fresh original SIMPLIS feedforward data, freshness guards and comparison with historical residuals; no automatic historical PASS.
- Step 5B: firmware integer command, seven groups / 55,100 samples, independent C parity, finalDuty / 12500.
- Step 5C: signed 64-bit digital runtime, Icarus 10.1.1, minimal GNDREF='N' fixture, 55,100-sample replay; no analog output initially.
- Step 5D prerequisites: separate schematic integrity and five-point analog duty bridge evidence, plus inherited reference-domain documentation.
- Full Step 5D closed-loop is outside this reconstruction round.

## Preservation and limits

No reset, clean, pull, bulk staging, replacement of existing modified files, or reuse of old generated results as fresh evidence. Generated DLLs and traces are evidence/build outputs, not firmware source. Firmware-input parity does not validate physical ADC calibration, interrupt scheduling, PLL phase delivery, or a closed-loop power stage.
