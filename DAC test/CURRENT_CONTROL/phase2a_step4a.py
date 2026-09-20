import argparse
import bisect
import csv
import ctypes
import hashlib
import json
import math
import random
import re
import subprocess
from pathlib import Path

from phase2a_step3_analyze import read_export


ROOT = Path(__file__).resolve().parent
REPO = ROOT.parents[1]
FIRMWARE = REPO / 'Boost_I_loop_inverter.X/mcc_generated_files'
GCC = Path('C:/Program Files/SIMetrix830/va/bin/gcc.exe')
FIELDS = ['e', 'Boost_PWM_counts', 'acc', 'raw', 'limited', 'virtual_finalDuty',
          'effective_correction', 'x1', 'x2', 'y1', 'y2',
          'correction_saturation_flag', 'finalDuty_high_clamp_flag',
          'finalDuty_low_clamp_flag']


def quantize(value, method):
    if method == 'truncate':
        return math.trunc(value)
    if method == 'floor':
        return math.floor(value)
    return int(math.copysign(math.floor(abs(value) + 0.5), value))


def interpolate(times, values, time):
    index = bisect.bisect_right(times, time) - 1
    if times[index] == time:
        return values[index]
    fraction = (time - times[index]) / (times[index + 1] - times[index])
    return values[index] + fraction * (values[index + 1] - values[index])


class Core:
    def __init__(self):
        self.state = [0, 0, 0, 0]
        self.last_mode = None
        self.pre_state = self.state[:]

    def step(self, error, boost, reference, dc):
        assert -32768 <= error <= 32767 and reference >= 0 and dc > 0
        assert abs((reference - dc) * 12500) < 2**31
        if boost != self.last_mode or not boost:
            self.state = [0, 0, 0, 0]
        self.pre_state = self.state[:]
        self.last_mode = boost
        error = 0 if -3 < error < 3 else error
        if not boost:
            return [error] + [0] * (len(FIELDS) - 1)
        numerator = (reference - dc) * 12500
        base = abs(numerator) // (reference + dc)
        base = max(0, min(12500, base if numerator >= 0 else -base))
        old_x1, old_x2, old_y1, old_y2 = self.state
        accumulator = (26000 * error - 32000 * old_x1 + 9640 * old_x2
                       + 30010 * old_y1 + 2668 * old_y2)
        assert -(2**63) <= accumulator < 2**63
        raw = accumulator >> 15
        limited = max(-2500, min(2500, raw))
        duty_sum = base + limited
        final = max(0, min(10625, duty_sum))
        effective = final - base
        self.state = [error, old_x1, effective, old_y1]
        return [error, base, accumulator, raw, limited, final, effective,
                *self.state, int(raw != limited), int(duty_sum > 10625), int(duty_sum < 0)]


def function_source(source, signature):
    match = re.search(re.escape(signature) + r'[^;{]*\{', source)
    assert match, signature
    start = match.start()
    brace = source.index('{', start)
    depth = 1
    end = brace + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]


def build_oracle(output):
    adc = (FIRMWARE / 'adc1.c').read_text(encoding='utf-8-sig')
    pwm = (FIRMWARE / 'pwm.c').read_text(encoding='utf-8-sig')
    callback = function_source(adc, 'void __attribute__((weak)) ADC1_channel_AN0_CallBack')
    transition = callback[callback.index('    if (last_current_mode == MODE_UNKNOWN)'):callback.index('    /*')]
    body_start = callback.index('    if ((err_now < ERR_DEADBAND)')
    body = callback[body_start:callback.index('    PG1DC =', body_start)]
    body = body.replace('        temp >>= 15;', '        observed_acc = temp;\n        temp >>= 15;\n        observed_raw = temp;')
    body = body.replace('        duty_cmd = Boost_PWM + (int32_t)temp;', '        observed_limited = temp;\n        duty_cmd = Boost_PWM + (int32_t)temp;\n        observed_sum = duty_cmd;')
    constants = '\n'.join(re.findall(r'^#define (?:ERR_DEADBAND|BOOST_TEMP_LIMIT|DUTY_BOOST_MAX|MODE_UNKNOWN)\s+[^\n]+', adc, re.M))
    arrays = '\n'.join(re.findall(r'const int32_t [AB]_Coefficient_BOOST\[\d\] = \{.*?\};', adc, re.S))
    clear = function_source(adc, 'static void CURRENT_LOOP_StateClear(void)')
    feedforward = function_source(pwm, 'void BOOST_DUTY(int32_t vref_cmd)')
    code = '''typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long long int64_t;
typedef unsigned char uint8_t;
_Static_assert(sizeof(int16_t)==2 && sizeof(int32_t)==4 && sizeof(int64_t)==8, "integer widths");
#define MODE_BUCK 0
#define MODE_BOOST 1
''' + constants + '\n' + arrays + '''
static int16_t err_boost[3], err_buck[3];
static int32_t duty_boost[2], duty_buck[2];
static uint8_t last_current_mode = MODE_UNKNOWN;
static int32_t Boost_PWM, Buck_PWM, Vdc;
static int16_t finallyDuty_boost, finallyDuty_buck;
static int32_t dbg_ierr, dbg_temp, dbg_mode_sw;
''' + clear + '\n' + feedforward + '''
__declspec(dllexport) void reset(void) {
    CURRENT_LOOP_StateClear();
    last_current_mode = MODE_UNKNOWN;
}
__declspec(dllexport) int step(int input_error, int input_mode, int reference, int dc, int64_t *out) {
    if (((int64_t)-1 >> 15) != -1) return 4;
    {
        int32_t err_now = input_error, duty_cmd = 0, duty_boost_prev = 0;
        uint8_t current_mode = input_mode;
        int64_t temp = 0, observed_acc = 0, observed_raw = 0, observed_limited = 0;
        int32_t observed_sum = 0;
        Vdc = dc;
        if (current_mode == MODE_BOOST) BOOST_DUTY(reference); else Boost_PWM = 0;
''' + transition + '\n' + body + '''
        out[0]=err_now; out[1]=Boost_PWM; out[2]=observed_acc;
        out[3]=observed_raw; out[4]=observed_limited; out[5]=finallyDuty_boost;
        out[6]=duty_boost[0]; out[7]=err_boost[1]; out[8]=err_boost[2];
        out[9]=duty_boost[0]; out[10]=duty_boost[1];
        out[11]=observed_raw != observed_limited;
        out[12]=observed_sum > DUTY_BOOST_MAX; out[13]=observed_sum < 0;
    }
    return 0;
}
'''
    source = output / 'firmware_core_oracle.c'
    source.write_text(code)
    executable = output / 'firmware_core_oracle.dll'
    subprocess.run([str(GCC), '-std=c11', '-Wall', '-Wextra', '-Werror', '-O2', '-shared', '-nostdlib', '-Wl,--entry,0', str(source), '-o', str(executable)], check=True)
    return executable


def compare(executable, inputs):
    oracle = ctypes.CDLL(str(executable))
    oracle.step.argtypes = [ctypes.c_int] * 4 + [ctypes.POINTER(ctypes.c_int64)]
    oracle.step.restype = ctypes.c_int
    oracle.reset()
    core = Core()
    outputs = []
    for index, sample in enumerate(inputs):
        buffer = (ctypes.c_int64 * len(FIELDS))()
        assert oracle.step(*sample, buffer) == 0
        expected = list(buffer)
        actual = core.step(*sample)
        assert actual == expected, (index, sample, dict(zip(FIELDS, actual)), expected)
        outputs.append(actual)
    return outputs


def summary(rows):
    result = {'samples': len(rows)}
    for name in ['TEST_error_counts_integer', 'e', 'raw', 'limited', 'effective_correction', 'x1', 'x2', 'y1', 'y2']:
        values = [row[name] for row in rows]
        result[name] = {'min': min(values), 'max': max(values),
                        'rms': math.sqrt(sum(value * value for value in values) / len(values))}
    for name in ['deadband_hit', 'correction_saturation_flag', 'finalDuty_high_clamp_flag', 'finalDuty_low_clamp_flag']:
        result[name + '_percent'] = 100 * sum(row[name] for row in rows) / len(rows)
    result['y1_differs_from_limited_samples'] = sum(row['y1'] != row['limited'] for row in rows)
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--quantizer', choices=['truncate', 'floor', 'nearest_away'], default='truncate')
    parser.add_argument('--sample-phase', type=float, default=0.85)
    parser.add_argument('--test-voltage-counts-per-volt', type=float, default=10)
    args = parser.parse_args()
    assert 0 <= args.sample_phase < 1 and args.test_voltage_counts_per_volt > 0
    output = ROOT / ('step4a_results_' + args.quantizer)
    output.mkdir(exist_ok=True)
    times, signals = read_export()
    mode_time, mode_value = [], []
    with (ROOT / 'phase2a_step4a_mode.txt').open() as source:
        for line in source:
            time, value = map(float, line.split())
            mode_time.append(time)
            mode_value.append(value)
    assert times[0] == 0 and times[-1] >= 0.12 and mode_time[-1] >= 0.12
    inputs, observations = [], []
    for tick in range(4800):
        time = (tick + args.sample_phase) / 40000
        unquantized = interpolate(times, signals['error_counts'], time)
        error = quantize(unquantized, args.quantizer)
        reference_volts = abs(interpolate(times, signals['Iref_A'], time) * 196)
        reference = math.trunc(reference_volts * args.test_voltage_counts_per_volt)
        dc = math.trunc(100 * args.test_voltage_counts_per_volt)
        boost = int(interpolate(mode_time, mode_value, time) > 2)
        inputs.append([error, boost, reference, dc])
        observations.append({'time': time, 'boost': boost,
                             'error_counts_unquantized': unquantized,
                             'TEST_error_counts_integer': error,
                             'TEST_vref_cmd': reference, 'TEST_Vdc_counts': dc,
                             'deadband_hit': int(-3 < error < 3)})
    executable = build_oracle(output)
    actual = compare(executable, inputs)
    rows = [dict(observation, **dict(zip(FIELDS, values))) for observation, values in zip(observations, actual)]
    core = Core()
    transitions, entries = 0, 0
    previous_mode = 0
    for sample in inputs:
        core.step(*sample)
        if not sample[1]:
            assert core.state == [0, 0, 0, 0]
        if sample[1] and not previous_mode:
            entries += 1
            assert core.pre_state == [0, 0, 0, 0]
        if not sample[1] and previous_mode:
            transitions += 1
        previous_mode = sample[1]
    directed = [[error, 1, 1001, 1000] for error in [-3, -2, -1, 0, 1, 2, 3]]
    directed += [[32767, 1, 100000, 1000]] * 100
    directed += [[-32768, 1, 1000, 1000]] * 100
    directed += [[0, 0, 0, 1000], [3, 1, 1001, 1000]]
    rng = random.Random(40_000)
    directed += [[rng.randint(-32768, 32767), int(rng.random() > 0.2), rng.randint(1000, 100000), 1000] for unused in range(10000)]
    stress = compare(executable, directed)
    coverage = {name: sum(row[FIELDS.index(name)] for row in stress) for name in FIELDS[-3:]}
    assert all(coverage.values())
    assert quantize(-2.9, 'truncate') == -2
    assert quantize(-2.1, 'floor') == -3
    assert quantize(-2.5, 'nearest_away') == -3
    assert quantize(2.5, 'nearest_away') == 3
    for error in [-3, -2, -1, 0, 1, 2, 3]:
        fresh = Core().step(error, 1, 1000, 1000)
        assert fresh[0] == (0 if abs(error) < 3 else error)
    window = [row for row in rows if 4 / 60 <= row['time'] < 7 / 60]
    assert len(window) == 2000
    result = {'method': 'offline observation replay; no live SIMPLIS controller instance; no ADC input parity',
              'configuration': vars(args), 'sample_rate_hz': 40000, 'duration_seconds': 0.12,
              'window_seconds': [4 / 60, 7 / 60], 'all_updates': summary(window),
              'boost_updates': summary([row for row in window if row['boost']]),
              'parity_mismatches': 0, 'replay_samples_compared': len(inputs),
              'stress_samples_compared': len(directed), 'stress_clamp_coverage': coverage,
              'boost_to_buck_clears_verified': transitions, 'zero_state_boost_entries_verified': entries,
              'all_buck_states_zero': True,
              'full_run': summary(rows),
              'hashes': {str(path.relative_to(REPO)): hashlib.sha256(path.read_bytes()).hexdigest()
                         for path in [FIRMWARE / 'adc1.c', FIRMWARE / 'pwm.c',
                                      ROOT / 'Bidirection High Gain Inverter_Current_Control.sxsch',
                                      ROOT / 'phase2a_step3_waveforms.txt', ROOT / 'phase2a_step4a_mode.txt']}}
    with (output / 'debug.csv').open('w', newline='') as stream:
        writer = csv.DictWriter(stream, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)
    (output / 'metrics.json').write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()
