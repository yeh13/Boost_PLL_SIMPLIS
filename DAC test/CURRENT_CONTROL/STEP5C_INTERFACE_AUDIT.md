# Step 5C interface initialization audit — 2026-10-01

INTERFACE INITIALIZATION PASS. Full Step 5C / Step 5B-vector runtime parity remains pending.

## Root cause and repair

The failed preflight 20261001T074006_939575Z first reports error 5061: Icarus preprocessing exits 1 because the generated root's drive-qualified absolute `include` cannot resolve clock.v. Error 5064 and Verilog_HDL_probe_info_setup initialization #1 are downstream failures. No controller arithmetic or coefficients were changed.

Controlled compile-only tests use the same existing leaf.v: relative include exits 0; absolute forward-slash, 8.3 short absolute, and backslash absolute includes all exit 1. This establishes an absolute-include resolution issue in this installed toolchain; it is not just spaces in the filename. Evidence: step5/interface_audit/20261001T074233_838157Z/path_probe/results.json.

Repair: copy unchanged sources into each fresh run directory, use relative MODULE_SOURCE="clock.v" / "replay.v" (or audit.v / stim.v), retain the run-directory include search path. Only these two MODULE_SOURCE lines were repaired in step5c_preflight.py. No installation/global configuration edits.

## Toolchain and local contract

- Executed C:/Program Files/SIMetrix830/iverilog/10.1.1/bin/iverilog.exe and vvp.exe. Version banner: Icarus Verilog 10.1 stable (v10_1_1).
- Child PATH prepends that exact bin directory; config selects 10.1.1, with run_from_verilog_bin_dir=true. The failing engine error also records the actual 10.1.1 executable invocation.
- C:/Program Files/SIMetrix830/bin/simplis_verilog.cfg and simplis_verilog.vpi exist and are readable (hashes below); successful co-simulation confirms VPI loading.
- Local example support/examples/SIMPLIS-Verilog-HDL/Digital_PWM_with_Verilog_HDL/Test Schematics for Verilog_Lib/test_pwm.sxsch uses relative MODULE_SOURCE and GNDREF='N'. Verilog_Lib/PID_compensator.v uses $simplis_vpi_probe on controller variables.
- GNDREF='N' is consistent for this purely digital connection. An earlier analog-terminated experiment separately produced error 5045: analog attachment requires a ground-reference pin. The current fixture has no analog connection to the controller. An isolated numerical anchor is not a duty bridge.

## Port and probe ablation

Fresh root: step5/interface_audit/20261001T074622_736189Z.
Minimal: clk, signed32 e, signed32 base (= Boost_PWM), mode (= boost_mode), one heartbeat output. The four logical inputs flatten to 66 pins: clk node1, e nodes2..33, base nodes34..65, mode node66. Each vector follows declared [31:0] order. No extra ground pin.

All ten stages PASS with engine return 0, new t1/t2, timestamp gates, SHA256, 103 points and span 0..50 us. Each stage adds BOTH its output port and its $simplis_vpi_probe. No port/probe starts the original failure.

| Stage | Added output/probe | Width | Result |
|---|---|---:|---|
|0|heartbeat|1|PASS|
|1|acc|64|PASS|
|2|raw|64|PASS|
|3|limited|32|PASS|
|4|finalDuty|32|PASS|
|5|effective_correction|32|PASS|
|6|x1|32|PASS|
|7|x2|32|PASS|
|8|y1|32|PASS|
|9|y2|32|PASS|

Full output mapping: heartbeat100; acc101..164; raw165..228; limited229..260; finalDuty261..292; effective_correction293..324; x1325..356; x2357..388; y1389..420; y2421..452. Total353 output bits. Module names, model port names/widths and instance bit counts agree. Every declared output has a net; output-only digital observation nodes need no analog load. All VPI probes reference existing declared signals. No old analog duty/probe definitions remain in these generated fixtures.

Probe syntax check: a separate audit attempt using `.PRINT ^100` failed at deck parsing with error1092 (not the original error). That syntax is not used in the passing fixtures. Local-example $simplis_vpi_probe is accepted for both signed64 and signed32 outputs. t2 contains analog bookkeeping variables, not nine-field integer parity evidence; successful probe registration does not claim lossless integer waveform export.

Original two-sample preflight also PASS after the relative-path repair: step5/preflight/20261001T074639_295489Z/preflight.json. Smoke compile/run, controller compile and fresh SIMPLIS co-simulation pass; both expected nine-field rows match. This is not the full 46,090-vector replay.

## Exact generated minimal deck

```spice
* Step5C interface audit, stage 0
.TRAN 50u 0
.options PSP_START=0 PSP_NPT=101 SAVE_INSTANTS_FILES
.KEEP *V
!V_STIM [ 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27
+ 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54
+ 55 56 57 58 59 60 61 62 63 64 65 66 ] [ ] MODEL=STIM_MODEL
.MODEL STIM_MODEL VERILOG_HDL_MODULE MODULE="STIM" MODULE_SOURCE="stim.v"  OUTPUT="clk,1,e,32,base,32,mode,1"
+ INPUT="" GNDREF='N'  RIN=10Meg ROUT=10 TH=2.5 HYSTWD=0.1 VOL=0 VOH=5
!V_AUDIT [ 100 ] [ 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24
+ 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51
+ 52 53 54 55 56 57 58 59 60 61 62 63 64 65 66 ] MODEL=AUDIT_MODEL
.MODEL AUDIT_MODEL VERILOG_HDL_MODULE MODULE="AUDIT" MODULE_SOURCE="audit.v"  OUTPUT="heartbeat,1"
+ INPUT="clk,1,e,32,base,32,mode,1" GNDREF='N'  RIN=10Meg ROUT=10 TH=2.5 HYSTWD=0.1
+ VOL=0 VOH=5
VANCHOR 9000 0 1
RANCHOR 9000 0 1k
.end


```

## Full-stage wrapper / probe definitions

```verilog
`timescale 1ns/1ps
`include "STEP5C_CONTROLLER_RUNTIME.v"
module AUDIT(input clk,input signed [31:0] e,base,input mode,output reg heartbeat ,output signed [63:0] acc,output signed [63:0] raw,output signed [31:0] limited,output signed [31:0] finalDuty,output signed [31:0] effective_correction,output signed [31:0] x1,output signed [31:0] x2,output signed [31:0] y1,output signed [31:0] y2);

STEP5C_CONTROLLER_RUNTIME dut(clk,1'b0,mode,e,base,acc,raw,limited,finalDuty,effective_correction,x1,x2,y1,y2);
initial begin
 heartbeat=0;
 $simplis_vpi_probe(heartbeat);
 $simplis_vpi_probe(acc);
 $simplis_vpi_probe(raw);
 $simplis_vpi_probe(limited);
 $simplis_vpi_probe(finalDuty);
 $simplis_vpi_probe(effective_correction);
 $simplis_vpi_probe(x1);
 $simplis_vpi_probe(x2);
 $simplis_vpi_probe(y1);
 $simplis_vpi_probe(y2);
end
always @(posedge clk) begin #0.001;heartbeat=~heartbeat;end
endmodule

```

## Preserved artifacts

- config_sha256: `f90374d442859d3f1b8565a987817bde23f0005ba656b9767d837a355ca9c763`
- vpi_sha256: `818a7e566bb0d991b2ebd66222e9ceea0ee1f7d912f84ba914b9919852f550f9`
- controller_sha256: `682e47ae090ea157086e6dcf07145e315e9efc81d3ecec417b7912c91e76975e`
- Step 5B inputs.csv, python.csv and c_oracle.csv match their recorded SHA256 values. No Step 5B vectors changed.
- Firmware, coefficients and schematics were not edited. No power-stage simulation or full runtime replay executed in this audit.


## Step 5C full runtime completion — 2026-10-01

STEP 5C RUNTIME PARITY PASS: 46,090 samples, 7 groups, 9 fields, mismatch=0 and max difference=0 against both recorded Step 5B Python/C oracles. Engine return=0; fresh datasets and exact 40 kHz cadence PASS; duration 1.152250 s. Supersedes pending full-runtime statements above. Root cause: absolute MODULE_SOURCE/include path incompatible with local Icarus invocation; relative source paths used. Controller and Step 5B vectors preserved. Step 5D prerequisites are eligible for a later authorized round; not executed. See DAC test/CURRENT_CONTROL/STEP5C_RUNTIME_STATUS.md.


Step5D generated-root clarification: instance bus pins are LSB-first (first pin maps bit0), not descending [31:0]. Core and runtime CSV parity unchanged.
