# Step 5D prerequisites — integrity and analog duty bridge

Schematic integrity PASS. Analog duty bridge PASS. Full closed-loop NOT EXECUTED.

Evidence: step5/step5d/20261001T075810_042316Z/results.json.

Baseline and new step5d_integration.sxsch SHA256 both:
23c1708af6a6acd5b25d24b72d85e81aad96e98936be95b1872494482ed1ecaa.
Both ordinary and SIMPLIS-mode fresh netlists match after removing comment/path headers. No GUI metadata difference or electrical difference. Component parameters, connectivity, R31=196 ohm, Z=196, P=500, physical Vdc=100 V, Vref rectifier/add/subtract path, original X block, U25, U1/U14 comparators and Buck path are preserved. Firmware Vdc=574 is a separate inherited count-domain value; it was not substituted for the physical voltage source.

## Standalone bridge results

| finalDuty_counts | Dboost_fw measured | Expected |
|---:|---:|---:|
|0|0|0|
|3125|0.25|0.25|
|6250|0.50|0.50|
|9375|0.75|0.75|
|10625|0.85|0.85|

Fresh Verilog-driven 14-bit integer bus -> analog weighted-voltage sum, bit weight=2^n/12500. Denominator is12500. Engine return0, fresh t1/t2 PASS, timestamps and SHA256 recorded, 505 points, 0..125 us. Measured settled windows exclude transitions; maximum residual at exported t2 precision=0 for all five windows. This is a physical analog node from digital bits, not merely a Python arithmetic check. GNDREF=Y with explicit ground is required at this analog boundary; the pure digital Step5C core remains unchanged.

SIMPLIS bus ordering is LSB-first: first instance pin is vector bit0. Earlier MSB-first mapping was rejected and corrected in the bridge only. Earlier syntax and mapping attempts remain as failure evidence; only the fresh passing run above is used for the result. The Step5C interface report's earlier statement implying [31:0] pin order is corrected by this generated-root evidence. Step5C runtime CSV parity is unaffected.

## Prepared formal Boost command path (not run)

prepared/STEP5D_COMMAND.v copies the original334-entry VrefTable into VrefTable.hex, uses inherited Vdc574 / integer feedforward and instantiates the unchanged, bit-exact Step5C core. prepared/command_bridge.inc reuses the tested LSB-first /12500 bridge. All MODULE_SOURCE/include paths are relative. Before executing in a future isolated run, resolve the readmemh data path to that run's copied VrefTable.hex because the installed vvp launches from its binary directory.

prepared/integration_candidate.net changes only U1/U14 comparator positive-command inputs from node238 (U25 continuous-X plus old correction) to node9001 (Dboost_fw). Original X/U25 remain observation only; comparator saw inputs and Buck components are unchanged. The pristine integration schematic remains the integrity reference; the intentional command substitution is in this separate prepared candidate netlist and is recorded in manifest.json. No power-stage deck was run or claimed electrically identical after the intentional command substitution.

External adapter inputs are explicitly exposed: 40k clock, reset, table index0..333 and Step4C-contract signed deadband error. The next round must bind these to synchronized reference/current sampling, verify polarity/scaling and phase/reset semantics, and resolve data lookup before a closed-loop launch. This candidate adapter has not itself received full runtime parity; Step5C core has. No Buck current controller is added.

Eligible to proceed to a Step5D half-load closed-loop work round; NOT a declaration that the prepared deck is immediately runnable or closed-loop validated. Baseline load unchanged. PHYSICAL VOLTAGE CALIBRATION: NOT FORMALLY RE-VALIDATED. No coefficients, Vdc counts, LUT contents, PLL, ADC cadence or active firmware modified. No full-load test.
