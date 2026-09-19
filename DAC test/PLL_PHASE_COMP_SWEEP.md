# PLL phase-compensation sweep

## Scope

Only offline analysis was performed. No firmware, PLL gain, SOGI equation or
coefficient, filter, slew limit, frequency limit, or lock criterion was
changed.

Fixed conditions:

- Grid input: 60 Hz, zero offset, zero phase
- Input amplitude: 1000 centered counts peak (1 Vpk in the test model)
- `PLL_VALID_BYPASS_TEST=1`
- Cold start: zero SOGI/filter/PI state, `theta=0`, initial frequency 60 Hz
- Sample rates: 40 kHz input and 2.5 kHz SOGI/PLL

The model parameter named `PHASE_COMP_DEG` is the feasibility-model equivalent
of firmware `PLL_PHASE_COMP_DEG`. Each point was independently simulated from
cold start; no state was carried between sweep points.

Positive Grid-to-PLL phase means PLL lag. Negative phase means PLL lead.
Zero crossings were interpolated between sampled points, paired with the
nearest same-cycle positive crossing, and wrapped to `[-180, +180)`.
Phase statistics start five grid cycles after lock and cover 20 grid cycles.
The frequency and fast/slow errors are steady values at the end of a 3-second
run.

## Required 0–5 degree sweep — simulation measured

| Phase Comp | Lock | Lock Time | PLL Freq | Grid -> PLL Phase | Max abs phase | Phase Error | Notes |
|---:|:---:|---:|---:|---:|---:|---:|---|
| 0.00 deg | Yes | 8.0 ms | 59.999801 Hz | -2.744763 deg | 2.744764 deg | fast -4.592 mpu; slow -5 mpu | PLL leads |
| 1.00 deg | Yes | 8.0 ms | 60.000100 Hz | -2.702379 deg | 2.702380 deg | fast -4.882 mpu; slow -5 mpu | PLL leads |
| 2.00 deg | Yes | 8.0 ms | 59.999926 Hz | -2.659050 deg | 2.659050 deg | fast -3.004 mpu; slow -3 mpu | PLL leads |
| 3.00 deg | Yes | 8.0 ms | 59.999746 Hz | -2.614053 deg | 2.614054 deg | fast -1.382 mpu; slow -1 mpu | PLL leads |
| 4.00 deg | Yes | 8.0 ms | 59.999914 Hz | -2.568148 deg | 2.568149 deg | fast -4.192 mpu; slow -4 mpu | Current setting; PLL leads |
| 5.00 deg | Yes | 8.0 ms | 60.000030 Hz | **-1.952543 deg** | **1.952543 deg** | fast -4.299 mpu; slow -4 mpu | Closest to zero in requested range |

All cases locked, all first asserted lock at approximately 8.0 ms, and all
steady frequency estimates remained effectively 60 Hz.

## 1–3 degree refinement — simulation measured

| Phase Comp | Lock | PLL Freq | Grid -> PLL average | Max abs phase | Fast error | Slow error |
|---:|:---:|---:|---:|---:|---:|---:|
| 1.00 deg | Yes | 60.000100 Hz | -2.702379 deg | 2.702380 deg | -4.882 mpu | -5 mpu |
| 1.25 deg | Yes | 60.000086 Hz | -2.691360 deg | 2.691360 deg | -4.793 mpu | -5 mpu |
| 1.50 deg | Yes | 59.999928 Hz | -2.680790 deg | 2.680791 deg | -2.873 mpu | -3 mpu |
| 1.75 deg | Yes | 59.999999 Hz | -2.669425 deg | 2.669425 deg | -3.876 mpu | -4 mpu |
| 2.00 deg | Yes | 59.999926 Hz | -2.659050 deg | 2.659050 deg | -3.004 mpu | -3 mpu |
| 2.25 deg | Yes | 59.999853 Hz | -2.647504 deg | 2.647504 deg | -2.254 mpu | -2 mpu |
| 2.50 deg | Yes | 59.999884 Hz | -2.637091 deg | 2.637092 deg | -2.793 mpu | -3 mpu |
| 2.75 deg | Yes | 59.999858 Hz | -2.625243 deg | 2.625243 deg | -2.616 mpu | -3 mpu |
| 3.00 deg | Yes | 59.999746 Hz | -2.614053 deg | 2.614054 deg | -1.382 mpu | -1 mpu |

The fine sweep directly disproves a simple correction based on subtracting the
previous 2.568-degree lead from the 4-degree setting. No 1–3 degree case is
close to zero output phase.

## 4–5 degree refinement — simulation measured

Because the coarse sweep showed a larger operating-point change between four
and five degrees, that interval was also measured at 0.25-degree spacing.

| Phase Comp | Lock | PLL Freq | Grid -> PLL average | Max abs phase | Fast error | Slow error |
|---:|:---:|---:|---:|---:|---:|---:|
| 4.00 deg | Yes | 59.999914 Hz | -2.568148 deg | 2.568149 deg | -4.192 mpu | -4 mpu |
| 4.25 deg | Yes | 59.999762 Hz | -2.556774 deg | 2.556774 deg | -3.179 mpu | -3 mpu |
| 4.50 deg | Yes | 59.999915 Hz | -2.528232 deg | 2.528232 deg | -4.785 mpu | -5 mpu |
| 4.75 deg | Yes | 60.000087 Hz | -2.234088 deg | 2.234089 deg | -4.792 mpu | -5 mpu |
| 5.00 deg | Yes | 60.000030 Hz | **-1.952543 deg** | **1.952543 deg** | -4.299 mpu | -4 mpu |

The response is not linear, especially above 4.5 degrees. The result was
therefore selected from actual sweep points and not from linear extrapolation.

## SOGI phase checks

`PHASE_COMP_DEG` is used only by the phase detector, so the SOGI phase results
were unchanged across every sweep point:

| Measurement | Result | Interpretation |
|---|---:|---|
| Grid -> SOGI alpha | +0.151139 deg | alpha lags grid |
| SOGI alpha -> SOGI beta | +89.984295 deg | beta lags alpha |

This confirms the sweep did not alter the SOGI equations or coefficients.

## Extended coarse sweep — simulation measured

| Phase Comp | Lock | Lock Time | PLL Freq | Signed phase | Average abs phase | Max abs phase | Fast error | Slow error | Grid -> alpha | Alpha -> beta |
|---:|:---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 5 deg | Yes | 8.0 ms | 60.000030 Hz | -1.952543 deg | 1.952543 deg | 1.952543 deg | -4.299 mpu | -4 mpu | +0.151139 deg | +89.984295 deg |
| 6 deg | Yes | 8.0 ms | 60.000235 Hz | -0.834026 deg | 0.834026 deg | 0.834026 deg | -4.092 mpu | -4 mpu | +0.151139 deg | +89.984295 deg |
| 7 deg | Yes | 8.0 ms | 60.000235 Hz | -0.154170 deg | 0.154170 deg | 0.154171 deg | +1.329 mpu | +1 mpu | +0.151139 deg | +89.984295 deg |
| 8 deg | Yes | 8.0 ms | 60.000345 Hz | -0.104481 deg | 0.104481 deg | 0.104482 deg | +1.136 mpu | +1 mpu | +0.151139 deg | +89.984295 deg |
| 9 deg | Yes | 8.0 ms | 60.000029 Hz | -0.048591 deg | 0.048591 deg | 0.048591 deg | +3.552 mpu | +4 mpu | +0.151139 deg | +89.984295 deg |
| 10 deg | Yes | 8.0 ms | 60.000329 Hz | -0.863149 deg | 0.863149 deg | 0.863150 deg | +2.582 mpu | +3 mpu | +0.151139 deg | +89.984295 deg |
| 12 deg | Yes | 8.0 ms | 60.000227 Hz | -0.653714 deg | 0.653714 deg | 0.653715 deg | +1.435 mpu | +1 mpu | +0.151139 deg | +89.984295 deg |
| 15 deg | Yes | 8.0 ms | 60.000423 Hz | -0.351155 deg | 0.351155 deg | 0.351156 deg | +3.007 mpu | +3 mpu | +0.151139 deg | +89.984295 deg |

All requested coarse points remained negative, so they did not by themselves
bracket a zero crossing. The search was extended only far enough to find the
first sign change: 16 degrees measured +0.480351 degrees. The first coarse
zero-crossing bracket was therefore `[15, 16]` degrees. The non-monotonic jump
at 10 degrees is retained in the table and was not smoothed or extrapolated.

## Zero-crossing refinement

### 0.25-degree sweep

| Comp | Lock | PLL Freq | Signed phase | Average abs | Max abs | Fast error | Slow error |
|---:|:---:|---:|---:|---:|---:|---:|---:|
| 15.00 deg | Yes | 60.000423 Hz | -0.351155 deg | 0.351155 deg | 0.351156 deg | +3.007 mpu | +3 mpu |
| 15.25 deg | Yes | 59.999999 Hz | -0.150340 deg | 0.150340 deg | 0.150341 deg | +5.138 mpu | +5 mpu |
| 15.50 deg | Yes | 60.000196 Hz | +0.063683 deg | 0.063683 deg | 0.063683 deg | +3.423 mpu | +3 mpu |
| 15.75 deg | Yes | 60.000279 Hz | +0.274710 deg | 0.274710 deg | 0.274711 deg | +2.884 mpu | +3 mpu |
| 16.00 deg | Yes | 59.999998 Hz | +0.480351 deg | 0.480351 deg | 0.480352 deg | +5.094 mpu | +5 mpu |

This reduced the bracket to `[15.25, 15.50]` degrees.

### 0.05-degree sweep

| Comp | PLL Freq | Signed phase | Max abs | Fast error | Slow error |
|---:|---:|---:|---:|---:|---:|
| 15.25 deg | 59.999999 Hz | -0.150340 deg | 0.150341 deg | +5.138 mpu | +5 mpu |
| 15.30 deg | 60.000377 Hz | -0.106180 deg | 0.106180 deg | +2.333 mpu | +2 mpu |
| 15.35 deg | 60.000344 Hz | -0.064871 deg | 0.064871 deg | +2.750 mpu | +3 mpu |
| 15.40 deg | 60.000081 Hz | -0.016341 deg | 0.016341 deg | +5.043 mpu | +5 mpu |
| 15.45 deg | 60.000239 Hz | +0.019294 deg | 0.019294 deg | +3.247 mpu | +3 mpu |
| 15.50 deg | 60.000196 Hz | +0.063683 deg | 0.063683 deg | +3.423 mpu | +3 mpu |

This reduced the bracket to `[15.40, 15.45]` degrees.

### 0.01-degree sweep

| Comp | Lock | Lock Time | PLL Freq | Signed phase | Average abs | Max abs | Fast error | Slow error |
|---:|:---:|---:|---:|---:|---:|---:|---:|---:|
| 15.40 deg | Yes | 8.0 ms | 60.000081 Hz | -0.016341 deg | 0.016341 deg | 0.016341 deg | +5.043 mpu | +5 mpu |
| 15.41 deg | Yes | 8.0 ms | 60.000069 Hz | -0.008478 deg | 0.008478 deg | 0.008478 deg | +4.560 mpu | +5 mpu |
| **15.42 deg** | **Yes** | **8.0 ms** | **60.000458 Hz** | **-0.002856 deg** | **0.002856 deg** | **0.002857 deg** | **+1.484 mpu** | **+2 mpu** |
| 15.43 deg | Yes | 8.0 ms | 60.000340 Hz | +0.004583 deg | 0.004583 deg | 0.004584 deg | +2.388 mpu | +2 mpu |
| 15.44 deg | Yes | 8.0 ms | 60.000414 Hz | +0.012188 deg | 0.012188 deg | 0.012188 deg | +2.434 mpu | +3 mpu |
| 15.45 deg | Yes | 8.0 ms | 60.000239 Hz | +0.019294 deg | 0.019294 deg | 0.019294 deg | +3.247 mpu | +3 mpu |

The final measured sign-change bracket is `[15.42, 15.43]` degrees. Of the
required 0.01-degree grid points, 15.42 degrees has the smallest absolute
average phase.

## Sensitivity around the optimum

| Comp | Grid -> PLL phase | PLL Freq | Fast error | Slow error |
|---:|---:|---:|---:|---:|
| 14.90 deg | -0.433862 deg | 60.000202 Hz | +4.182 mpu | +4 mpu |
| 15.00 deg | -0.351155 deg | 60.000423 Hz | +3.007 mpu | +3 mpu |
| 15.10 deg | -0.266460 deg | 60.000290 Hz | +3.099 mpu | +3 mpu |
| 15.20 deg | -0.184616 deg | 60.000252 Hz | +3.516 mpu | +4 mpu |
| 15.30 deg | -0.106180 deg | 60.000377 Hz | +2.333 mpu | +2 mpu |
| 15.40 deg | -0.016341 deg | 60.000081 Hz | +5.043 mpu | +5 mpu |
| **15.42 deg** | **-0.002856 deg** | **60.000458 Hz** | **+1.484 mpu** | **+2 mpu** |
| 15.50 deg | +0.063683 deg | 60.000196 Hz | +3.423 mpu | +3 mpu |
| 15.60 deg | +0.147752 deg | 60.000016 Hz | +4.978 mpu | +5 mpu |
| 15.70 deg | +0.230685 deg | 60.000484 Hz | +1.110 mpu | +1 mpu |
| 15.80 deg | +0.312661 deg | 60.000360 Hz | +2.093 mpu | +2 mpu |
| 15.90 deg | +0.395011 deg | 60.000217 Hz | +3.374 mpu | +3 mpu |

Around the first zero crossing the phase slope is roughly 0.8 degree of output
phase per degree of compensation. It is not a singular spike: approximately
15.30–15.50 degrees remains within 0.11 degree, while approximately
15.40–15.45 degrees remains within 0.02 degree. The small non-smooth changes in
frequency and error are consistent with the integer slow-error/deadband state.

## SOGI reference preservation

Every extended coarse, fine, and sensitivity case measured the same references
to the shown precision:

```text
Grid -> SOGI alpha       = +0.151138887 deg  (alpha lags grid)
SOGI alpha -> SOGI beta = +89.984295485 deg (beta lags alpha)
```

They do not follow the phase-compensation sweep. No SOGI/measurement
cross-coupling was observed.

## Updated conclusion

The best tested compensation on the required 0.01-degree grid is:

```text
PHASE_COMP_DEG              = 15.42 deg
Grid -> PLL average phase   = -0.002856442 deg
Grid -> PLL max abs phase   =  0.002857064 deg
steady PLL frequency        = 60.000457689 Hz
residual direction          = PLL leads grid
equivalent average dt       = -0.132 us
```

The time conversion uses the measured 60 Hz grid period, not a phase-comp
linear extrapolation. The neighboring 15.43-degree point is already positive
at +0.004583 degrees, so `[15.42, 15.43]` is the resolved zero-crossing
bracket. No firmware or PLL/SOGI dynamic constant was modified.
