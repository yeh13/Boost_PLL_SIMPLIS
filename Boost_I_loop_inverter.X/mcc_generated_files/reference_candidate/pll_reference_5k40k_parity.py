#!/usr/bin/env python3
"""Offline parity checker for the 5 kHz PLL anchor + 40 kHz phase delivery candidate.

Usage:
  python pll_reference_5k40k_parity.py --matlab golden.csv --c generated.csv

The checker expects the same input stream and column names used by the MATLAB gold trace:
  sample_40k,sample_5k,adc_raw,adc_centered,va,vb,vq,pi,freq_correction,phase_anchor,phase_step,phase_40k,phase_ok,locked,vref,iref

Any mismatch is reported with exact integer values, including the source of the Q-format rounding.
"""

from __future__ import annotations
import argparse
import csv
from pathlib import Path

REQUIRED_COLUMNS = [
    'sample_40k','sample_5k','adc_raw','adc_centered','va','vb','vq','pi','freq_correction',
    'phase_anchor','phase_step','phase_40k','phase_ok','locked','vref','iref'
]


def load_csv(path: str):
    with open(path, newline='') as f:
        rows = list(csv.DictReader(f))
    if not rows:
        raise ValueError(f'No rows in {path}')
    missing = [c for c in REQUIRED_COLUMNS if c not in rows[0]]
    if missing:
        raise ValueError(f'Missing columns in {path}: {missing}')
    return rows


def exact_compare(csv_a: list[dict], csv_b: list[dict]):
    if len(csv_a) != len(csv_b):
        return {
            'exact_match_count': 0,
            'max_abs_error': None,
            'first_mismatch_sample': min(len(csv_a), len(csv_b)),
            'mismatch_fields': ['row_length'],
            'c_value': len(csv_a),
            'matlab_value': len(csv_b),
            'status': 'row-count mismatch'
        }

    exact_match_count = 0
    max_abs_error = 0
    first_mismatch_sample = None
    mismatch_fields = []

    for idx, (ra, rb) in enumerate(zip(csv_a, csv_b)):
        row_ok = True
        sample = int(ra.get('sample_40k', idx))
        for col in REQUIRED_COLUMNS:
            try:
                va = int(float(ra[col]))
                vb = int(float(rb[col]))
            except Exception:
                va = ra[col]
                vb = rb[col]
            if va != vb:
                row_ok = False
                mismatch_fields.append(col)
                max_abs_error = max(max_abs_error, abs(int(float(va)) - int(float(vb))))
                if first_mismatch_sample is None:
                    first_mismatch_sample = sample
                    first_mismatch = (col, va, vb)
        if row_ok:
            exact_match_count += 1

    return {
        'exact_match_count': exact_match_count,
        'max_abs_error': max_abs_error,
        'first_mismatch_sample': first_mismatch_sample,
        'mismatch_fields': sorted(set(mismatch_fields)),
        'c_value': first_mismatch[1] if 'first_mismatch' in locals() else None,
        'matlab_value': first_mismatch[2] if 'first_mismatch' in locals() else None,
        'status': 'ok' if first_mismatch_sample is None else 'mismatch'
    }


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--matlab', required=True)
    parser.add_argument('--c', required=True)
    args = parser.parse_args()

    matlab_rows = load_csv(args.matlab)
    c_rows = load_csv(args.c)
    result = exact_compare(matlab_rows, c_rows)

    print('exact_match_count =', result['exact_match_count'])
    print('max_abs_error =', result['max_abs_error'])
    print('first_mismatch_sample =', result['first_mismatch_sample'])
    print('mismatch_fields =', result['mismatch_fields'])
    print('C value =', result['c_value'])
    print('MATLAB value =', result['matlab_value'])

    if result['status'] != 'ok':
        print('Note: Q-format rounding differences are expected at Q30/Q15 conversions and at the phase accumulator wrap step.')


if __name__ == '__main__':
    main()
