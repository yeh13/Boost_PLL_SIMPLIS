#!/usr/bin/env python3
import math

# Verification helper for the signed quotient/remainder behavior used by the MATLAB
# and Verilog phase-delivery candidates.
# This is a reference checker, not an algorithm change.

def trunc_qr(x: int):
    q = int(math.trunc(x / 8.0))
    r = x - q * 8
    return q, r


def distributed_sequence(q: int, r: int):
    acc = 0
    seq = []
    for _ in range(8):
        corr = q
        acc += abs(r)
        if acc >= 8:
            corr += 1 if r > 0 else (-1 if r < 0 else 0)
            acc -= 8
        seq.append(corr)
    return seq, sum(seq)


def print_case(label: str, err: int):
    q, r = trunc_qr(err)
    seq, total = distributed_sequence(q, r)
    print(f"{label}: error={err}, q={q}, r={r}, seq={seq}, total={total}")


if __name__ == '__main__':
    cases = [1, 7, 8, 9, -1, -7, -8, -9, 15, -15]
    print("Directed signed quotient/remainder checks")
    for e in cases:
        print_case("case", e)

    print("\nFloor-vs-fix check for negative values")
    for e in (-1, -7, -8, -9, -15):
        q_fix, r_fix = trunc_qr(e)
        q_floor = math.floor(e / 8.0)
        r_floor = e - q_floor * 8
        seq_fix, total_fix = distributed_sequence(q_fix, r_fix)
        seq_floor, total_floor = distributed_sequence(q_floor, r_floor)
        print(
            f"err={e}: fix(q={q_fix}, r={r_fix}, seq={seq_fix}, total={total_fix}), "
            f"floor(q={q_floor}, r={r_floor}, seq={seq_floor}, total={total_floor}), "
            f"same_seq={seq_fix == seq_floor}, same_total={total_fix == total_floor}"
        )

    print("\nArray-size exact-ramp check (128-sample, step = nominal+1)")
    # This mirrors the MATLAB synthetic test: exact ramp must be reproduced without a one-tick delay.
    step = 9059696 + 1
    M = 360 * 512 * 4096 * 8
    phase = 0
    oracle = []
    for n in range(128):
        phase = (phase + step) % M
        oracle.append(phase)
    print(f"sample_count={len(oracle)}, first={oracle[:8]}, last={oracle[-8:]}")
