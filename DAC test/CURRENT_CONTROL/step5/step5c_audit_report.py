"""Publish the completed interface audit; never runs a simulation."""
import hashlib
import json
from pathlib import Path

here=Path(__file__).resolve().parent
cc=here.parent
repo=cc.parents[1]
root=here/'interface_audit/20261001T074622_736189Z'
results=json.loads((root/'results.json').read_text())
assert len(results)==10 and all(r['status']=='PASS' for r in results)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
manifest=json.loads((root/'manifest.json').read_text())
assert sha(here/'STEP5C_CONTROLLER_RUNTIME.v')==manifest['controller_sha256']
b=cc/'step5b_results/20261001T073208_039093Z'
metrics=json.loads((b/'metrics.json').read_text())
for name in ['inputs.csv','python.csv','c_oracle.csv']:
    assert sha(b/name)==metrics['evidence_sha256'][name]
deck=(root/'ports_00/engine.deck').read_text()
wrapper=(root/'ports_09/audit.v').read_text()
text='''# Step 5C interface initialization audit — 2026-10-01

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
'''+deck+'\n```\n\n## Full-stage wrapper / probe definitions\n\n```verilog\n'+wrapper+'\n```\n\n## Preserved artifacts\n\n'
for k in ['config_sha256','vpi_sha256','controller_sha256']:
    text+=f'- {k}: `{manifest[k]}`\n'
text+='- Step 5B inputs.csv, python.csv and c_oracle.csv match their recorded SHA256 values. No Step 5B vectors changed.\n- Firmware, coefficients and schematics were not edited. No power-stage simulation or full runtime replay executed in this audit.\n'
(cc/'STEP5C_INTERFACE_AUDIT.md').write_text(text,encoding='utf-8')
append='''\n\n## Step 5C interface audit — 2026-10-01

- Interface initialization PASS: minimal plus nine cumulative output/VPI-probe stages; each fresh 0..50 us run returns 0. Original two-sample preflight also PASS.
- Root cause: installed Icarus cannot resolve generated drive-qualified absolute includes; use run-local relative MODULE_SOURCE. Controller arithmetic and Step 5B vectors unchanged.
- Full Step 5C 46,090-vector runtime parity remains PENDING; no power-stage integration performed. This supersedes the earlier statement that no Verilog files have been created.
- Evidence: DAC test/CURRENT_CONTROL/STEP5C_INTERFACE_AUDIT.md.
'''
for name in ['PROJECT_STATUS.md','TODO.md']:
    p=repo/'docs'/name
    with p.open('ab') as f:f.write(append.encode('utf-8'))
print(cc/'STEP5C_INTERFACE_AUDIT.md')
