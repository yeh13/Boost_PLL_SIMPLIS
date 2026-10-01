"""Interface-only ablation: no arithmetic edits, no Step 5B vector changes."""
import csv
import json
import shutil
import subprocess
import sys
import time
from datetime import datetime,timezone
from pathlib import Path
sys.dont_write_bytecode=True
from step5c_preflight import HERE,CC,IV,VVP,ENGINE,CONFIG,SOURCE,FIELDS,env,sha,sx,short


def run_case(root,n):
    folder=root/f'ports_{n:02d}';folder.mkdir()
    names=FIELDS[:n]
    widths={k:64 if k in ['acc','raw'] else 32 for k in FIELDS}
    shutil.copyfile(SOURCE,folder/SOURCE.name)
    stim='''`timescale 1ns/1ps
module STIM(output reg clk,output reg signed [31:0] e,base,output reg mode);
initial begin clk=0;e=3;base=0;mode=1;end
always #12500 clk=~clk;
endmodule
'''
    (folder/'stim.v').write_text(stim)
    declarations=','.join('output signed [%d:0] %s'%(widths[k]-1,k) for k in names)
    internal='\n'.join('wire signed [%d:0] %s;'%(widths[k]-1,k) for k in FIELDS if k not in names)
    wrapper='''`timescale 1ns/1ps
`include "STEP5C_CONTROLLER_RUNTIME.v"
module AUDIT(input clk,input signed [31:0] e,base,input mode,output reg heartbeat EXTRA);
INTERNAL
STEP5C_CONTROLLER_RUNTIME dut(clk,1'b0,mode,e,base,acc,raw,limited,finalDuty,effective_correction,x1,x2,y1,y2);
initial begin
 heartbeat=0;
 PROBES
end
always @(posedge clk) begin #0.001;heartbeat=~heartbeat;end
endmodule
'''.replace('EXTRA',','+declarations if declarations else '').replace('INTERNAL',internal).replace('PROBES','\n '.join('$simplis_vpi_probe('+k+');' for k in ['heartbeat']+names))
    (folder/'audit.v').write_text(wrapper)
    inputs=list(range(1,67)) # clk 1, e 32, base 32, mode 1
    outputs=list(range(100,101+sum(widths[k] for k in names)))
    net=f'''* Step5C interface audit, stage {n}
.TRAN 50u 0
.OPTIONS PSP_START=0 PSP_NPT=101 SAVE_INSTANTS_FILES
.KEEP *V
!V_STIM [ {' '.join(map(str,inputs))} ] [ ] MODEL=STIM_MODEL
.MODEL STIM_MODEL VERILOG_HDL_MODULE MODULE="STIM" MODULE_SOURCE="stim.v"
+ OUTPUT="clk,1,e,32,base,32,mode,1" INPUT="" GNDREF='N'
+ RIN=10Meg ROUT=10 TH=2.5 HYSTWD=0.1 VOL=0 VOH=5
!V_AUDIT [ {' '.join(map(str,outputs))} ] [ {' '.join(map(str,inputs))} ] MODEL=AUDIT_MODEL
.MODEL AUDIT_MODEL VERILOG_HDL_MODULE MODULE="AUDIT" MODULE_SOURCE="audit.v"
+ OUTPUT="heartbeat,1{''.join(','+k+','+str(widths[k]) for k in names)}" INPUT="clk,1,e,32,base,32,mode,1" GNDREF='N'
+ RIN=10Meg ROUT=10 TH=2.5 HYSTWD=0.1 VOL=0 VOH=5
VANCHOR 9000 0 1
RANCHOR 9000 0 1k
.END
'''
    (folder/'input.net').write_text(net)
    sx(f'PreProcessNetlist "{(folder/"input.net").as_posix()}" "{(folder/"engine.deck").as_posix()}"',folder)
    assert not list(folder.glob('*.t1')) and not list(folder.glob('*.t2'))
    start=time.time_ns()
    try:
        p=subprocess.run([str(ENGINE),'engine.deck','-f'],cwd=folder,env=env(),capture_output=True,text=True,timeout=40,creationflags=subprocess.CREATE_NO_WINDOW)
        code=p.returncode; log=p.stdout+p.stderr
    except subprocess.TimeoutExpired as e:
        code=-999;log='TIMEOUT\n'+str(e.stdout)+str(e.stderr)
    end=time.time_ns()
    (folder/'engine.log').write_text(log)
    result=dict(stage=n,added_port=names[-1] if names else 'heartbeat only',outputs=names,engine_return_code=code)
    error=folder/'engine.deck.err'
    if error.exists():result['engine_error']=error.read_text(errors='replace')
    if code==0:
        for ext in ['t1','t2']:
            path=folder/('engine.deck.'+ext)
            assert path.exists() and path.stat().st_size>0 and start-2_000_000_000<=path.stat().st_mtime_ns<=end+2_000_000_000
            result[ext+'_sha256']=sha(path)
        with (folder/'engine.deck.t2').open() as f:
            assert next(f).startswith('$$$');next(f);next(f)
            count=int(next(f).split()[0]);nv=int(next(f).split()[0])
            for _ in range(nv+1):next(f)
            times=[float(line.split()[0]) for line in f if line.strip()]
        assert len(times)==count and count>=101 and times[0]==0 and abs(times[-1]-50e-6)<1e-12
        result.update(status='PASS',points=count,start=times[0],stop=times[-1])
    else:result['status']='FAIL'
    (folder/'result.json').write_text(json.dumps(result,indent=2))
    print(json.dumps(result),flush=True)
    return result


def main():
    root=HERE/'interface_audit'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S_%fZ');root.mkdir(parents=True)
    manifest={'iverilog':str(IV),'vvp':str(VVP),'config_sha256':sha(CONFIG),'vpi_sha256':sha(ENGINE.parent/'simplis_verilog.vpi'),
        'controller_sha256':sha(SOURCE),'PATH':env()['PATH'],'input_port_count':66,'ground':'N'}
    manifest['version']=subprocess.run([str(IV),'-V'],env=env(),capture_output=True,text=True).stdout.splitlines()[0]
    (root/'manifest.json').write_text(json.dumps(manifest,indent=2))
    results=[]
    for n in range(10):
        result=run_case(root,n);results.append(result)
        if result['status']!='PASS':break
    assert sha(SOURCE)==manifest['controller_sha256']
    (root/'results.json').write_text(json.dumps(results,indent=2))
    print(root)


if __name__=='__main__':main()
