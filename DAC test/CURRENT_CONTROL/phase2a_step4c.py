"""Fresh firmware-input parity. Never edits firmware or reuses a run directory."""
import csv
import ctypes
import hashlib
import json
import math
import random
import re
import subprocess
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parent
REPO = ROOT.parents[1]
FW = REPO / 'Boost_I_loop_inverter.X/mcc_generated_files'
GCC = Path('C:/Program Files/SIMetrix830/va/bin/gcc.exe')
FIELDS = 'adc0_filt iref_cmd err_raw err mode Boost_PWM Buck_PWM acc raw limited finalDuty effective_correction x1 x2 y1 y2 finalDuty_buck mode_switches'.split()
INPUTS = 'adc index phase locked phase_ok pll_vref pll_iref'.split()


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def function(text, signature):
    start = text.index(signature)
    brace = text.index('{', start)
    depth, end = 1, brace + 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[start:end]


def define(text, name):
    lines = text.splitlines()
    for n, line in enumerate(lines):
        if re.match(r'^#define\s+' + name + r'\b', line):
            result = [line]
            while result[-1].endswith('\\'):
                n += 1
                result.append(lines[n])
            return '\n'.join(result)
    raise ValueError(name)


def replace_once(text, old, new):
    assert text.count(old) == 1, old
    return text.replace(old, new)


def oracle_build(adc, pwm, parameter, output):
    names = 'ADC_CENTER VREF_PEAK DUTY_BUCK_MAX DUTY_BOOST_MAX ERR_DEADBAND ADC_FILT_SHIFT IREF_PEAK_COUNTS IREF_ZERO_BAND IREF_FROM_VREF_Q IREF_FROM_VREF_GAIN_Q BOOST_TEMP_LIMIT MODE_UNKNOWN ADC_INTERNAL_DUAL_TEST PLL_TEST_FORCE_PLL_OUTPUT PLL_OUTPUT_USE_PLL'.split()
    constants = '\n'.join(define(adc, n) for n in names)
    constants += '\n' + '\n'.join(define(parameter, n) for n in ['PLL_CONTROL_ENABLE', 'GRID_TIE_REQUIRE_PLL_LOCK'])
    arrays = '\n'.join(re.findall(r'const int32_t [AB]_Coefficient_BOOST\[\d\] = \{.*?\};', adc, re.S))
    table = re.search(r'const int16_t VrefTable\[334\] = \{.*?\};', pwm, re.S).group()
    callback = function(adc, 'void __attribute__((weak)) ADC1_channel_AN0_CallBack')
    callback = callback.replace('__attribute__((weak)) ', '')
    callback = replace_once(callback, '    if ((err_now < ERR_DEADBAND)', '    observed_err_raw = err_now;\n    if ((err_now < ERR_DEADBAND)')
    callback = replace_once(callback, '        temp >>= 15;', '        observed_acc = temp;\n        temp >>= 15;\n        observed_raw = temp;')
    code = '''typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef signed long long int64_t;
typedef unsigned char uint8_t;
_Static_assert(sizeof(int16_t)==2 && sizeof(int32_t)==4 && sizeof(int64_t)==8, "widths");
#define MODE_BUCK 0
#define MODE_BOOST 1
''' + constants + '\n' + arrays + '\n' + table + '''
static const int16_t Vdc = 574;
static int32_t adc0_filt = ADC_CENTER;
static int16_t err_boost[3], err_buck[3];
static int32_t duty_boost[2], duty_buck[2];
static uint8_t last_current_mode = MODE_UNKNOWN;
static int32_t Boost_PWM, Buck_PWM;
static int16_t finallyDuty_boost, finallyDuty_buck;
static int32_t dbg_ierr, dbg_temp, dbg_mode_sw, dbg_mode;
static int16_t dbg_iref_cmd, dbg_adc0_raw, dbg_adc0_filt;
static int32_t PG1DC, PG2DC;
static uint16_t i, phase;
static uint8_t pll_locked, dbg_pll_phase_ok, pll_sync_phase;
static int16_t pll_vref_count, i_ref_count;
static int64_t observed_acc, observed_raw, observed_err_raw;
'''
    code += '\n'.join([function(adc, 'static void CURRENT_LOOP_StateClear(void)'),
                       function(pwm, 'static int32_t PWM_VrefGet(void)\n{'),
                       function(pwm, 'static uint8_t PWM_PhaseGet(void)\n{'),
                       function(pwm, 'void BOOST_DUTY(int32_t vref_cmd)\n{'),
                       function(pwm, 'void BUCK_DUTY(int32_t vref_cmd, uint8_t phase_cmd)\n{'), callback])
    code += '''
__declspec(dllexport) void reset(void) {
    CURRENT_LOOP_StateClear(); adc0_filt = ADC_CENTER;
    last_current_mode = MODE_UNKNOWN; dbg_mode_sw = 0;
    Boost_PWM = Buck_PWM = finallyDuty_boost = finallyDuty_buck = 0;
}
__declspec(dllexport) int step(int adc, int index, int half, int locked, int phase_ok,
                              int voltage_ref, int current_ref, int64_t *out) {
    int32_t ref; uint8_t ph;
    if (((int64_t)-1 >> 15) != -1 || ((int32_t)-1 >> 2) != -1) return 1;
    i = index; phase = half; pll_sync_phase = half;
    pll_locked = locked; dbg_pll_phase_ok = phase_ok;
    pll_vref_count = voltage_ref; i_ref_count = current_ref;
    ref = PWM_VrefGet(); ph = PWM_PhaseGet();
    if (ref >= Vdc) { BOOST_DUTY(ref); Buck_PWM = ph == 1 ? 12500 : 0; }
    else { BUCK_DUTY(ref, ph); Boost_PWM = 0; }
    observed_acc = observed_raw = observed_err_raw = 0;
    ADC1_channel_AN0_CallBack((uint16_t)adc);
    out[0]=adc0_filt; out[1]=dbg_iref_cmd; out[2]=observed_err_raw; out[3]=dbg_ierr;
    out[4]=dbg_mode; out[5]=Boost_PWM; out[6]=Buck_PWM; out[7]=observed_acc;
    out[8]=observed_raw; out[9]=dbg_temp; out[10]=finallyDuty_boost;
    out[11]=finallyDuty_boost-Boost_PWM; out[12]=err_boost[1]; out[13]=err_boost[2];
    out[14]=duty_boost[0]; out[15]=duty_boost[1]; out[16]=finallyDuty_buck;
    out[17]=dbg_mode_sw; return 0;
}
'''
    src, dll = output / 'firmware_input_oracle.c', output / 'firmware_input_oracle.dll'
    src.write_text(code, encoding='utf-8')
    command = [str(GCC), '-std=c11', '-Wall', '-Wextra', '-Werror', '-O2', '-shared', '-nostdlib', '-Wl,--entry,0', str(src), '-o', str(dll)]
    build = subprocess.run(command, capture_output=True, text=True)
    (output / 'build.log').write_text(build.stdout + build.stderr, encoding='utf-8')
    build.check_returncode()
    oracle = ctypes.CDLL(str(dll))
    oracle.step.argtypes = [ctypes.c_int] * 7 + [ctypes.POINTER(ctypes.c_int64)]
    oracle.step.restype = ctypes.c_int
    oracle.reset.argtypes = []
    oracle.reset.restype = None
    return oracle, command


class Model:
    def __init__(self, table):
        self.table = table
        self.filt, self.last, self.switches = 2048, None, 0
        self.states = [0, 0, 0, 0]

    def step(self, adc, index, phase, locked, phase_ok, pll_vref, pll_iref):
        use_pll = locked and phase_ok
        ref = pll_vref if use_pll else self.table[index]
        boost = int(ref >= 574)
        if boost:
            base = min(12500, max(0, (ref - 574) * 12500 // (ref + 574)))
            buck = 12500 if phase else 0
        else:
            base = 0
            delta = ref * 6250 // 574
            buck = max(0, min(12500, 6250 + (delta if phase else -delta)))
        if self.last is not None and self.last != boost:
            self.states = [0, 0, 0, 0]
            self.switches += 1
        self.last = boost
        if use_pll:
            signed = 0 if -20 < pll_iref < 20 else pll_iref
        else:
            gain = ((925 << 15) + 3787 // 2) // 3787
            amp = (ref * gain + (1 << 14)) >> 15
            amp = 0 if amp < 20 else amp
            signed = amp if phase else -amp
        iref = 2048 + signed
        self.filt += (adc - self.filt) // 4
        error_raw = (iref - self.filt) * (1 if phase else -1)
        error = 0 if -3 < error_raw < 3 else error_raw
        acc = raw = limited = final = 0
        if boost:
            x1, x2, y1, y2 = self.states
            acc = 26000 * error - 32000 * x1 + 9640 * x2 + 30010 * y1 + 2668 * y2
            raw = acc // 32768
            limited = min(2500, max(-2500, raw))
            final = min(10625, max(0, base + limited))
            self.states = [error, x1, final - base, y1]
        else:
            self.states = [0, 0, 0, 0]
        return [self.filt, iref, error_raw, error, boost, base, buck, acc, raw, limited,
                final, final-base, *self.states, buck, self.switches]


def vectors():
    # Every sample specifies source inputs; reset only at explicit independent cases.
    for phase in [0, 1]:
        for current in range(-21, 22):
            for voltage in [573, 574, 575, 3787]:
                yield 'reference_boundaries', True, [2048, 0, phase, 1, 1, voltage, current]
        for error in range(-4, 5):
            desired_filter = 2048 - error if phase else 2048 + error
            yield 'error_boundaries', True, [2048+4*(desired_filter-2048), 0, phase, 1, 1, 574, 0]
    for phase in [0, 1]:
        for index in [0, 167]:
            for adc in range(4096):
                yield 'adc_sweep', adc == 0, [adc, index, phase, 0, 0, 0, 0]
    for index in range(334):
        for phase in [0, 1]:
            for locked, phase_ok in [(0, 0), (0, 1), (1, 0), (1, 1)]:
                yield 'lut_and_reference_selection', index == phase == locked == phase_ok == 0, [2048, index, phase, locked, phase_ok, 3787, 925 if phase else -925]
    for tick in range(4800):
        theta = 2 * math.pi * 60 * tick / 40000
        pos = (tick * 60 * 668 // 40000) % 668
        yield '60hz_120ms', tick == 0, [round(2048 + 880*math.sin(theta)), pos % 334, int(pos < 334), 0, 0, 0, 0]
    for segment, (adc, voltage, current) in enumerate([(0, 3787, 925), (4095, 574, -925), (3000, 573, 0), (0, 3787, 925)]):
        for n in range(500):
            yield 'clamps_and_transitions', segment == n == 0, [adc, 0, 1, 1, 1, voltage, current]
    rng = random.Random(40008)
    for n in range(20000):
        yield 'random_inputs', n == 0, [rng.randrange(4096), rng.randrange(334), rng.randrange(2), rng.randrange(2), rng.randrange(2), rng.choice([0, 573, 574, 575, 3787, rng.randrange(3788)]), rng.randrange(-925, 926)]


def main():
    stamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S_%fZ')
    output = ROOT / 'step4c_results' / stamp
    output.mkdir(parents=True, exist_ok=False)
    sources = [FW / n for n in ['adc1.c', 'pwm.c', 'parameter.h', 'clock.c']]
    sources += [ROOT / 'Bidirection High Gain Inverter_Current_Control.sxsch', Path(__file__)]
    hashes = {str(p.relative_to(REPO)): sha(p) for p in sources}
    modified = subprocess.check_output(['git', 'diff', '--name-only', '-z'], cwd=REPO).decode().split('\0')
    preserved = {p: sha(REPO / p) for p in modified if p and (REPO / p).is_file()}
    (output / 'preservation_before.json').write_text(json.dumps(preserved, indent=2))
    adc, pwm, parameter, clock = [p.read_text(encoding='utf-8-sig') for p in sources[:4]]
    # Exact guards stop reconstruction against a different contract.
    for token in ['PG1EVTLbits.ADTR1PS = 0;', 'PG1PER = 0x30D3;', 'PG1EVTL = 0x108;', 'PG1EVTH = 0x00;', 'PG1TRIGA = 10625;']:
        assert token in pwm, token
    for name, value in [('ADC_ISR_HZ', '40000L'), ('PLL_DECIM_N', '8'), ('ADC_INTERNAL_DUAL_TEST', '0')]:
        assert define(adc, name).split()[-1] == value
    assert 'ADTRIG0L = 0x0404;' in adc and 'ADIEL = 0x03;' in adc
    assert 'const int16_t Vdc = 574;' in pwm
    assert define(parameter, 'PLL_CONTROL_ENABLE').split()[-1] == '1'
    assert define(parameter, 'GRID_TIE_REQUIRE_PLL_LOCK').split()[-1] == '0'
    clear = function(adc, 'static void CURRENT_LOOP_StateClear(void)')
    assert 'adc0_filt' not in clear
    for name, expected in [('A', [30010, 2668]), ('B', [26000, -32000, 9640])]:
        body = re.search(name + r'_Coefficient_BOOST\[\d\] = \{(.*?)\};', adc, re.S).group(1)
        assert list(map(int, re.findall(r'-?\d+', body))) == expected
    table = list(map(int, re.findall(r'-?\d+', re.search(r'VrefTable\[334\] = \{(.*?)\};', pwm, re.S).group(1))))
    assert len(table) == 334
    oracle, command = oracle_build(adc, pwm, parameter, output)
    model = Model(table)
    mismatch = dict.fromkeys(FIELDS, 0)
    maximum = dict.fromkeys(FIELDS, 0)
    groups, coverage, first = {}, {}, None
    def hit(name, condition):
        coverage[name] = coverage.get(name, 0) + int(condition)
    with (output / 'inputs.csv').open('w', newline='') as fi, (output / 'python.csv').open('w', newline='') as fp, (output / 'c_oracle.csv').open('w', newline='') as fc:
        wi, wp, wc = csv.writer(fi), csv.writer(fp), csv.writer(fc)
        wi.writerow(['group', 'reset', *INPUTS]); wp.writerow(FIELDS); wc.writerow(FIELDS)
        for n, (group, reset, sample) in enumerate(vectors()):
            if reset:
                model = Model(table); oracle.reset()
            pre_filter, pre_mode = model.filt, model.last
            buffer = (ctypes.c_int64 * len(FIELDS))()
            assert oracle.step(*sample, buffer) == 0
            actual, expected = model.step(*sample), list(buffer)
            wi.writerow([group, int(reset), *sample]); wp.writerow(actual); wc.writerow(expected)
            groups[group] = groups.get(group, 0) + 1
            for field, a, b in zip(FIELDS, actual, expected):
                diff = abs(a-b)
                mismatch[field] += int(diff != 0); maximum[field] = max(maximum[field], diff)
                if diff and first is None:
                    first = dict(sample=n, group=group, field=field, python=a, c=b)
            row = dict(zip(FIELDS, expected))
            hit('positive_correction_clamp', row['limited'] == 2500)
            hit('negative_correction_clamp', row['limited'] == -2500)
            hit('upper_final_clamp', row['Boost_PWM']+row['limited'] > 10625)
            hit('lower_final_clamp', row['mode'] and row['Boost_PWM']+row['limited'] < 0)
            hit('y1_not_limited', row['mode'] and row['y1'] != row['limited'])
            hit('boost_to_buck', pre_mode == 1 and row['mode'] == 0)
            hit('buck_to_boost', pre_mode == 0 and row['mode'] == 1)
            hit('buck_filter_retained', row['mode'] == 0 and pre_filter != 2048)
            if row['mode'] == 0:
                assert all(row[k] == 0 for k in ['x1', 'x2', 'y1', 'y2', 'limited', 'finalDuty'])
                assert row['finalDuty_buck'] == row['Buck_PWM']
            assert row['adc0_filt'] == pre_filter + ((sample[0]-pre_filter) >> 2)
            if group == 'reference_boundaries':
                signed = sample[-1]
                assert row['iref_cmd'] == 2048 + (0 if abs(signed) < 20 else signed)
            if group == 'error_boundaries':
                err = row['err_raw']
                assert row['err'] == (0 if abs(err) < 3 else err)
                hit('err_boundary_' + str(err), True)
    assert all(coverage.values()), coverage
    assert set(range(-4, 5)) <= {int(k.removeprefix('err_boundary_')) for k in coverage if k.startswith('err_boundary_')}
    assert all(sha(REPO / p) == h for p, h in hashes.items())
    assert all(sha(REPO / p) == h for p, h in preserved.items())
    result = dict(status='PASS' if not any(mismatch.values()) else 'FAIL', utc=stamp,
                  head=subprocess.check_output(['git', 'rev-parse', 'HEAD'], cwd=REPO, text=True).strip(),
                  samples=sum(groups.values()), groups=groups, mismatch_counts=mismatch,
                  max_absolute_difference=maximum, first_mismatch=first, coverage=coverage,
                  source_sha256=hashes, compile_command=command,
                  compiler=subprocess.check_output([str(GCC), '--version'], text=True).splitlines()[0],
                  source_and_existing_modified_files_unchanged=True)
    result['evidence_sha256'] = {p.name: sha(p) for p in output.iterdir() if p.is_file()}
    (output / 'metrics.json').write_text(json.dumps(result, indent=2) + '\n', encoding='utf-8')
    report = '# Step 4C firmware-input digital parity\n\n'
    report += f"Reconstructed fresh result: **{result['status']}**, {result['samples']:,} samples. UTC run: `{stamp}`.\n\n"
    report += 'Python integer model versus independently compiled source-extracted C oracle. The C oracle retains the full AN0 callback (including both reference branches), state clear, LUT, and Buck/Boost feedforward functions. Only host/register stubs and non-mutating trace instrumentation are added. PWM feedforward is updated before each AN0 call; this is a coherent input fixture, not an interrupt-race simulation. PLL state inputs are injected; PLL dynamics are not tested.\n\n'
    report += '| Group | Samples |\n|---|---:|\n' + ''.join(f'| {k} | {v} |\n' for k, v in groups.items())
    report += '\n| Field | Mismatches | Max absolute difference |\n|---|---:|---:|\n' + ''.join(f'| {k} | {mismatch[k]} | {maximum[k]} |\n' for k in FIELDS)
    report += '\nCoverage: `' + json.dumps(coverage) + '`.\n\n'
    report += 'Reference -19..19 -> 0, +/-20 retained; error -2..2 -> 0, +/-3 retained. All 4096 ADC codes, all 334 LUT entries, both half-cycle polarities, lock/phase-valid gating, mode transitions, both correction clamps and both final-duty clamps are exercised. Buck clears controller history without resetting the ADC filter. Effective correction, not limited correction, is fed back as y1.\n\n'
    report += 'Signed 64-bit C accumulator; host arithmetic right shifts are checked at runtime. This validates host-executed firmware input arithmetic, not an XC16 target build, ADC calibration, board timing, SOGI/PLL parity, or power-stage stability.\n\n'
    report += f'Evidence: [metrics.json](step4c_results/{stamp}/metrics.json), inputs.csv, python.csv, c_oracle.csv, generated firmware_input_oracle.c, build.log in the same fresh directory. DLL is a generated build product.\n\n'
    report += '## SHA256\n\n' + ''.join(f'- `{p}`: `{h}`\n' for p, h in hashes.items())
    report += '\nRe-run with Python 3: `python phase2a_step4c.py`. Every invocation creates a unique directory and report; the top-level report is created only if absent. No firmware or existing report is overwritten.\n'
    (output / 'STEP4C_ADC_INPUT_PARITY.md').write_text(report, encoding='utf-8')
    target = ROOT / 'STEP4C_ADC_INPUT_PARITY.md'
    if not target.exists():
        with target.open('x', encoding='utf-8') as stream:
            stream.write(report)
    print(json.dumps({'status': result['status'], 'samples': result['samples'], 'mismatches': sum(mismatch.values()), 'output': str(output)}, indent=2))
    if result['status'] != 'PASS':
        raise SystemExit(1)


if __name__ == '__main__':
    main()
