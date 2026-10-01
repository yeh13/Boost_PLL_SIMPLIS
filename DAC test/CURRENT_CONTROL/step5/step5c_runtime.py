"""Replay recorded Step 5B controller inputs; fail closed before parity analysis."""
import csv
import json
import re
import shutil
import sys
from collections import Counter
from datetime import datetime, timezone
from decimal import Decimal
sys.dont_write_bytecode=True
from step5c_preflight import HERE, CC, SOURCE, FIELDS, sha, fixture, cosim


def rows(p):
    with p.open(newline='') as f:return list(csv.DictReader(f))


def main():
    folder=HERE/'runtime'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S_%fZ')
    folder.mkdir(parents=True,exist_ok=False)
    print('Fresh run: '+str(folder),flush=True)
    try:
        run(folder)
    except Exception as e:
        (folder/'STOP.json').write_text(json.dumps({'status':'STOP','error':repr(e)},indent=2))
        raise


def run(folder):
    baseline=CC/'step5b_results/20261001T073208_039093Z'
    metrics=json.loads((baseline/'metrics.json').read_text())
    original={}
    for name in ['inputs.csv','python.csv','c_oracle.csv']:
        p=baseline/name
        assert sha(p)==metrics['evidence_sha256'][name],name
        original[name]=sha(p)
        shutil.copyfile(p,folder/name)
        assert sha(folder/name)==original[name]
    inputs=rows(folder/'inputs.csv'); py=rows(folder/'python.csv'); c=rows(folder/'c_oracle.csv')
    assert len(inputs)==len(py)==len(c)==46090
    assert py==c
    groups=Counter(r['group'] for r in inputs)
    assert len(groups)==7
    core_hash=sha(SOURCE)
    # Projection only: preserve every row and reset flag; no model/vector regeneration.
    with (folder/'vectors.txt').open('w') as f:
        for i,p in zip(inputs,py):
            f.write(f"{int(i['reset'])} {int(p['err'])} {int(p['Boost_PWM'])} {int(p['mode'])}\n")
    fixture(folder,len(inputs))
    wrapper=folder/'replay.v'
    content=wrapper.read_text()
    probes='\n'.join(' $simplis_vpi_probe('+k+');' for k in FIELDS)
    content=content.replace(' heartbeat=0; n=0;',' heartbeat=0; n=0;\n'+probes)
    wrapper.write_text(content)
    assert sha(folder/SOURCE.name)==core_hash
    for p in [folder/'input.net',wrapper]:
        for path in re.findall(r'(?:MODULE_SOURCE=|`include\s+)"([^"]+)"',p.read_text()):
            assert ':' not in path and not path.startswith(('/','\\')),path
    manifest=dict(source_hashes=original,controller_sha256=core_hash,groups=dict(groups),samples=46090,
                  vectors_sha256=sha(folder/'vectors.txt'),runner_sha256=sha(HERE/'step5c_runtime.py'))
    (folder/'manifest.json').write_text(json.dumps(manifest,indent=2))
    print('Running SIMPLIS: 46090 samples at 40 kHz, 1.15225 seconds.',flush=True)
    fresh=cosim(folder,46090,prepare=False)
    # Gate compile/runtime logs before any integer comparison.
    for name in ['engine.log','engine.vss_root.v.err','engine.vss_root.v.log']:
        p=folder/name
        if p.exists():
            assert not re.search(r'FATAL|INPUT_READ_FAIL|FILE_OPEN_FAIL|error\(s\) during elaboration',p.read_text(errors='replace')),name
    assert (folder/'engine.vss_root.vvp').exists()
    for path in re.findall(r'`include\s+"([^"]+)"',(folder/'engine.vss_root.v').read_text()):
        assert ':' not in path and not path.startswith(('/','\\')),path
    actual=rows(folder/'runtime.csv')
    assert len(actual)==46090
    for n,row in enumerate(actual):
        assert int(row['sample'])==n
        assert Decimal(row['time_ns'])==Decimal(12500+n*25000)+Decimal('0.001')
    print('Freshness and runtime cadence PASS; comparing nine integer fields.',flush=True)
    comparisons={}
    for field in FIELDS:
        differences=[abs(int(a[field])-int(p[field])) for a,p in zip(actual,py)]
        differences_c=[abs(int(a[field])-int(p[field])) for a,p in zip(actual,c)]
        assert differences==differences_c
        comparisons[field]={'mismatch_count':sum(d!=0 for d in differences),'max_difference':max(differences)}
    for name,digest in original.items():assert sha(baseline/name)==digest
    assert sha(SOURCE)==core_hash
    for name,info in fresh['files'].items():assert sha(folder/name)==info['sha256']
    passed=all(v['mismatch_count']==v['max_difference']==0 for v in comparisons.values())
    result=dict(status='PASS' if passed else 'FAIL',samples=46090,groups=dict(groups),duration_seconds=1.15225,
                fields=comparisons,freshness=fresh,manifest=manifest)
    (folder/'parity.json').write_text(json.dumps(result,indent=2))
    report='# Step 5C full runtime parity\n\nSTEP 5C RUNTIME PARITY '+result['status']+'\n\n'
    report+=f'Evidence: `{folder.relative_to(CC).as_posix()}`.\n\n'
    report+='Reused all 46,090 recorded Step 5B samples in their original seven-group order; no vectors regenerated. Controller inputs are a row-preserving projection of reset plus the saved, Python/C-validated err, Boost_PWM and mode fields. The input/reference chain is not reimplemented here.\n\n'
    report+='40 kHz; period 25 us; simulation 0..1.152250 s. First controller sample at 12.5 us, then every 25 us; exact integer trace sampled 1 ps after each edge to observe updated states. 46,090 sequential sample IDs and all timestamps verified.\n\n'
    report+='| Field | Python mismatch | C mismatch | Max difference |\n|---|---:|---:|---:|\n'
    for k,v in comparisons.items():report+=f"|{k}|{v['mismatch_count']}|{v['mismatch_count']}|{v['max_difference']}|\n"
    report+='\nAll nine signals register $simplis_vpi_probe using the audited local syntax. Exact signed integer comparison uses $fwrite from the same SIMPLIS-launched Verilog runtime, avoiding precision loss in analog t2 export. Runtime CSV was absent before execution and is included in the freshness/hash gate. No standalone-vvp result substitutes for co-simulation.\n\n'
    report+=f"Engine return code 0; freshness PASS; fresh t1/t2 and runtime.csv; t2 point count {fresh['point_count']}; start/stop and monotonicity verified. Timestamp tolerance remains the established +/-2 seconds for USB filesystem rounding. All file SHA256 values and execution timestamps are recorded in freshness.json / parity.json.\n\n"
    report+='Sources, vectors and data reside in the fresh isolated directory. MODULE_SOURCE and generated root includes are relative only. Data-file $fopen paths address this run directory explicitly because the installed configuration launches vvp from its binary directory; these are not Verilog source/include paths. GNDREF=\'N\'.\n\n'
    report+='Root cause resolved earlier: absolute MODULE_SOURCE/include path incompatible with local Icarus invocation. This run does not repeat interface audit. Controller arithmetic is unchanged (SHA256 below), including signed64 accumulation, arithmetic shift, clamps, effective-correction write-back and Buck clear.\n\n'
    report+=f'Controller SHA256: `{core_hash}`\n\nGroups: `{dict(groups)}`\n\n'
    report+='DIGITAL DOMAIN ONLY. PHYSICAL VOLTAGE CALIBRATION: NOT FORMALLY RE-VALIDATED. No analog duty bridge, power stage or Step 5D execution.\n'
    (CC/'STEP5C_RUNTIME_STATUS.md').write_text(report,encoding='utf-8')
    assert passed,'Runtime parity FAIL: stop; inspect parity.json'
    update='\n\n## Step 5C full runtime completion — 2026-10-01\n\nSTEP 5C RUNTIME PARITY PASS: 46,090 samples, 7 groups, 9 fields, mismatch=0 and max difference=0 against both recorded Step 5B Python/C oracles. Engine return=0; fresh datasets and exact 40 kHz cadence PASS; duration 1.152250 s. Supersedes pending full-runtime statements above. Root cause: absolute MODULE_SOURCE/include path incompatible with local Icarus invocation; relative source paths used. Controller and Step 5B vectors preserved. Step 5D prerequisites are eligible for a later authorized round; not executed. See DAC test/CURRENT_CONTROL/STEP5C_RUNTIME_STATUS.md.\n'
    for p in [CC/'STEP5C_INTERFACE_AUDIT.md',CC.parents[1]/'docs/PROJECT_STATUS.md',CC.parents[1]/'docs/TODO.md']:
        with p.open('ab') as f:f.write(update.encode('utf-8'))
    print(json.dumps({'status':result['status'],'samples':46090,'fields':comparisons,'fresh_points':fresh['point_count'],'folder':str(folder)},indent=2))


if __name__=='__main__':main()
