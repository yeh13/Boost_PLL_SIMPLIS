"""Ordered Step 5C toolchain checks and minimal SIMPLIS co-simulation."""
import csv
import ctypes
import hashlib
import json
import os
import re
import subprocess
import sys
import time
from datetime import datetime, timezone
from pathlib import Path

sys.dont_write_bytecode=True
HERE=Path(__file__).resolve().parent
CC=HERE.parent
sys.path.insert(0,str(CC/'step5a_reconstruction'))
from step5a_run import sx

BIN=Path('C:/Program Files/SIMetrix830/iverilog/10.1.1/bin')
IV=BIN/'iverilog.exe'; VVP=BIN/'vvp.exe'
ENGINE=Path('C:/Program Files/SIMetrix830/bin/simplis.exe')
CONFIG=ENGINE.parent/'simplis_verilog.cfg'
SOURCE=HERE/'STEP5C_CONTROLLER_RUNTIME.v'
FIELDS='acc raw limited finalDuty effective_correction x1 x2 y1 y2'.split()


def sha(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()


def short(p):
    buf=ctypes.create_unicode_buffer(32768)
    assert ctypes.windll.kernel32.GetShortPathNameW(str(p),buf,len(buf))
    return buf.value.replace('\\','/')


def env():
    result=os.environ.copy()
    result['PATH']=str(BIN)+os.pathsep+result.get('PATH','')
    return result


def call(args,folder,label):
    p=subprocess.run(list(map(str,args)),cwd=folder,env=env(),capture_output=True,text=True,
                     creationflags=subprocess.CREATE_NO_WINDOW)
    (folder/(label+'.log')).write_text(p.stdout+p.stderr,encoding='utf-8')
    assert p.returncode==0,(label,p.returncode,p.stdout,p.stderr)
    return p.stdout+p.stderr


def fixture(folder, samples):
    # Inputs format: reset, deadband error, integer feedforward, boost mode.
    # The clock comes from SIMPLIS in cosim, from a testbench only in vvp smoke.
    wrapper='''`timescale 1ns/1ps
`include "STEP5C_CONTROLLER_RUNTIME.v"
module STEP5C_REPLAY(input clk_40k, output reg heartbeat);
reg reset_sample, boost_mode;
reg signed [31:0] e, Boost_PWM_counts;
wire signed [63:0] acc,raw;
wire signed [31:0] limited,finalDuty,effective_correction,x1,x2,y1,y2;
integer fi,fo,rc,n,r,m;
STEP5C_CONTROLLER_RUNTIME dut(clk_40k,reset_sample,boost_mode,e,Boost_PWM_counts,
 acc,raw,limited,finalDuty,effective_correction,x1,x2,y1,y2);
task load_next;
begin
 rc=$fscanf(fi,"%d %d %d %d\\n",r,e,Boost_PWM_counts,m);
 if(rc!=4) begin $display("INPUT_READ_FAIL %d",n); $finish; end
 reset_sample=r; boost_mode=m;
end
endtask
initial begin
 heartbeat=0; n=0;
 fi=$fopen("INPUT_PATH","r"); fo=$fopen("OUTPUT_PATH","w");
 if(fi==0 || fo==0) begin $display("FILE_OPEN_FAIL"); $finish; end
 $fwrite(fo,"sample,time_ns,acc,raw,limited,finalDuty,effective_correction,x1,x2,y1,y2\\n");
 load_next;
end
always @(posedge clk_40k) begin
 #0.001;
 if(n < SAMPLE_COUNT) begin
  $fwrite(fo,"%0d,%0.3f,%0d,%0d,%0d,%0d,%0d,%0d,%0d,%0d,%0d\\n",
    n,$realtime,acc,raw,limited,finalDuty,effective_correction,x1,x2,y1,y2);
  n=n+1; heartbeat=~heartbeat;
  if(n == SAMPLE_COUNT) begin $fclose(fi); $fclose(fo); end
  else load_next;
 end
end
endmodule
'''.replace('INPUT_PATH',(folder/'vectors.txt').as_posix()).replace('OUTPUT_PATH',(folder/'runtime.csv').as_posix()).replace('SAMPLE_COUNT',str(samples))
    (folder/'STEP5C_CONTROLLER_RUNTIME.v').write_bytes(SOURCE.read_bytes())
    (folder/'replay.v').write_text(wrapper)
    (folder/'clock.v').write_text('`timescale 1ns/1ps\nmodule STEP5C_CLOCK(output reg clk_40k); initial clk_40k=0; always #12500 clk_40k=~clk_40k; endmodule\n')
    duration=samples/40000
    deck=f'''* Step 5C digital-only co-simulation, no duty bridge/power stage
.TRAN {duration:.12f} 0
.OPTIONS PSP_START=0 PSP_NPT={samples*2+1} SAVE_INSTANTS_FILES
.KEEP *V
!V_CLOCK [ 1 ] [ ] MODEL=CLOCK_MODEL
.MODEL CLOCK_MODEL VERILOG_HDL_MODULE MODULE="STEP5C_CLOCK"
+ MODULE_SOURCE="clock.v"
+ OUTPUT="clk_40k,1" INPUT=""
+ RIN=10Meg ROUT=10 TH=2.5 HYSTWD=0.1 VOL=0 VOH=5 GNDREF='N'
!V_REPLAY [ 2 ] [ 1 ] MODEL=REPLAY_MODEL
.MODEL REPLAY_MODEL VERILOG_HDL_MODULE MODULE="STEP5C_REPLAY"
+ MODULE_SOURCE="replay.v"
+ OUTPUT="heartbeat,1" INPUT="clk_40k,1"
+ RIN=10Meg ROUT=10 TH=2.5 HYSTWD=0.1 VOL=0 VOH=5 GNDREF='N'
* Isolated numerical anchor, no connection to the digital controller
VANCHOR 10 0 1
RANCHOR 10 0 1k
.END
'''
    (folder/'input.net').write_text(deck)
    return duration


def cosim(folder,samples,prepare=True):
    duration=fixture(folder,samples) if prepare else samples/40000
    assert not list(folder.glob('*.t1')) and not list(folder.glob('*.t2')) and not (folder/'runtime.csv').exists()
    sx(f'PreProcessNetlist "{(folder/"input.net").as_posix()}" "{(folder/"engine.deck").as_posix()}"',folder)
    deck=folder/'engine.deck'
    assert deck.exists() and not re.search(r'(?im)^\.var\b|\bvars:',deck.read_text())
    start=time.time_ns()
    call([ENGINE,deck.name,'-f'],folder,'engine')
    end=time.time_ns()
    files={}
    for name in ['engine.deck.t1','engine.deck.t2','runtime.csv']:
        p=folder/name
        assert p.exists() and p.stat().st_size>0,name
        assert start-2_000_000_000<=p.stat().st_mtime_ns<=end+2_000_000_000,name
        files[name]=dict(sha256=sha(p),bytes=p.stat().st_size,mtime_ns=p.stat().st_mtime_ns)
    with (folder/'engine.deck.t2').open() as f:
        assert next(f).startswith('$$$')
        assert next(f).strip()=='INPUT FILE: engine.deck'
        assert next(f).strip()=='Transient Analysis'
        count=int(next(f).split()[0]); variables=int(next(f).split()[0])
        names=[next(f).split()[1] for _ in range(variables+1)]
        times=[]
        for line in f:
            if not line.strip():continue
            values=line.split(); assert len(values)==len(names)
            times.append(float(values[0]))
    assert len(times)==count and count>=samples*2+1
    assert times[0]==0 and abs(times[-1]-duration)<1e-12
    assert all(a<=b for a,b in zip(times,times[1:]))
    result=dict(engine_return_code=0,freshness='PASS',start_ns=start,end_ns=end,files=files,
                point_count=count,start=times[0],stop=times[-1],samples=samples,
                deck_sha256=sha(deck),config_sha256=sha(CONFIG),source_sha256=sha(SOURCE))
    (folder/'freshness.json').write_text(json.dumps(result,indent=2))
    return result


def main():
    folder=HERE/'preflight'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S_%fZ')
    folder.mkdir(parents=True,exist_ok=False)
    cfg=CONFIG.read_text()
    assert 'version = "10.1.1"' in cfg and 'verilog_bin_dir = "..\\iverilog\\10.1.1\\bin"' in cfg
    version=call([IV,'-V'],folder,'iverilog_version')
    assert 'v10_1_1' in version
    call([VVP,'-V'],folder,'vvp_version')
    smoke='module smoke; initial begin if ((-64\'sd32769 >>> 15) != -2) $finish; $display("SMOKE_PASS"); $finish; end endmodule\n'
    (folder/'smoke.v').write_text(smoke)
    call([IV,'-g2005','-s','smoke','-o','smoke.vvp','smoke.v'],folder,'smoke_compile')
    assert 'SMOKE_PASS' in call([VVP,'smoke.vvp'],folder,'smoke_runtime')
    call([IV,'-g2005','-s','STEP5C_CONTROLLER_RUNTIME','-o','controller.vvp',SOURCE],folder,'controller_compile')
    # Known directed two-sample check exercises signed shift and state update.
    (folder/'vectors.txt').write_text('1 3 0 1\n0 -3 0 1\n')
    result=cosim(folder,2)
    rows=list(csv.DictReader((folder/'runtime.csv').open()))
    assert len(rows)==2
    assert [int(rows[0][k]) for k in FIELDS]==[78000,2,2,2,2,3,0,2,0]
    assert [int(rows[1][k]) for k in FIELDS]==[-113980,-4,-4,0,0,-3,3,0,2]
    result.update(status='PASS',iverilog=str(IV),vvp=str(VVP),version=version.splitlines()[0],
        stages=['minimal Verilog smoke','iverilog compile','vvp runtime','controller compile','SIMPLIS co-simulation'],
        path_policy='prepend only Icarus 10.1.1 bin to child PATH; global environment/config unchanged')
    (folder/'preflight.json').write_text(json.dumps(result,indent=2))
    print(json.dumps(result,indent=2)); print(folder)


if __name__=='__main__':main()
