"""Prepare integration artifacts only. No engine invocation."""
import json,re,shutil,sys
sys.dont_write_bytecode=True
from step5c_preflight import HERE,CC,SOURCE,sha,sx
root=HERE/'step5d/20261001T075810_042316Z'
baseline=CC/'Bidirection High Gain Inverter_Current_Control.sxsch'
copy=root/'step5d_integration.sxsch'
for src,name in [(baseline,'baseline_simplis.net'),(copy,'integration_simplis.net')]:
    sx(f'OpenSchem "{src.as_posix()}"\nNetlist /simplis "{(root/name).as_posix()}"',root)
canonical=lambda p:'\n'.join(l for l in p.read_text(encoding='utf-8-sig').splitlines() if l.strip() and not l.lstrip().startswith('*'))
assert canonical(root/'baseline_simplis.net')==canonical(root/'integration_simplis.net'),'STOP: electrical difference'
source=(root/'integration_simplis.net').read_text(encoding='utf-8-sig')
assert 'R31 190 286 196' in source and '.var Vdc=100' in source and '.var Z=196' in source
prepared=root/'prepared';prepared.mkdir(exist_ok=False)
shutil.copyfile(SOURCE,prepared/SOURCE.name)
pwm=CC.parents[1]/'Boost_I_loop_inverter.X/mcc_generated_files/pwm.c'
table=re.search(r'const int16_t VrefTable\[334\]\s*=\s*\{([^}]+)\}',pwm.read_text()).group(1)
values=[int(v) for v in re.findall(r'-?\d+',table)]
assert len(values)==334
(prepared/'VrefTable.hex').write_text('\n'.join(f'{v & 65535:04x}' for v in values)+'\n')
# Candidate adapter only; untouched Step5C core provides all correction arithmetic.
(prepared/'STEP5D_COMMAND.v').write_text('''`timescale 1ns/1ps
`include "STEP5C_CONTROLLER_RUNTIME.v"
module STEP5D_COMMAND(input clk_40k,reset_sample,input [8:0] table_index,
 input signed [31:0] e,output [13:0] finalDuty_counts);
reg signed [15:0] VrefTable[0:333];
reg signed [31:0] vref_cmd,Boost_PWM;
reg boost_mode;
wire signed [63:0] acc,raw;
wire signed [31:0] limited,finalDuty,effective_correction,x1,x2,y1,y2;
initial $readmemh("VrefTable.hex",VrefTable);
always @* begin
 vref_cmd=VrefTable[table_index]; boost_mode=(vref_cmd>=574);
 Boost_PWM=0;
 if(boost_mode) begin
  Boost_PWM=(vref_cmd-574)*12500;
  Boost_PWM=Boost_PWM/(vref_cmd+574);
  if(Boost_PWM>12500) Boost_PWM=12500;
  if(Boost_PWM<0) Boost_PWM=0;
 end
end
STEP5C_CONTROLLER_RUNTIME core(clk_40k,reset_sample,boost_mode,e,Boost_PWM,
 acc,raw,limited,finalDuty,effective_correction,x1,x2,y1,y2);
assign finalDuty_counts=finalDuty[13:0];
endmodule
''')
# Library top-level inputs: clk, reset, index bits0..8, error bits0..31.
pins=['CLK','RESET']+['INDEX'+str(i) for i in range(9)]+['ERR'+str(i) for i in range(32)]
bits=[str(100+i) for i in range(14)]
lib=['.SUBCKT STEP5D_COMMAND_BRIDGE DUTY '+' '.join(pins),
'!V_CMD %%gnd_ref 0 [ '+' '.join(bits)+' ] [ '+' '.join(pins)+' ] MODEL=COMMAND_MODEL',
'.MODEL COMMAND_MODEL VERILOG_HDL_MODULE MODULE="STEP5D_COMMAND" MODULE_SOURCE="STEP5D_COMMAND.v"',
'+ OUTPUT="finalDuty_counts,14" INPUT="clk_40k,1,reset_sample,1,table_index,9,e,32" GNDREF=\'Y\'',
'+ RIN=10Meg ROUT=10 TH=0.5 HYSTWD=0.1 VOL=0 VOH=1']
for n,node in enumerate(bits):lib.append(f'EW{n} {200+n} {0 if n==0 else 199+n} {node} 0 {2**n/12500:.17g}')
lib+=['E_DBOOST DUTY 0 213 0 1','.ENDS STEP5D_COMMAND_BRIDGE']
(prepared/'command_bridge.inc').write_text('\n'.join(lib)+'\n')
changes=[]
for ref in ['U1','U14']:
    pattern=r'(?m)^X\$'+ref+r' .+$'
    old=re.search(pattern,source).group(0);parts=old.split()
    assert parts[4]=='238'
    parts[4]='9001';new=' '.join(parts)
    source=source.replace(old,new);changes.append({'before':old,'after':new})
# Explicit external terminals await the next round's sampled feedback/reference adapter.
extra='\n.include "command_bridge.inc"\nXSTEP5D 9001 '+' '.join('STEP5D_'+p for p in pins)+' STEP5D_COMMAND_BRIDGE\n'
source=re.sub(r'(?im)^\.end\s*$','',source)
(prepared/'integration_candidate.net').write_text('* PREPARED ONLY: bind external Step5D inputs before any run\n'+source+extra+'\n.END\n')
manifest={'comparator_only_changes':changes,'external_input_contract':pins,'core_sha256':sha(SOURCE),'pwm_sha256':sha(pwm),'table_hex_sha256':sha(prepared/'VrefTable.hex'),'baseline_simplis_sha256':sha(root/'baseline_simplis.net'),'integration_simplis_sha256':sha(root/'integration_simplis.net'),'prepared_only':True}
(prepared/'manifest.json').write_text(json.dumps(manifest,indent=2))
assert sha(baseline)==sha(copy)=='23c1708af6a6acd5b25d24b72d85e81aad96e98936be95b1872494482ed1ecaa'
report='''# Step 5D prerequisites — integrity and analog duty bridge

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
'''
(CC/'STEP5D_INTEGRITY_BRIDGE.md').write_text(report,encoding='utf-8')
update='\n\n## Step 5D prerequisites — 2026-10-01\n\nSchematic integrity PASS (identical SHA256 and fresh SIMPLIS netlist). Standalone digital-to-analog /12500 bridge PASS: 0/3125/6250/9375/10625 -> 0/.25/.5/.75/.85; engine0, fresh505-point t1/t2, 0..125us. Prepared separate Boost command candidate changes only U1/U14 command input; Buck and baseline load remain unchanged. Closed-loop not run. Next round must bind sampled current/reference inputs and resolve run-local LUT data path before half-load execution. See DAC test/CURRENT_CONTROL/STEP5D_INTEGRITY_BRIDGE.md.\n'
for name in ['PROJECT_STATUS.md','TODO.md']:
    with (CC.parents[1]/'docs'/name).open('ab') as f:f.write(update.encode('utf-8'))
with (CC/'STEP5C_INTERFACE_AUDIT.md').open('ab') as f:f.write(b'\n\nStep5D generated-root clarification: instance bus pins are LSB-first (first pin maps bit0), not descending [31:0]. Core and runtime CSV parity unchanged.\n')
print('Prepared artifacts and report complete; no closed-loop run.')
