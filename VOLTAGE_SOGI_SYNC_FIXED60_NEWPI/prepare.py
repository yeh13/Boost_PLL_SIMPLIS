from pathlib import Path
import shutil,json,hashlib,difflib
R=Path(__file__).resolve().parent
S=R.parent/'VOLTAGE_SOGI_SYNC_TEST/VOLTAGE_SOGI_SYNC_TEST.X'
C=R/'VOLTAGE_SOGI_SYNC_FIXED60_NEWPI.X'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def manifest(p):return {f.relative_to(p).as_posix():sha(f) for f in p.rglob('*') if f.is_file()}
assert not C.exists(),'Never overwrite existing candidate'
before=manifest(S);(R/'source_manifest.json').write_text(json.dumps(before,indent=2))
shutil.copytree(S,C,ignore=shutil.ignore_patterns('build*','dist*','.generated_files','private'))
p=C/'mcc_generated_files/voltage_sogi_sync.c';s=p.read_text()
s=s.replace('See extraction.diff and audit.json. Numeric order/coefficients unchanged.','Fixed60/new-PI isolated candidate; see parent REPORT.md and static.diff.')
a=s.index('static void SOGI_LoadCoeffFromStepQ');b=s.index('\n}',a)+2
s=s[:a]+'''/* Fixed existing 60.0 Hz row: (600 - 550) = 50. No frequency mapping. */
#define SOGI_FIXED60_LUT_INDEX 50U
static void SOGI_LoadCoeffFixed60(void)
{
    dbg_sogi_freq_x10 = 600;
    sogi_a1_q30 = sogi_coeff_lut[SOGI_FIXED60_LUT_INDEX][0];
    sogi_a2_q30 = sogi_coeff_lut[SOGI_FIXED60_LUT_INDEX][1];
    sogi_b0_q30 = sogi_coeff_lut[SOGI_FIXED60_LUT_INDEX][2];
    sogi_qb0_q30 = sogi_coeff_lut[SOGI_FIXED60_LUT_INDEX][3];
    sogi_qb1_q30 = sogi_coeff_lut[SOGI_FIXED60_LUT_INDEX][4];
    sogi_qb2_q30 = sogi_qb0_q30;
}

/* e=error/1000; S=sum(error); Ts=1/5000.
 * delta_step=(30*e + 300*Ts*S/1000)*(180/pi)*2^21/5000.
 * Q24 gains: round(720.9477515845177*2^24),
 *            round(1.4418955031690353*2^24).
 * Keep original S accumulator/clamp. Truncate combined result toward zero,
 * exactly matching offline E; explicit shifts avoid runtime 64-bit division.
 */
#define PLL_NEW_KP_STEP_Q24 12095496153LL
#define PLL_NEW_KI_STEP_Q24 24190992LL
static int32_t PLL_NewPIToStep(int32_t error, int32_t sum)
{
    int64_t value = (int64_t)error * PLL_NEW_KP_STEP_Q24 +
                    (int64_t)sum * PLL_NEW_KI_STEP_Q24;
    if (value >= 0)
        return (int32_t)(value >> 24);
    return -(int32_t)((-value) >> 24);
}
'''+s[b:]
assert s.count('SOGI_LoadCoeffFromStepQ(pll_step_q);')==1
s=s.replace('SOGI_LoadCoeffFromStepQ(pll_step_q);','SOGI_LoadCoeffFixed60();')
a=s.index('    if ((pll_locked == 0) && (abs_err_lpf > PLL_LOCK_ERR_TH))');b=s.index('    if (w_delta_q > PLL_TRIM_LIMIT_Q)',a)
s=s[:a]+'    w_delta_q = PLL_NewPIToStep(pll_err_pi, pll_int_acc);\n\n'+s[b:]
p.write_text(s)
for p in (C/'nbproject').glob('*'):
    if not p.is_file():continue
    raw=p.read_bytes();new=raw.replace(b'VOLTAGE_SOGI_SYNC_TEST',b'VOLTAGE_SOGI_SYNC_FIXED60_NEWPI')
    if p.name=='project.xml':new=new.replace(b'<name>closed_loop_inverter</name>',b'<name>VOLTAGE_SOGI_SYNC_FIXED60_NEWPI</name>')
    if p.name=='Package-default.bash':new=new.replace(b'closed_loop_inverter.X',b'VOLTAGE_SOGI_SYNC_FIXED60_NEWPI.X')
    if new!=raw:p.write_bytes(new)
build=(R.parent/'VOLTAGE_SOGI_SYNC_TEST/build.py').read_text().replace('VOLTAGE_SOGI_SYNC_TEST.X','VOLTAGE_SOGI_SYNC_FIXED60_NEWPI.X')
(R/'build.py').write_text(build)
diff=[];changed=[]
for p in C.rglob('*'):
    if not p.is_file():continue
    rel=p.relative_to(C).as_posix()
    if sha(p)!=before[rel]:
        changed.append(rel)
        diff.extend(difflib.unified_diff((S/rel).read_text().splitlines(True),p.read_text().splitlines(True),fromfile='baseline/'+rel,tofile='candidate/'+rel))
(R/'static.diff').write_text(''.join(diff))
assert manifest(S)==before
(R/'changed_files.json').write_text(json.dumps(changed,indent=2))
print(json.dumps(changed,indent=2))
