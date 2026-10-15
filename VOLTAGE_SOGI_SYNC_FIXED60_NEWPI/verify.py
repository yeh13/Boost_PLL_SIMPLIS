from pathlib import Path
import sys,json,hashlib,ctypes,subprocess,random,re
R=Path(__file__).resolve().parent;C=R/'VOLTAGE_SOGI_SYNC_FIXED60_NEWPI.X'
S=R.parent/'VOLTAGE_SOGI_SYNC_TEST/VOLTAGE_SOGI_SYNC_TEST.X'
A=R.parent/'PLL_PHASE_CONVENTION_AUDIT'
sys.path.insert(0,str(A));import pll_phase_convention_audit as audit
np=audit.np
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
before=json.loads((R/'source_manifest.json').read_text())
assert {f.relative_to(S).as_posix():sha(f) for f in S.rglob('*') if f.is_file()}==before
changed=json.loads((R/'changed_files.json').read_text())
assert [x for x in changed if not x.startswith('nbproject/')]==['mcc_generated_files/voltage_sogi_sync.c']
for f in C.rglob('*'):
    if f.is_file() and f.suffix in ['.c','.h','.mc3'] and f.relative_to(C).as_posix()!='mcc_generated_files/voltage_sogi_sync.c':
        assert sha(f)==before[f.relative_to(C).as_posix()],str(f)
mod=(C/'mcc_generated_files/voltage_sogi_sync.c').read_text();old=(S/'mcc_generated_files/voltage_sogi_sync.c').read_text()
def function(s,name):
    if name!='VoltageSogiSync_Read':return audit.function(s,name)
    a=s.index('SogiSyncSnapshot VoltageSogiSync_Read(');b=s.index('\n}',a)+2
    return s[a:b]
for name in ['SyncPublish','VoltageSogiSync_Read','PLL_NormalizeExact','PLL_StepToFreqX10']:
    assert function(mod,name)==function(old,name),name
assert 'SOGI_LoadCoeffFromStepQ' not in mod
assert 'SOGI_Interp' not in mod
assert mod.count('sogi_coeff_lut[SOGI_FIXED60_LUT_INDEX]')==5
assert '#define SOGI_FIXED60_LUT_INDEX 50U' in mod
assert 'pll_step_q += pll_step_diff_q;' in mod
allc='\n'.join(p.read_text() for p in (C/'mcc_generated_files').glob('*.c'))
assert len(re.findall(r'\bi\s*\+\+',allc))==1
adc=(C/'mcc_generated_files/adc1.c').read_text();pwm=(C/'mcc_generated_files/pwm.c').read_text()
assert 'i++;' in audit.function(adc,'ADC1_channel_AN0_CallBack')
assert 'ADTRIG0L = 0x0604;' in adc
for token in ['PG1PER = 0x30D3;','PG2PER = 0x30D3;','PG2EVTLbits.ADTR1PS = 7;','PG2EVTLbits.ADTR1EN1 = 1;']:assert token in pwm
T=R/'parity';T.mkdir(exist_ok=True)
E=A/'ab_20261015T073801_698139Z/FIXED_SOGI_NEW_PI'
shutil=__import__('shutil')
shutil.copy2(E/'stdint.h',T/'stdint.h')
fields=['grid_offset_q','grid_offset','sogi_va','sogi_vb','sogi_va_n1','sogi_va_n2','sogi_vb_n1','sogi_vb_n2','sogi_vin_n1','sogi_vin_n2','pll_idx','theta_acc_q','pll_step_q','pll_int_acc','pll_err_lpf','pll_err_slow_lpf_q8','pll_err_slow_lpf','pll_lock_count','pll_unlock_count','pll_phase_ok_count','pll_phase_bad_count','pll_locked','dbg_pll_phase_ok','dbg_an1_input_valid']
def runner(call):return '\nvoid parity_step(uint16_t adc,int64_t *out){'+call+'(adc);'+''.join(f'out[{i}]={x};' for i,x in enumerate(fields))+'}\n'
def compile(name,code,opt):
    path=T/(name+'.c');path.write_text(code);dll=T/(name+'.dll')
    cmd=['C:/Program Files/SIMetrix830/va/bin/gcc.exe','-std=c11','-'+opt,'-shared','-nostdlib','-Wl,--entry,0','-I'+str(T),'-I'+str(C/'mcc_generated_files'),str(path),'-o',str(dll)]
    p=subprocess.run(cmd,capture_output=True,text=True);(T/(name+'.log')).write_text(p.stdout+p.stderr);assert p.returncode==0,p.stderr
    lib=ctypes.CDLL(str(dll));lib.parity_step.argtypes=[ctypes.c_ushort,ctypes.POINTER(ctypes.c_longlong)];return lib
oracle=compile('offline_E',(E/'core.c').read_text()+runner('ADC1_channel_AN1_CallBack'),'O2')
piwrap='\nint32_t test_pi(int32_t e,int32_t s){return PLL_NewPIToStep(e,s);}\n'
libs=[oracle]+[compile('candidate_'+o,mod+runner('VoltageSogiSync_Update')+piwrap,o) for o in ['O0','O2']]
bufs=[(ctypes.c_longlong*len(fields))() for _ in libs]
t=np.arange(150000)/5000;adcvals=np.floor(2048+1860*np.sin(2*np.pi*60*t)+.5).astype(np.uint16)
digest=hashlib.sha256()
for k,x in enumerate(adcvals):
    for lib,b in zip(libs,bufs):lib.parity_step(int(x),b)
    assert list(bufs[0])==list(bufs[1])==list(bufs[2]),(k,[list(b) for b in bufs])
    digest.update(bytes(bufs[2]))
# Independent Python integer oracle for Q24 PI conversion, including clamp limits.
limit=(30*360*512*4096)//5000
rng=random.Random(6000300)
cases=[(e,s) for e in [-1000,-1,0,1,1000] for s in [-limit,-1,0,1,limit]]
cases += [(rng.randint(-1000,1000),rng.randint(-limit,limit)) for _ in range(100000)]
for lib in libs[1:]:
    lib.test_pi.argtypes=[ctypes.c_int32,ctypes.c_int32];lib.test_pi.restype=ctypes.c_int32
    for e,s in cases:
        value=e*12095496153+s*24190992
        expected=(abs(value)>>24)*(1 if value>=0 else -1)
        assert lib.test_pi(e,s)==expected,(e,s)
dis=(R/'pll_object.disasm.txt').read_text()
assert len(re.findall(r'PC RELATIVE BRANCH\s+___divdi3\b',dis))==1
assert '___udivdi3' not in dis
result=dict(status='PASS',source_unchanged=True,frozen_sources_byte_identical=True,changed_files=changed,
    fixed_sogi_lut_index=50,closed_loop_nco=True,single_original_i_increment=True,
    configured_rates_hz=dict(AN0=40000,AN1=5000,PG1=40000,PG2=40000,measured=False),
    parity=dict(seconds=30,samples=150000,fields=fields,comparisons_per_pair=150000*len(fields),
        mismatches=0,candidate_O0_O2_and_offline_E=True,trace_sha256=digest.hexdigest(),
        independent_Python_PI_cases=len(cases),scope='Host C extraction/full-state parity + Python integer PI oracle; NOT target runtime bit-exact proof'),
    codegen=dict(total_divdi3_relocations=1,new_PI_divdi3=0,retained_normalize_fallback=True),
    pi_max_intermediate_bound=1000*12095496153+limit*24190992)
(R/'verification.json').write_text(json.dumps(result,indent=2));print(json.dumps(result,indent=2))
