# Step 5C full runtime parity

STEP 5C RUNTIME PARITY PASS

Evidence: `step5/runtime/20261001T075233_783623Z`.

Reused all 46,090 recorded Step 5B samples in their original seven-group order; no vectors regenerated. Controller inputs are a row-preserving projection of reset plus the saved, Python/C-validated err, Boost_PWM and mode fields. The input/reference chain is not reimplemented here.

40 kHz; period 25 us; simulation 0..1.152250 s. First controller sample at 12.5 us, then every 25 us; exact integer trace sampled 1 ps after each edge to observe updated states. 46,090 sequential sample IDs and all timestamps verified.

| Field | Python mismatch | C mismatch | Max difference |
|---|---:|---:|---:|
|acc|0|0|0|
|raw|0|0|0|
|limited|0|0|0|
|finalDuty|0|0|0|
|effective_correction|0|0|0|
|x1|0|0|0|
|x2|0|0|0|
|y1|0|0|0|
|y2|0|0|0|

All nine signals register $simplis_vpi_probe using the audited local syntax. Exact signed integer comparison uses $fwrite from the same SIMPLIS-launched Verilog runtime, avoiding precision loss in analog t2 export. Runtime CSV was absent before execution and is included in the freshness/hash gate. No standalone-vvp result substitutes for co-simulation.

Engine return code 0; freshness PASS; fresh t1/t2 and runtime.csv; t2 point count 138271; start/stop and monotonicity verified. Timestamp tolerance remains the established +/-2 seconds for USB filesystem rounding. All file SHA256 values and execution timestamps are recorded in freshness.json / parity.json.

Sources, vectors and data reside in the fresh isolated directory. MODULE_SOURCE and generated root includes are relative only. Data-file $fopen paths address this run directory explicitly because the installed configuration launches vvp from its binary directory; these are not Verilog source/include paths. GNDREF='N'.

Root cause resolved earlier: absolute MODULE_SOURCE/include path incompatible with local Icarus invocation. This run does not repeat interface audit. Controller arithmetic is unchanged (SHA256 below), including signed64 accumulation, arithmetic shift, clamps, effective-correction write-back and Buck clear.

Controller SHA256: `682e47ae090ea157086e6dcf07145e315e9efc81d3ecec417b7912c91e76975e`

Groups: `{'steady_state': 12000, 'mode_transition': 8000, 'clamp_stress': 16384, 'buck_state_clear': 4000, 'positive_negative_phase': 5344, 'reference_zero_band_boundary': 344, 'error_deadband_boundary': 18}`

DIGITAL DOMAIN ONLY. PHYSICAL VOLTAGE CALIBRATION: NOT FORMALLY RE-VALIDATED. No analog duty bridge, power stage or Step 5D execution.
