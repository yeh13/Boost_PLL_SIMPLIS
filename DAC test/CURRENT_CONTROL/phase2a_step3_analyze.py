import json
import math
from array import array
from pathlib import Path


ROOT = Path(__file__).resolve().parent
NAMES = ['sense_delta_V', 'error_counts', 'error_A', 'Iout_A', 'Iref_A']
START = 4 / 60
STOP = 7 / 60
GAIN = 0.46 * 4096 / 3.3


def read_export():
    traces = []
    times = array('d')
    values = array('d')
    previous_time = -1.0
    with (ROOT / 'phase2a_step3_waveforms.txt').open() as source:
        for line in source:
            fields = line.split()
            if len(fields) == 1:
                break
            time, value = map(float, fields)
            if time < previous_time:
                traces.append((times, values))
                times, values = array('d'), array('d')
            times.append(time)
            values.append(value)
            previous_time = time
    traces.append((times, values))
    assert len(traces) == len(NAMES)
    assert all(trace[0] == traces[0][0] for trace in traces)
    return traces[0][0], dict(zip(NAMES, [trace[1] for trace in traces]))


def metrics(times, values):
    integral = 0.0
    minimum, maximum = math.inf, -math.inf
    for index in range(1, len(times)):
        begin, end = times[index - 1], times[index]
        left, right = max(begin, START), min(end, STOP)
        if right <= left:
            continue
        slope = (values[index] - values[index - 1]) / (end - begin)
        first = values[index - 1] + slope * (left - begin)
        last = values[index - 1] + slope * (right - begin)
        integral += (right - left) * (first * first + first * last + last * last) / 3
        minimum, maximum = min(minimum, first, last), max(maximum, first, last)
    return {'rms': math.sqrt(integral / (STOP - START)),
            'peak_abs': max(abs(minimum), abs(maximum)),
            'minimum': minimum, 'maximum': maximum}


def main():
    times, signals = read_export()
    selected = [index for index, time in enumerate(times) if START <= time <= STOP]
    reference, output = signals['Iref_A'], signals['Iout_A']
    error, counts = signals['error_A'], signals['error_counts']
    opposite = [index for index in selected if reference[index] * output[index] < 0]
    positive_wrong = [index for index in selected if reference[index] > 0 and output[index] <= 0]
    bad_error = [index for index in selected
                 if output[index] < reference[index] - 1e-9 and counts[index] <= 0]
    result = {
        'window_seconds': [START, STOP],
        'cycles': 3,
        'samples_per_vector': len(times),
        'CURRENT_COUNTS_PER_AMP': GAIN,
        'metrics': {name: metrics(times, values) for name, values in signals.items()},
        'max_error_identity_residual_A': max(abs(error[index] - reference[index] + output[index]) for index in selected),
        'max_scaling_identity_residual_counts': max(abs(counts[index] - error[index] * GAIN) for index in selected),
        'positive_reference_nonpositive_output_samples': len(positive_wrong),
        'opposite_sign_samples': len(opposite),
        'opposite_sign_max_abs_reference_A': max((abs(reference[index]) for index in opposite), default=0),
        'opposite_sign_max_abs_output_A': max((abs(output[index]) for index in opposite), default=0),
        'opposite_sign_time_bounds_s': [times[opposite[0]], times[opposite[-1]]] if opposite else [],
        'incorrect_error_sign_samples_tolerance_1nA': len(bad_error),
        'polarity_samples_evaluated': len(selected),
        'opposite_sign_samples_outside_30mA_reference_band': sum(abs(reference[index]) > 0.03 for index in opposite),
        'positive_peak_sample': {name: values[max(selected, key=lambda index: reference[index])] for name, values in signals.items()},
        'negative_peak_sample': {name: values[min(selected, key=lambda index: reference[index])] for name, values in signals.items()},
    }
    (ROOT / 'phase2a_step3_metrics.json').write_text(json.dumps(result, indent=2) + '\n')
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()
