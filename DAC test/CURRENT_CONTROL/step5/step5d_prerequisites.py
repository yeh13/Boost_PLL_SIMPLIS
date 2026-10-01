"""Step 5D prerequisite-only evidence. Never executes a power-stage deck."""
import json,re,shutil,subprocess,sys,time
from datetime import datetime,timezone
sys.dont_write_bytecode=True
from step5c_preflight import HERE,CC,SOURCE,ENGINE,env,sha,sx

BASE=CC/'Bidirection High Gain Inverter_Current_Control.sxsch'

def main():
    root=HERE/'step5d'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S_%fZ');root.mkdir(parents=True)
    print(root,flush=True)
    baseline_hash=sha(BASE)
    assert baseline_hash=='23c1708af6a6acd5b25d24b72d85e81aad96e98936be95b1872494482ed1ecaa'
    copy=root/'step5d_integration.sxsch';shutil.copyfile(BASE,copy)
    for src,name in [(BASE,'baseline'),(copy,'integration')]:
        sx(f'Netlist /path "{src.as_posix()}" "{(root/(name+".net")).as_posix()}"',root)
    canonical=lambda p:'\n'.join(l for l in p.read_text(encoding='utf-8-sig').splitlines() if l.strip() and not l.lstrip().startswith('*'))
    assert sha(copy)==baseline_hash
    assert canonical(root/'baseline.net')==canonical(root/'integration.net'),'Electrical difference: STOP'
    integrity={'status':'PASS','baseline_sha256':baseline_hash,'copy_sha256':sha(copy),'netlist_electrically_identical':True}
    (root/'integrity.json').write_text(json.dumps(integrity,indent=2))
    print('Schematic/netlist integrity PASS',flush=True)
    run=root/'bridge';run.mkdir()
    # 14-bit unsigned bus is sufficient for the already-clamped 0..10625 command.
    (run/'bridge_stim.v').write_text('''`timescale 1ns/1ps
module BRIDGE_STIM(output reg [13:0] finalDuty_counts);
initial begin finalDuty_counts=0; #25000 finalDuty_counts=3125;
#25000 finalDuty_counts=6250; #25000 finalDuty_counts=9375;
#25000 finalDuty_counts=10625; end
endmodule
''')
    bits=list(range(100,114))
    # Voltage-controlled sources sum bit weights, with high=1 V and denominator12500.
    bridge=[]
    for n,node in enumerate(bits):
        out=200+n;prev=0 if n==0 else out-1
        bridge.append(f'EW{n} {out} {prev} {node} 0 {2**n/12500:.17g}')
    bridge.append('E_DBOOST 300 0 213 0 1')
    (run/'input.net').write_text('''* Standalone integer-to-analog duty bridge only
.TRAN 125u 0
.OPTIONS PSP_START=0 PSP_NPT=501 SAVE_INSTANTS_FILES
.KEEP *V
!V_STIM %%gnd_ref 0 [ '''+' '.join(map(str,bits))+''' ] [ ] MODEL=STIM_MODEL
.MODEL STIM_MODEL VERILOG_HDL_MODULE MODULE="BRIDGE_STIM" MODULE_SOURCE="bridge_stim.v"
+ OUTPUT="finalDuty_counts,14" INPUT="" GNDREF='Y'
+ RIN=10Meg ROUT=10 TH=0.5 HYSTWD=0.1 VOL=0 VOH=1
'''+ '\n'.join(bridge)+'\n.END\n')
    sx(f'PreProcessNetlist "{(run/"input.net").as_posix()}" "{(run/"engine.deck").as_posix()}"',run)
    assert not list(run.glob('*.t1')) and not list(run.glob('*.t2'))
    start=time.time_ns()
    p=subprocess.run([str(ENGINE),'engine.deck','-f'],cwd=run,env=env(),capture_output=True,text=True,creationflags=subprocess.CREATE_NO_WINDOW)
    end=time.time_ns();(run/'engine.log').write_text(p.stdout+p.stderr)
    assert p.returncode==0,('engine STOP',p.returncode)
    files={}
    for ext in ['t1','t2']:
        path=run/('engine.deck.'+ext)
        assert path.exists() and path.stat().st_size and start-2_000_000_000<=path.stat().st_mtime_ns<=end+2_000_000_000
        files[ext]={'sha256':sha(path),'mtime_ns':path.stat().st_mtime_ns}
    with (run/'engine.deck.t2').open() as f:
        assert next(f).startswith('$$$');next(f);next(f)
        count=int(next(f).split()[0]);nv=int(next(f).split()[0])
        names=[next(f).split()[1] for _ in range(nv+1)]
        data=[[float(v) for v in line.split()] for line in f if line.strip()]
    assert len(data)==count and count>=501 and data[0][0]==0 and abs(data[-1][0]-125e-6)<1e-12
    assert all(a[0]<=b[0] for a,b in zip(data,data[1:]))
    col=names.index('V(300)');mapping=[]
    for i,value in enumerate([0,3125,6250,9375,10625]):
        subset=[row[col] for row in data if (i*25+5)*1e-6<=row[0]<((i+1)*25-1)*1e-6]
        assert subset
        error=max(abs(v-value/12500) for v in subset)
        assert error<1e-6,(value,error)
        mapping.append({'counts':value,'expected':value/12500,'measured':subset[len(subset)//2],'max_error':error})
    result={'integrity':integrity,'bridge':'PASS','engine_return_code':p.returncode,'freshness':'PASS','start_ns':start,'end_ns':end,'files':files,'points':count,'start':data[0][0],'stop':data[-1][0],'mapping':mapping}
    (root/'results.json').write_text(json.dumps(result,indent=2))
    assert sha(BASE)==baseline_hash
    print(json.dumps(result,indent=2),flush=True)

if __name__=='__main__':main()
