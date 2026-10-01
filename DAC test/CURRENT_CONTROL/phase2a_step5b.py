"""Step 5B: reuse Step 4C arithmetic; new seven-group command/CSV validation."""
import csv
import ctypes
import json
import random
import re
import subprocess
import sys
from datetime import datetime, timezone
from decimal import Decimal
from pathlib import Path

sys.dont_write_bytecode = True
import phase2a_step4c as core

ROOT = Path(__file__).resolve().parent
FIELDS = core.FIELDS


def vectors(table):
    for tick in range(12000):
        pos = (tick*60*668//40000) % 668
        index, phase = pos % 334, int(pos >= 334)
        amplitude = round(table[index] * .23)
        yield 'steady_state', tick == 0, [2048+(amplitude if phase else -amplitude),index,phase,0,0,0,0]
    for tick in range(8000):
        ref = [573,574,575,3787][(tick//20)%4]
        phase = (tick//80)%2
        yield 'mode_transition', tick == 0, [1500+(tick*37)%1100,0,phase,1,1,ref,600 if phase else -600]
    first = True
    for phase in [0,1]:
        for ref in [574,3787]:
            for adc in [0,4095]:
                for _ in range(1024):
                    yield 'clamp_stress', first, [adc,0,phase,1,1,ref,925 if phase else -925]
                    first = False
    rng = random.Random(50002)
    for _ in range(8192):
        phase=rng.randrange(2)
        yield 'clamp_stress', False, [rng.randrange(4096),0,phase,1,1,rng.choice([574,575,3787]),rng.randrange(-925,926)]
    for tick in range(4000):
        # Dirty Boost history -> several Buck callbacks -> fresh Boost history.
        boost = tick%20 < 10
        phase = (tick//20)%2
        yield 'buck_state_clear', tick == 0, [3500 if phase else 500,0,phase,1,1,3787 if boost else 300,925 if phase else -925]
    first=True
    for index in range(334):
        for phase in [0,1]:
            for locked,valid in [(0,0),(0,1),(1,0),(1,1)]:
                for adc in [1000,3096]:
                    yield 'positive_negative_phase', first, [adc,index,phase,locked,valid,table[index],925 if phase else -925]
                    first=False
    for phase in [0,1]:
        for signed in range(-21,22):
            for ref in [573,574,575,3787]:
                yield 'reference_zero_band_boundary', True, [2048,0,phase,1,1,ref,signed]
    for phase in [0,1]:
        for error in range(-4,5):
            target_filter = 2048-error if phase else 2048+error
            yield 'error_deadband_boundary', True, [2048+4*(target_filter-2048),0,phase,1,1,574,0]


def main():
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S_%fZ')
    out=ROOT/'step5b_results'/stamp
    out.mkdir(parents=True,exist_ok=False)
    prior=json.loads((ROOT/'step4c_results/20261001T063216_353015Z/metrics.json').read_text())
    assert prior['status']=='PASS' and len(FIELDS)==18
    # Reuse is permitted only for the exact Step 4C source/model contract.
    for rel,h in prior['source_sha256'].items():
        assert core.sha(core.REPO/rel)==h, f'Validated source changed: {rel}'
    preserved={str(core.REPO/rel):h for rel,h in prior['source_sha256'].items()}
    for name in ['STEP4B_CADENCE_RESTORE.md','STEP4C_ADC_INPUT_PARITY.md','STEP5A_FRESH_RESULTS.md']:
        preserved[str(ROOT/name)]=core.sha(ROOT/name)
    adc,pwm,parameter=[(core.FW/name).read_text(encoding='utf-8-sig') for name in ['adc1.c','pwm.c','parameter.h']]
    assert 'const int16_t Vdc = 574;' in pwm
    table=list(map(int,re.findall(r'-?\d+',re.search(r'VrefTable\[334\] = \{(.*?)\};',pwm,re.S).group(1))))
    oracle,command=core.oracle_build(adc,pwm,parameter,out)
    model=core.Model(table)
    counts=dict.fromkeys(FIELDS,0); maxima=dict.fromkeys(FIELDS,0)
    groups={}; coverage={}; first=None; expected_rows=[]; input_rows=[]
    group_errors={}
    def hit(name,condition):
        coverage[name]=coverage.get(name,0)+int(bool(condition))
    with (out/'inputs.csv').open('w',newline='') as fi, (out/'python.csv').open('w',newline='') as fp, (out/'c_oracle.csv').open('w',newline='') as fc:
        wi,wp,wc=csv.writer(fi),csv.writer(fp),csv.writer(fc)
        wi.writerow(['group','reset',*core.INPUTS]); wp.writerow(FIELDS+['Dboost_fw']); wc.writerow(FIELDS+['Dboost_fw'])
        for n,(group,reset,sample) in enumerate(vectors(table)):
            if reset: model=core.Model(table); oracle.reset()
            pre_filter,pre_mode=model.filt,model.last
            buffer=(ctypes.c_int64*len(FIELDS))()
            assert oracle.step(*sample,buffer)==0
            py=model.step(*sample); c=list(buffer); row=dict(zip(FIELDS,c))
            groups[group]=groups.get(group,0)+1
            group_errors.setdefault(group,0)
            for field,a,b in zip(FIELDS,py,c):
                diff=abs(a-b); counts[field]+=int(diff!=0); maxima[field]=max(maxima[field],diff)
                group_errors[group]+=int(diff!=0)
                if diff and first is None: first=dict(sample=n,group=group,field=field,python=a,c=b)
            wi.writerow([group,int(reset),*sample])
            wp.writerow(py+[format(Decimal(py[FIELDS.index('finalDuty')])/Decimal(12500),'.8f')])
            wc.writerow(c+[format(Decimal(row['finalDuty'])/Decimal(12500),'.8f')])
            expected_rows.append(py); input_rows.append((group,int(reset),sample))
            assert 0<=row['finalDuty']<=10625
            assert row['effective_correction']==row['finalDuty']-row['Boost_PWM']==row['y1']
            hit('correction_upper_clipped',row['raw']>2500)
            hit('correction_lower_clipped',row['raw'] < -2500)
            hit('finalDuty_upper_clipped',row['mode'] and row['Boost_PWM']+row['limited']>10625)
            hit('finalDuty_lower_clipped',row['mode'] and row['Boost_PWM']+row['limited']<0)
            hit('finalDuty_at_10625',row['finalDuty']==10625)
            hit('finalDuty_at_zero',row['finalDuty']==0)
            hit('y1_differs_from_limited',row['mode'] and row['y1']!=row['limited'])
            hit('boost_to_buck',pre_mode==1 and row['mode']==0)
            hit('buck_to_boost',pre_mode==0 and row['mode']==1)
            if row['mode']==0:
                assert all(row[k]==0 for k in ['x1','x2','y1','y2','acc','raw','limited','finalDuty','Boost_PWM'])
                assert row['finalDuty_buck']==row['Buck_PWM']
                hit('buck_clear_samples',True)
                hit('buck_filter_retained',pre_filter!=2048)
            if pre_mode==0 and row['mode']==1:
                assert row['x2']==row['y2']==0
            assert row['adc0_filt']==pre_filter+((sample[0]-pre_filter)>>2)
            hit('phase_0',sample[2]==0); hit('phase_1',sample[2]==1)
            hit('pll_input_path',sample[3] and sample[4]); hit('lut_input_path',not(sample[3] and sample[4]))
            if group=='reference_zero_band_boundary':
                signed=sample[-1]
                assert row['iref_cmd']==2048+(0 if -20<signed<20 else signed)
                hit('reference_boundary_'+str(signed),True)
            if group=='error_deadband_boundary':
                rawerr=row['err_raw']
                assert row['err']==(0 if -3<rawerr<3 else rawerr)
                hit('error_boundary_'+str(rawerr),True)
    assert len(groups)==7 and all(coverage.values()), coverage
    # Round-trip every integer field without using the normalized float as storage.
    roundtrip=0
    for filename in ['python.csv','c_oracle.csv']:
        with (out/filename).open(newline='') as f:
            saved=list(csv.DictReader(f))
        assert len(saved)==len(expected_rows)
        for parsed,original in zip(saved,expected_rows):
            for name,value in zip(FIELDS,original):
                roundtrip+=int(int(parsed[name])!=value)
            duty=Decimal(parsed['Dboost_fw'])
            roundtrip+=int(duty*12500!=int(parsed['finalDuty']))
            assert Decimal(0)<=duty<=Decimal('.85')
    replay_mismatch=0
    with (out/'inputs.csv').open(newline='') as f:
        saved_inputs=list(csv.DictReader(f))
    assert len(saved_inputs)==len(expected_rows)
    for parsed,(group,reset,sample),expected in zip(saved_inputs,input_rows,expected_rows):
        values=[int(parsed[k]) for k in core.INPUTS]
        assert parsed['group']==group and int(parsed['reset'])==reset and values==sample
        if reset: oracle.reset()
        buffer=(ctypes.c_int64*len(FIELDS))()
        assert oracle.step(*values,buffer)==0
        replay_mismatch+=sum(a!=b for a,b in zip(buffer,expected))
    assert all(core.sha(Path(p))==h for p,h in preserved.items())
    finals=[r[FIELDS.index('finalDuty')] for r in expected_rows]
    assert min(finals)==0 and max(finals)==10625
    passed=not any(counts.values()) and roundtrip==replay_mismatch==0
    result=dict(status='PASS' if passed else 'FAIL', domain_status='DIGITAL DOMAIN VALIDATED' if passed else 'NOT VALIDATED',
        physical_voltage_calibration='NOT FORMALLY RE-VALIDATED',utc=stamp,
        group_count=len(groups),groups=groups,samples=len(finals),formal_field_count=len(FIELDS),
        mismatch_counts=counts,max_absolute_difference=maxima,group_mismatches=group_errors,first_mismatch=first,
        coverage=coverage,csv_roundtrip_mismatches=roundtrip,csv_input_c_replay_mismatches=replay_mismatch,
        finalDuty_range=[min(finals),max(finals)],Dboost_fw_range=[0,.85],source_sha256=preserved,
        runner_sha256=core.sha(Path(__file__)),compile_command=command,
        compiler=subprocess.check_output([str(core.GCC),'--version'],text=True).splitlines()[0],
        existing_source_files_unchanged=True)
    result['evidence_sha256']={p.name:core.sha(p) for p in out.iterdir() if p.is_file()}
    (out/'metrics.json').write_text(json.dumps(result,indent=2)+'\n')
    text=f"# Step 5B firmware integer command\n\n**{result['status']} — {result['domain_status']}**\n\nPHYSICAL VOLTAGE CALIBRATION: **NOT FORMALLY RE-VALIDATED**\n\n"
    text+=f"Fresh run `{stamp}`: **{len(groups)} groups, {len(finals):,} samples, 18 formal fields**. No historical vectors were available; these are new deterministic reconstruction vectors, not a claim to reproduce historical 55,100 samples.\n\n"
    text+='| Group | Samples | Field mismatches |\n|---|---:|---:|\n'+''.join(f'| {g} | {c} | {group_errors[g]} |\n' for g,c in groups.items())
    text+='\n## Implementation reuse\n\nDirectly imports the unchanged Step 4C `Model`, `oracle_build`, `FIELDS`, and `INPUTS`; does not invoke its main() or rerun Step 4C. SHA256 guards require the original validated Step 4C source hashes. The independently compiled C oracle retains extracted actual AN0 callback, LUT, reference selection, state clear, and Buck/Boost feedforward functions. Python and C do not share arithmetic implementation. Only the vector generator and comparison/CSV harness are new.\n\n'
    text+='Boost_PWM uses C integer division of `(vref_cmd-574)*12500/(vref_cmd+574)` in the Boost region. Because both operands are nonnegative there, Python floor division agrees with C truncation toward zero. Buck bypasses Boost division and sets Boost_PWM=0. No negative-division approximation is used. A=[30010,2668], B=[26000,-32000,9640], signed int64 accumulator, arithmetic >>15, correction [-2500,+2500], finalDuty [0,10625], and y1=finalDuty-Boost_PWM are unchanged. The C host width and negative right-shift checks run in the reused fixture. No new counts/V calibration or VrefTable changes.\n\n'
    text+='## Parity\n\n| Field | Mismatches | Max difference |\n|---|---:|---:|\n'+''.join(f'| {k} | {counts[k]} | {maxima[k]} |\n' for k in FIELDS)
    text+='\n## Coverage\n\n'+''.join(f'- {k}: {v}\n' for k,v in coverage.items() if not k.startswith(('reference_boundary_','error_boundary_')))
    text+='\nClamp statistics count samples with strict pre-clamp violations, not distinct time episodes. Exact-limit counts are reported separately. Buck clears x1/x2/y1/y2 every callback, retains ADC filter history, and outputs Buck feedforward only. Boost re-entry has x2=y2=0. Both phases, LUT and injected PLL-reference paths, reference -21..21, and error -4..4 are explicitly checked. PLL dynamics and interrupt race timing are outside this digital fixture.\n\n'
    text+=f'## Duty normalization / CSV\n\nDboost_fw=finalDuty/12500.0, observed range [0,0.85]; maximum finalDuty=10625. All 18 integer fields are serialized as integer decimal strings. Duty is serialized with eight decimal places and read using Decimal; duty*12500 recovers the exact finalDuty. Roundtrip mismatches={roundtrip}; CSV input replay through C mismatches={replay_mismatch}.\n\n'
    text+='Digital arithmetic evidence does not establish physical voltage calibration, target MCU execution timing, PLL phase delivery, runtime Verilog behavior, analog duty output, or power-stage stability. The inherited 574/LUT domain remains in use.\n\n'
    text+=f'Evidence: [metrics.json](step5b_results/{stamp}/metrics.json), inputs.csv, python.csv, c_oracle.csv, firmware_input_oracle.c, build.log in that run directory. DLL is generated, not formal source. Reproduce with Python 3: `python phase2a_step5b.py`; every invocation uses a unique directory.\n\n## SHA256\n\n'+''.join(f'- `{p}`: `{h}`\n' for p,h in preserved.items())
    (out/'STEP5B_INTEGER_COMMAND.md').write_text(text,encoding='utf-8')
    target=ROOT/'STEP5B_INTEGER_COMMAND.md'
    if not target.exists():
        with target.open('x',encoding='utf-8') as f: f.write(text)
    print(json.dumps({k:result[k] for k in ['status','group_count','samples','mismatch_counts','max_absolute_difference','coverage','csv_roundtrip_mismatches','csv_input_c_replay_mismatches','Dboost_fw_range']},indent=2))
    print(out)
    if not passed: raise SystemExit(1)


if __name__=='__main__': main()
